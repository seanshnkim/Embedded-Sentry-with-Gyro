#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#include "mbed.h"

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

struct GyroData {
    float x, y, z;
    uint32_t timestamp;
};

extern GyroData recordedGesture[1000];
extern volatile bool isRecording;
extern volatile int gestureIndex;

void read_gyro_data(float &gx, float &gy, float &gz);
void sample_data();
void startRecording();
void stopRecording();
void init_gyroscope();

#endif