# Embedded-Sentry-with-Gyro
Embedded Challenge Fall 2024 Term Project: "Embedded Sentry"

#### Presentation and Video:
https://docs.google.com/presentation/d/1zvveqmzDyTDRLGLaccSPtkZkLWgNfwToebSEEsMKPLg/edit?usp=sharing


## Objective
The objective is to implement "unlock/lock" system using hand gestures. It uses the STM32F429 microcontroller. The device has several peripherals, but I mainly used three components:

1) LCD display
2) Gyroscope
3) Button

- LCD displays text and emojis so the user knows whether the input is correct.
- The gyroscope captures hand gestures as angular rate or rotational speed on the x, y, and z axes. I used SPI to communicate with the gyroscope.

# How to Interact

## Start with Button
1. Power the board.
2. The LCD shows “press to start”
3. Press the blue button:

- Long press (> 1 second): record/register a new key gesture.
- Short press (≤ 1 second): enter a gesture to unlock.

The button press and release are handled by interrupts, and a timer measures the press duration (`pressDuration`) to decide between “record(=register)” and “enter.”

## Sample Gyroscope Data
A separate thread samples gyroscope data in a loop based on a state variable `localState`:

1) `RECORDING` or `ENTERING`: collects gesture data and then stop sampling.
2) `COMPARING`: runs a similarity comparison algorithm, **Dynamic Time Warping (DTW)** on the stored key vs. input gesture; if the returned "distance score" is below a threshold, the device is considered “unlocked” and a message will be printed.

- In `sample_gyro_data`, it receives bytes from the gyroscope via **SPI** protocol. 
- I used **mutex** to prevent data race condition for `currentState` between the button interrupt and the sampling/comparison logic.


## Design Notes
### Question 1: What can be improved?
- Using an infinite while loop doesn't seem like a good idea. Why not let the button interrupt directly invoke each function?
- `keyGesture` and `enteredGesture` are defined in a separate header file, which can be confusing. Likewise, the state variables (`IDLE`, `RECORDING`, etc.) are defined in `state.h`. For a small-scale project, this level of separation may not be necessary.


### Question 2: Why use SPI over I2C?
The gyroscope L3GD20 supports both I2C and SPI digital output interfaces. Then why choose SPI over I2C?

-> SPI is used instead of I2C because it provides higher throughput and lower latency, and the extra pins are not an issue on this board.

### Question 3: `MAX_LEN` set to 100?

- The gesture length `MAX_LEN` is limited to 100 to fit DTW 2D matrix (total size is `8 bytes/float * MAX_LEN * MAX_LEN`).
- The STM32F429 has 2MB of flash, 256 KB of SRAM. 
- Doubling `MAX_LEN` to 200 would make the DTW matrix alone require about 320 KB and risk memory errors.