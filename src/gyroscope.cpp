#include "gyroscope.h"

SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
// DigitalOut cs(PC_1);
Ticker sampler;
EventFlags flags;

std::vector<GyroData> recordedGesture;

// Buffers for SPI data transfer:
// - write_buf: stores data to send to the gyroscope
// - read_buf: stores data received from the gyroscope
uint8_t write_buf[32], read_buf[32];

void spi_cb(int event) {
    flags.set(SPI_FLAG);  // Set the SPI_FLAG to signal that transfer is complete
}

void read_gyro_data(float &gx, float &gy, float &gz) {
    uint16_t raw_gx, raw_gy, raw_gz;

    // Prepare to read gyroscope output starting at OUT_X_L
    write_buf[0] = OUT_X_L | 0x80 | 0x40; // Read mode + auto-increment
    
    // cs = 0;
    spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
    flags.wait_all(SPI_FLAG);
    // cs = 1;

    // --- Extract and Convert Raw Data ---
    // Combine high and low bytes for X-axis
    raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];

    // Combine high and low bytes for Y-axis
    raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];

    // Combine high and low bytes for Z-axis
    raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

    gx = raw_gx * DEG_TO_RAD;
    gy = raw_gy * DEG_TO_RAD;
    gz = raw_gz * DEG_TO_RAD;
}

void sample_data() {
    if (isRecording) {
        float gx, gy, gz;
        read_gyro_data(gx, gy, gz);
        
        GyroData data = {gx, gy, gz, us_ticker_read() / 1000};
        recordedGesture.push_back(data);
    }
}

void startRecording() {
    recordedGesture.clear();
    isRecording = true;
    sampler.attach(&sample_data, 10ms);
}

void stopRecording() {
    isRecording = false;
    sampler.detach();
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