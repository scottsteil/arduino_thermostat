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