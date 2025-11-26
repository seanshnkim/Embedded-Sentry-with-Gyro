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

## IDE
- PlatformIO
- VSCode

## Mainboard & Microcontroller
- STM32F429I Discovery


## 1. User Button
1. Connect the board to power
2. The LCD will display “press to start”
3. Press the blue button

Here is the important point. It has two different modes depending on how long the button is pressed.

- Long press: If `pressDuration` is longer than 1 second, the program starts recording a key. In other words, it registers a new key (apologize for the bad naming).
- Short press: If the duration is equal to or shorter than 1 second, it starts receiving an input key to unlock the device.

### Question: How does it know if it's short or long?

Using the `mbed` library, I created an interrupt for button press and release (key down and up).

`.rise` = button pressed down
`.fall` = button released

And using a timer tick, it calculates the difference between 
- A: the time when the button is pressed
- B: the time when the button is released

If the difference `B-A` is longer than 1 second it will invoke a callback for registering a key.
If shorter, than invoke a callback for entering a key.


## 2. Sampling Data
There is a separate thread for sampling the hand gesture (= time series) data.

In an infinite loop, it reads a flag variable `localState`, and depending on its value it performs different operations.

1) If `RECORDING` or `ENTERING`: It collects gyro data, gets the gesture data length, prints out a debug message, and then breaks out of the while loop.
2) Else if `COMPARING`: It runs similarity comparison algorithm, specifically **Dynamic Time Warping (DTW)**. It returns a "distance score", and if that score is below a threshold it "unlocks" the device. How does it unlock the device? Well, it prints a message ("*Unlocked!*" or "*Try it again*"), and the user will know whether the device is unlocked.

In `sample_gyro_data`, it receives bytes from the gyroscope via SPI protocol. 

### Question 1: Why use mutex?
Mutex prevents data race condition when multiple threads try to access the same data at the same time. For example, user might inadvertently press the button (which changes `currentState`) while the program is in `COMPARING` mode (which also updates `currentState` to `IDLE`). This could lead to unexpected or inconsistent results. In other words, a mutex is used to guarantee an atomic operation.

### Question 2: What can be improved?
- Using an infinite while loop doesn't seem like a good idea. Why not let the button interrupt directly invoke each function?
- `keyGesture` and `enteredGesture` are defined in a separate header file, which can be confusing. Likewise, the state variables (`IDLE`, `RECORDING`, etc.) are defined in `state.h`. For a small-scale project, this level of separation may not be necessary.


### Question 3: Why use SPI over I2C?
The gyroscope L3GD20 supports both I2C and SPI digital output interfaces. Then why choose SPI over I2C?

SPI is a good choice for high throughput, low latency, deterministic sampling and DMA/burst transfers. But it requires more pins (typically 4 pins per device). However, since the gyroscope is already wired to the processor on the board, the pin count is not a big concern here.

I2C is better for low pin count and multi-device buses where throughput or latency are less critical. I2C has start/stop conditions and ACK/NACK for error handling, which can slow down transmission.


## 3. Dynamic Time Warping and Moving Average Window
It compares two sequences of data and computes a distance. It finds the best alignment between two time series, even if they differ in speed or length. I limited the size of gesture sequence `MAX_LEN` to 100, as STM32F429 has relatively small memory - 2MB of flash and 256 KByte of SRAM.

I used moving average filter to smooth out the samples and reduce unwanted fluctuations.

### Question: What type of error can happen if the input size is very long?
There are possible issues if the input size becomes very large, although they have not been verified by tests yet:

- Memory allocation failure / Memory (RAM) overflow
- HardFault / BusFault / stack overflow if stack or heap is corrupted by large local arrays
- Silent data corruption and sporadic crashes if memory gets overwritten (undefined behavior)

Quick math: If `MAX_LEN` is doubled, will it cause memory allocation failure?
-> Answer is NO. `dtwMatrix` is a 2D matrix with size of `MAX_LEN * MAX_LEN`, then it will require approximately (`8 bytes/float * 200 * 200`) ~31KB, which is still less than the RAM size.

