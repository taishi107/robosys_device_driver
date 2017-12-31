# robosys_device_driver
LED Device Driver for Raspberry Pi3

# Demo
* [LED device driver]()

# Requirements

- RaspberryPi 3
  - Raspbian
- linux kernel source
  - download kernel source into /usr/src/linux
  - kernel build scripts : https://github.com/ryuichiueda/raspberry_pi_kernel_build_scripts
- LED
- Resistor 10[Ω]

# Picture
![circuit](https://github.com/taishi107/robosys_device_driver/blob/master/DSC_0942.jpg)

# Installation
- download repository
 ```
 git clone https://github.com/taishi107/robosys_device_driver.git
 ```
- compile and loading kernel module  
 ```
 cd robosys_device_driver 
 make  
 sudo insmod led.ko
 sudo chmod 666 /dev/myled0
 ```
 
# Usage
- Light for 3 seconds
```
echo t 3 > /dev/myled0
```
- Light 4 times
```
echo c 4 > /dev/myled0
```
