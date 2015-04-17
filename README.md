# rpiGPIOserial-to-mysql
cross compile linux eclipse project for raspberry pi GPIO serial port save data to mySQL database

This project started as a fun idea to just cross-compile in my "desktop" linux, creating binaries to execute on the raspberry pi.  This quickly led to, "Hey!  I should log the data into a databse!"
Which of course led to, "If it's in a standard databse, then I can show the stuff on web pages or serve up the data via some REST APIs!"

So, it's a work in progress, and a fun project so far.

development environment:
linux mint 17 64-bit
cinnamon 2.2.16

eclipse IDE:
Version: Luna Release (4.4.0)
Build id: 20140612-0600

overview:
1 clone the raspberry/tools repo
2 install and configure the eclipse C/C++ IDE

setup Eclipse IDE for a new project:
 In the “Cross compiler prefix” field type: “arm-linux-gnueabihf-”
In the “Cross compiler path” field type the full path to the toolchain
for example   /rpi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/

Huge Thanks!!! to: http://hertaville.com/2012/09/28/development-environment-raspberry-pi-cross-compiler/
