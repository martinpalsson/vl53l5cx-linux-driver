# vl53l5cx-linux-driver
A fork of the official VL53L5CX linux driver from ST Microelectronics, adding enablers for multi-sensor systems.

## Get started
1. Connect sensor/take note of how it is connected.
2. Adapt kernel/stmvl53l5cx.dts according to your installation.
3. Read through docs/README.md, only Option 1/Kernel mode is relevant here although user mode is still supported. The source in this repo is prepared for Option 1/Kernel mode.
4. After trying the test examples, go on to compile and execute user/multi_test to verify multi-sensor is working.
```
§ cd user/multi_test
§ make
§ sudo ./multi_test
```

5. You can check that the sensors has been configured with correct I2C addresses with the i2cdetect tool
```
§ sudo apt-get install i2c-tools
§ i2cdetect -y 1 # detect devices on i2c-1
```

Addresses "taken" by any kernel module will show up as UU in the address table, e.g.
```
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: UU UU -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

## Added functionality

### I2C address assignment at system startup
The I2C address can be configured on each sensor. This can only be done if there is only one sensor with the default I2C address active on the I2C bus at a time. This condition can easily be achieved by setting all sensors to low power mode, at system start up, passively, by utilizing the LPn (Low Power inverted) pin of the sensor and simple but a bit specific GPIO config. 

Important! To be sure all sensors are in low power mode at system start up, the GPIO connected to the LPn must be configured as
* pull-down (or use external pull-down resistor)
* active-high
* 0 as initialized output

### Platform glue adapted for multi-sensor on linux
The ST example only give a hardcoded file descriptor path. This has been amended with a field in the platform struct where the application can set the file descriptor path for each individual sensor, with corresponding code in the vl53l5cx_comms_init function.

### Improved example device tree source
Added comments to the example device tree source (dts). The original dts was somewhat lacking of description. Still some things to work out, as well as IMO less than optimal naming. 

## Todo

### Preemptive I2C reset during setup
As the linux machine may very well be rebooted without cycling the supply for each sensor, the previous I2C address may linger on the sensor the next time the system loads the kernel module. Currently, the kernel module expect the sensor on the default I2C address at this point, with a lingering I2C addres on the sensor the kernel module will not find it. This can be solved by defaulting the I2C address by reseting the I2C controller on the sensor.

## Misc
This kernel module has been tested on RPI 3B, with two VL53L5CX-SATEL breakout boards (which coincidentally are sold in pairs).

The original ST Microelectronics documentation has been moved to the docs catalog.