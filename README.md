arduino_thermostat
==================

This application, mounted on an X-Board_Relay with an attached SHT1X Humidity 
and Temperature Sensor, controls a local furnace. The fernace is activated by 
triggering one of the relays on the board. 
 
On boot up, the application connects to the network, obtains an IP and 
prepares to respond to incoming http requests. External services can then 
connect to the device and set the desired temperature. This application will 
monitor the attached humidity and temperature sensors and to determine when 
the furnace should be activated.

Required Frameworks
-------------------

There are two frameworks that are not standard include libraries:

 - *SHT1x* is a library for reading the humidity/temperature sensor. It is [available for download] (http://www.dfrobot.com/image/data/DFR0066/SHT1x_Arduino%20Compatible%20with%20Arduino%20IDE%201.0%20or%20later.zip) from dfrobots.

 - *TextFinder* is a stream parser specifically for handling ethernet client and server data. [grab it] (http://playground.arduino.cc/Code/TextFinder) from the Arduino site.