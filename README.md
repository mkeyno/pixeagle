# PixEagle
Flight controller  reference design based on the STM32F765VGT6 microcontroller similar to  FMUv5

# Hardware Selection


## STM32F765VGT6:
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

<img width="435" height="1621" alt="image" src="https://github.com/user-attachments/assets/8e4169dc-6607-411e-a680-ceb5e197ef5e" />


