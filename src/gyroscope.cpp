#include "gyroscope.h"

SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
EventFlags flags;

GyroData keyGesture[1000];
GyroData enteredGesture[1000];
volatile int gestureIndex = 0;

uint8_t write_buf[32], read_buf[32];
volatile bool isRecording = false;
volatile bool isEntering = false;

// Window Size for Moving Average Window
#define WINDOW_SIZE 10
float window_gx[WINDOW_SIZE] = {0}, window_gy[WINDOW_SIZE] = {0}, window_gz[WINDOW_SIZE] = {0};
int window_index = 0;

void spi_cb(int event) {
    flags.set(SPI_FLAG);
}

void init_gyroscope() {
    // Configure SPI interface:
    // - 8-bit data size
    // - Mode 3 (CPOL = 1, CPHA = 1): idle clock high, data sampled on falling edge
    spi.format(8, 3);
    
    // Set SPI communication frequency to 1 MHz
    spi.frequency(1'000'000);

    // --- Gyroscope Initialization ---
    // Configure Control Register 1 (CTRL_REG1)
    // - write_buf[0]: address of the register to write (CTRL_REG1)
    // - write_buf[1]: configuration value to enable gyroscope and axes
    write_buf[0] = CTRL_REG1;
    write_buf[1] = CTRL_REG1_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);  // Initiate SPI transfer
    flags.wait_all(SPI_FLAG);  // Wait until the transfer completes

    // Configure Control Register 4 (CTRL_REG4)
    // - write_buf[0]: address of the register to write (CTRL_REG4)
    // - write_buf[1]: configuration value to set sensitivity and high-resolution mode
    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);  // Initiate SPI transfer
    flags.wait_all(SPI_FLAG);  // Wait until the transfer completes
}


void sample_gyro_data(GyroData* InputGesture) {
    float gx, gy, gz;
    short raw_gx, raw_gy, raw_gz;
    gestureIndex = 0;

    while (gestureIndex < 100) {
        // Check the currentState live to stop sampling if needed
        if (currentState != RECORDING && currentState != ENTERING) {
            printf("Sampling interrupted.\n");
            break;
        }

        write_buf[0] = OUT_X_L | 0x80 | 0x40;  // Read multiple registers
        spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
        flags.wait_all(SPI_FLAG);
        flags.clear(SPI_FLAG);

        // Combine the high and low bytes
        raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];
        raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];
        raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

        gx = raw_gx * DEG_TO_RAD;
        gy = raw_gy * DEG_TO_RAD;
        gz = raw_gz * DEG_TO_RAD;

        // 2. Moving Average FIR
        window_gx[window_index] = gx;
        window_gy[window_index] = gy;
        window_gz[window_index] = gz;
        float avg_gx = 0.0f, avg_gy = 0.0f, avg_gz = 0.0f;
        for (int i = 0; i < WINDOW_SIZE; i++) {
            avg_gx += window_gx[i];
            avg_gy += window_gy[i];
            avg_gz += window_gz[i];
        }
        avg_gx /= WINDOW_SIZE;
        avg_gy /= WINDOW_SIZE;
        avg_gz /= WINDOW_SIZE;
        window_index = (window_index + 1) % WINDOW_SIZE;

        if (gestureIndex % 10 == 0) {
            printf("Raw GX: %d, GY: %d, GZ: %d\n", raw_gx, raw_gy, raw_gz);
            printf("Scaled GX: %.2f, GY: %.2f, GZ: %.2f\n", gx, gy, gz);
        }

        InputGesture[gestureIndex] = {gx, gy, gz, us_ticker_read() / 1000};
        gestureIndex++;

        ThisThread::sleep_for(100ms);
    }
}

float dtw(const GyroData* gest1, const GyroData* gest2, int len1, int len2) {
    const int MAX_LEN = 100;  // Ensure maximum size
    if (len1 > MAX_LEN || len2 > MAX_LEN || len1 <= 0 || len2 <= 0) {
        printf("Error: Invalid gesture lengths: len1=%d, len2=%d\n", len1, len2);
        return std::numeric_limits<float>::infinity();
    }

    static float dtwMatrix[MAX_LEN][MAX_LEN];  // static allocate on data segment

    // Initialize DTW matrix
    for (int i = 0; i < len1; i++) {
        for (int j = 0; j < len2; j++) {
            dtwMatrix[i][j] = std::numeric_limits<float>::infinity();
        }
    }
    dtwMatrix[0][0] = 0;

    printf("DTW Initialization Complete. Starting computation...\n");

    // Calculate DTW matrix
    for (int i = 1; i < std::min(len1, MAX_LEN); i++) {
        for (int j = 1; j < std::min(len2, MAX_LEN); j++) {
            float cost = std::sqrt(
                std::pow(gest1[i].x - gest2[j].x, 2) +
                std::pow(gest1[i].y - gest2[j].y, 2) +
                std::pow(gest1[i].z - gest2[j].z, 2)
            );

            dtwMatrix[i][j] = cost + std::min({
                dtwMatrix[i-1][j],    // Insertion
                dtwMatrix[i][j-1],    // Deletion
                dtwMatrix[i-1][j-1]   // Match
            });

            // Debug print progress -> costly!
            // printf("DTW[%d][%d]: %.2f\n", i, j, dtwMatrix[i][j]);
        }
    }
    printf("DTW Computed Successfully.\n");
    return dtwMatrix[len1-1][len2-1];
}


bool compareGest(const GyroData* keyGest, const GyroData* enteredGest, int keyLen, int enteredLen) {
    float dtwDistance = dtw(keyGest, enteredGest, keyLen, enteredLen);
    const float THRESHOLD = 5.0f;
    return dtwDistance < THRESHOLD;
}
