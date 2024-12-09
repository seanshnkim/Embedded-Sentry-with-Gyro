#include "mbed.h"

// =================================================
// * Recitation 7: Gyroscope & Interrupts *
// =================================================

// TODOs:
// [1] Read the XYZ axis data from the Gyroscope and Visualize on the Teleplot.
// [2] Introduction to Interrupts! 
// [3] Fetching Data from the sensor via Polling vs Interrupts?

// --- Register Addresses and Configuration Values ---
#define CTRL_REG1 0x20               // Control register 1 address
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1  // Configuration: ODR=100Hz, Enable X/Y/Z axes, power on
#define CTRL_REG4 0x23               // Control register 4 address
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0  // Configuration: High-resolution, 2000dps sensitivity

// SPI communication completion flag
#define SPI_FLAG 1

// Address of the gyroscope's X-axis output lower byte register
#define OUT_X_L 0x28

// Scaling factor for converting raw sensor data in dps (deg/s) to angular velocity in rps (rad/s)
// Combines sensitivity scaling and conversion from degrees to radians
#define DEG_TO_RAD (17.5f * 0.0174532925199432957692236907684886f / 1000.0f)

// EventFlags object to synchronize asynchronous SPI transfers
EventFlags flags;

// --- SPI Transfer Callback Function ---
// Called automatically when an SPI transfer completes
void spi_cb (int event) {
    flags.set(SPI_FLAG);  // Set the SPI_FLAG to signal that transfer is complete
}

// int main(){
//     SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);  // Create SPI object on pins D13, D12, and D11

//     uint8_t write_buf[32], read_buf[32];  // Buffers for SPI write and read data

//     spi.format(8, 3);  // Setup SPI format with 8 data bits and mode 3
//     spi.frequency(1'000'000);  // Set SPI frequency to 1MHz

//     write_buf[0] = CTRL_REG1;  // Set the first byte of the write buffer to the control register 1 address
//     write_buf[1] = CTRL_REG1_CONFIG;  // Set the second byte to the control register 1 configuration value
//     spi.transfer(write_buf, 2, read_buf, 2, spi_cb);  // Start an asynchronous SPI transfer to write the control register 1 configuration
//     flags.wait_all(SPI_FLAG);  // Wait for the SPI transfer to complete

//     write_buf[0] = CTRL_REG4;
//     write_buf[1] = CTRL_REG4_CONFIG;
//     spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
//     flags.wait_all(SPI_FLAG);

//     while (1) {
//         uint16_t raw_gx, raw_gy, raw_gz;  // Variables to store raw data
//         float gx, gy, gz;  // Variables to store converted angular velocity values

//         // Prepare to read gyroscope output starting at OUT_X_L
//         // - write_buf[0]: register address with read (0x80) and auto-increment (0x40) bits set
//         write_buf[0] = OUT_X_L | 0x80 | 0x40; // Read mode + auto-increment

//         // Perform SPI transfer to read 6 bytes (X, Y, Z axis data)
//         // - write_buf[1:6] contains dummy data for clocking
//         // - read_buf[1:6] will store received data
//         spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
//         flags.wait_all(SPI_FLAG);  // Wait until the transfer completes

//         // --- Extract and Convert Raw Data ---
//         // Combine high and low bytes for X-axis
//         raw_gx = (((uint16_t)read_buf[2]) << 8) | read_buf[1];

//         // Combine high and low bytes for Y-axis
//         raw_gy = (((uint16_t)read_buf[4]) << 8) | read_buf[3];

//         // Combine high and low bytes for Z-axis
//         raw_gz = (((uint16_t)read_buf[6]) << 8) | read_buf[5];

//         /*
//         Why oscillation? Because it is an unsigned byte.
//         Hovering between 0, so -1 is converted into 65536
//         That is why it looks like oscillating.
//         */ 

//         // --- Debug and Teleplot Output ---
//         // Print raw values for debugging purposes
//         printf("RAW Angular Speed -> gx: %d deg/s, gy: %d deg/s, gz: %d deg/s\n", raw_gx, raw_gy, raw_gz);

//         // Print formatted output for Teleplot
//         printf(">x_axis: %d|g\n", raw_gx);
//         printf(">y_axis: %d|g\n", raw_gy);
//         printf(">z_axis: %d|g\n", raw_gz);

//         // --- Convert Raw Data to Angular Velocity ---
//         // Scale raw data using the predefined scaling factor
//         gx = raw_gx * DEG_TO_RAD;
//         gy = raw_gy * DEG_TO_RAD;
//         gz = raw_gz * DEG_TO_RAD;

//         // Print converted values (angular velocity in rad/s)
//         printf("Angular Speed -> gx: %.5f rad/s, gy: %.5f rad/s, gz: %.5f rad/s\n", gx, gy, gz);

//         // Delay for 100 ms before the next read
//         thread_sleep_for(100);
//     }

//     // button.rise(&toggle_led);
//     // You can use button interrupt in your project

//     return 0;
// }