# PixEagle
![Screenshot 2025-08-02 143302](https://github.com/user-attachments/assets/5958b0be-9736-45da-82f3-39a60d72d2f4)


### Flight controller  reference design based on the STM32H743VIT6 microcontroller, and PX4 clone of  PX4-Autopilot\boards\px4\fmu-v6c

# Hardware Selection


## STM32H743VIT6:
Cortex-M7 core, 1 MB flash, and 512 KB SRAM
## IST8310 
(Magnetometer): I2C-based, low power, 
## MMC5983MA 
(Magnetometer): High-precision magnetometer on a dedicated SPI bus 
## BMI088 
(IMU): High-performance accelerometer and gyroscope 
## ICM-42688-P 
(IMU): Another high-performance IMU, providing redundancy and potentially better noise characteristics.
## BMP388 
(Barometer): High-precision barometer for altitude estimation, shares I2C1 with IST8310.
## BMP390 
(Barometer): Similar to BMP388, on dedicated I2C2 for redundancy or specialized use cases.
## FM25V01A-GTR 
(FRAM): 128 KB FRAM for parameter storage, faster and more durable than EEPROM, 
## MicroSD Card: 
Standard for PX4 data logging, connected via SPI4, which is appropriate.

# Pin Assignmnet 

<img width="458" height="1601" alt="image" src="https://github.com/user-attachments/assets/0c4f43f0-293c-434a-8e6e-daa5bc631675" />


<img width="1621" height="625" alt="image" src="https://github.com/user-attachments/assets/e1436987-e5f9-4632-915b-7d24ad0e51b0" />


# Sensor peripheral allocation 	

<img width="190" height="201" alt="image" src="https://github.com/user-attachments/assets/d732798a-1cc7-4755-992b-0509bb32f1ea" />

# Power and Voltage Regulation

The board uses a LMR14050SDDAR buck converter to step down the battery voltage to 5V, which powers both the main module and external sensors. This 5V rail then feeds two dual-channel NCV8154MW330330TBG LDOs.
One LDO is dedicated to powering the MCU and the ICM-42688-P IMU, while the second LDO powers the BMI088 and the remaining sensors. To enable or disable power to the sensor rail, pin PA15 is used as an enable signal. The other three LDO enable pins are tied high for continuous operation. A BLM18PG121SN1D ferrite bead protects the power pin of each internal sensor.


# LEDs and Indicators

The WS2812B LED on pin PE14 is configured for a daisy-chain connection with two LEDs. This allows for complex visual effects controlled by a library like FastLED, which is included in the project's source folder. The CMakeLists.txt file must be updated to link this library, as the pin will not be driven by a standard timer-based LED driver.

# Connectivity and Communication

The SBUS_PPM connector supports automatic detection of the RC input protocol. It uses pin PB1 (with a timer peripheral) for PPM input and pin PD9 (UART3 RX) for both normal and inverted SBUS.
The MicroSD card is connected via SPI2 using PB11 as Chip Select (CS), and pins PB10, PB14, and PB15 for the data lines, rather than using the dedicated SDMMC interface.
For serial communication, UART4 and UART5 are dedicated to debug and external 5V sensors. USART2 is reserved for 5V telemetry and includes hardware flow control. UART7 provides an internal serial connection to a companion computer, such as an ESP32 or CM4.
The STM32H743VIT6 MCU was specifically chosen for its dual CAN FD (Controller Area Network with Flexible Data-rate) lines, which connect to external CAN-enabled modules and sensors via two TCAN1044VDRQ1 5V transceivers.





