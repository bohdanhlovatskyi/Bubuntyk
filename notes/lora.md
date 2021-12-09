

# Prerequisits

STM B-L072Z-LRWAN (also will work with little change for Nucleo board with LoRa support)

# Getting source code
https://www.st.com/content/st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32cube-expansion-packages/i-cube-lrwan.html

then follow this path to get the example project that will contain required drivers and primitive LoRa setup

/Users/hlovatskyibohdan/STM32CubeExpansion_LRWAN_V2.1.0/Projects/B-L072Z-LRWAN1/Applications/SubGHz_Phy/SubGHz_Phy_PingPong

Note that the LoRa module is called actually SX1276, though this example project contains BSP wrapper and actual code for this sensor is put into Middleware folder. 

Main functional protopypes are located in the `radio.h`. 

Also note that one of the sensor should be defined as slave (isMaster define in one of the files (can be located from the example source via look definition command)). Though example worked without it (only for one sensor, for some reason). 

# to list available ports
ls /dev/{tty,cu}.*

# to open the serial connection from temrinal
screen /dev/tty.usbmodem142203 115200 

(screen port_name baud_rate)

# to kill the screen
- open different terminal window
- `screen -list`
- then you will see sth like this:
There is a screen on:
23536.pts-0.wdzee       (10/04/2012 08:40:45 AM)        (Detached)
1 Socket in /var/run/screen/S-root.

- then you can kill the screen you are using via: `screen -S 23536 -X quit`



# Future work:
- find out more on LoRa as code contains a lot of complexity, connected with the setup (especially how to transmit data packages that are larger than one transmission). (Though function declaration hint us that we can put LoRa into transmission state for some time and it will take its one second of on-air time each 100 seconds ???)
- try to fetch this as library to self-generated project, as it would bring unneccesary complexitry to the project (basing main code on this example project). 
- Related to previous one: stm32cubeide does not contain ioc file for some reason - speak with mr. Oleg about this

# Secon possible solution
- uses mbed os
- there is online compiler with example project (though it did not started from very beginning) (or it did started, but some master-slave setup is missing ???)
https://ide.mbed.com/compiler/#nav:/SX1276PingPong;

here is nice tutorial: https://youtu.be/A3cZSTfxmb4

then we can import this project into MBed studio
go copy the url of project https://os.mbed.com/teams/Semtech/code/SX1276PingPong/

then main page: import project: paste the url: import to local storage (not exact naming, but the spirit is passed, I assume)

(it does not work ((( wrong mbed studio version)

actually that is the same code as in the previous way, though it seems that is shows how to get rid of unneccessary complexity of the system

https://ide.mbed.com/compiler/#nav:/SX1276PingPong;panel:import_wizard;
