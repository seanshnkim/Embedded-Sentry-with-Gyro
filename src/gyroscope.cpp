#include "gyroscope.h"

SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
// DigitalOut cs(PC_1);
EventFlags flags;

GyroData recordedGesture[1000];
volatile int gestureIndex = 0;

// Buffers for SPI data transfer:
// - write_buf: stores data to send to the gyroscope
// - read_buf: stores data received from the gyroscope
uint8_t write_buf[32], read_buf[32];
volatile bool isRecording = false;

void spi_cb(int event) {
    flags.set(SPI_FLAG);  // Set the SPI_FLAG to signal that transfer is complete
}

void read_gyro_data(float &gx, float &gy, float &gz) {
    static uint16_t raw_gx, raw_gy, raw_gz;
    static int state = 0;

    switch(state) {
        case 0:
            write_buf[0] = OUT_X_L | 0x80 | 0x40;
            spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
            state = 1;
            break;
        case 1:
            if(flags.get() & SPI_FLAG) {
                flags.clear(SPI_FLAG);
                raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];
                raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];
                raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

                // --- Debug and Teleplot Output ---
                // Print raw values for debugging purposes
                printf("RAW Angular Speed -> gx: %d deg/s, gy: %d deg/s, gz: %d deg/s\n", raw_gx, raw_gy, raw_gz);

                // Print formatted output for Teleplot
                printf(">x_axis: %d|g\n", raw_gx);
                printf(">y_axis: %d|g\n", raw_gy);
                printf(">z_axis: %d|g\n", raw_gz);

                gx = raw_gx * DEG_TO_RAD;
                gy = raw_gy * DEG_TO_RAD;
                gz = raw_gz * DEG_TO_RAD;

                // Print converted values (angular velocity in rad/s)
                printf("Angular Speed -> gx: %.5f rad/s, gy: %.5f rad/s, gz: %.5f rad/s\n", gx, gy, gz);

                state = 0;
            }
            break;
    }
}

void _read_gyro_data(float &gx, float &gy, float &gz) {
    uint16_t raw_gx, raw_gy, raw_gz;

    // Prepare to read gyroscope output starting at OUT_X_L
    write_buf[0] = OUT_X_L | 0x80 | 0x40; // Read mode + auto-increment
    
    // cs = 0;
    spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
    // flags.wait_all(SPI_FLAG);
    // cs = 1;

    // --- Extract and Convert Raw Data ---
    // Combine high and low bytes for X-axis
    raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];

    // Combine high and low bytes for Y-axis
    raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];

    // Combine high and low bytes for Z-axis
    raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

    // --- Debug and Teleplot Output ---
    // Print raw values for debugging purposes
    printf("RAW Angular Speed -> gx: %d deg/s, gy: %d deg/s, gz: %d deg/s\n", raw_gx, raw_gy, raw_gz);

    // Print formatted output for Teleplot
    printf(">x_axis: %d|g\n", raw_gx);
    printf(">y_axis: %d|g\n", raw_gy);
    printf(">z_axis: %d|g\n", raw_gz);

    gx = raw_gx * DEG_TO_RAD;
    gy = raw_gy * DEG_TO_RAD;
    gz = raw_gz * DEG_TO_RAD;

    // Print converted values (angular velocity in rad/s)
    printf("Angular Speed -> gx: %.5f rad/s, gy: %.5f rad/s, gz: %.5f rad/s\n", gx, gy, gz);
}

void sample_data() {
    if (gestureIndex) {
        float gx, gy, gz;
        read_gyro_data(gx, gy, gz);
    
        recordedGesture[gestureIndex] = {gx, gy, gz, us_ticker_read() / 1000};
        gestureIndex++;
    }
    
}

void startRecording() {
    gestureIndex = 0;
    isRecording = true;
}

void stopRecording() {
    isRecording = false;
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
    // flags.wait_all(SPI_FLAG);  // Wait until the transfer completes

    // Configure Control Register 4 (CTRL_REG4)
    // - write_buf[0]: address of the register to write (CTRL_REG4)
    // - write_buf[1]: configuration value to set sensitivity and high-resolution mode
    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);  // Initiate SPI transfer
    // flags.wait_all(SPI_FLAG);  // Wait until the transfer completes
}