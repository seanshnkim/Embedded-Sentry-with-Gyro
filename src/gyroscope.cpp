#include "gyroscope.h"

SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
// DigitalOut cs(PC_1);
EventFlags flags;

GyroData keyGesture[100];
GyroData enteredGesture[100];
volatile int gestureIndex = 0;

// Buffers for SPI data transfer:
// - write_buf: stores data to send to the gyroscope
// - read_buf: stores data received from the gyroscope
uint8_t write_buf[32], read_buf[32];

volatile bool isRecording = false;
volatile bool isEntering = false;

void spi_cb(int event) {
    flags.set(SPI_FLAG);  // Set the SPI_FLAG to signal that transfer is complete
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

// function input is GyroData array
void sample_gyro_data(GyroData* InputGesture) {
    float gx, gy, gz;
    uint16_t raw_gx, raw_gy, raw_gz;
    gestureIndex = 0;
    
    while (isRecording || isEntering) {
        write_buf[0] = OUT_X_L | 0x80 | 0x40;
        spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
        flags.wait_all(SPI_FLAG);
        flags.clear(SPI_FLAG);

        raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];
        raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];
        raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

        gx = raw_gx * DEG_TO_RAD;
        gy = raw_gy * DEG_TO_RAD;
        gz = raw_gz * DEG_TO_RAD;

        InputGesture[gestureIndex] = {gx, gy, gz, us_ticker_read() / 1000};
        gestureIndex++;

        ThisThread::sleep_for(100ms);
    }
}

float dtw(const GyroData* gest1, const GyroData* gest2, int len1, int len2) {
    const int MAX_LEN = 100; // Adjust this based on your maximum gesture length
    float dtw[MAX_LEN][MAX_LEN];

    // Initialize the DTW matrix
    for (int i = 0; i < len1; i++) {
        for (int j = 0; j < len2; j++) {
            dtw[i][j] = std::numeric_limits<float>::infinity();
        }
    }

    dtw[0][0] = 0;

    // Calculate DTW matrix
    for (int i = 1; i < len1; i++) {
        for (int j = 1; j < len2; j++) {
            float cost = std::sqrt(
                std::pow(gest1[i].x - gest2[j].x, 2) +
                std::pow(gest1[i].y - gest2[j].y, 2) +
                std::pow(gest1[i].z - gest2[j].z, 2)
            );
            dtw[i][j] = cost + std::min({dtw[i-1][j], dtw[i][j-1], dtw[i-1][j-1]});
        }
    }

    // Return the DTW distance
    return dtw[len1-1][len2-1];
}

bool compareGest(const GyroData* keyGest, const GyroData* enteredGest, int keyLen, int enteredLen) {
    float dtwDistance = dtw(keyGest, enteredGest, keyLen, enteredLen);
    
    // You may need to adjust this threshold based on your specific use case
    const float THRESHOLD = 5.0f;
    
    return dtwDistance < THRESHOLD;
}