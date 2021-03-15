# rpi_incubator

Egg incubator

Sensors:
  DHT22 - humidity
  DS18B20 - temperature

Rest of system:
  RTC - time measuring
  High Torque motor - rotation
      *MG996R (removed servo electronics) working on L298N driver
  2 microswitches for homing left/right rotation
  Heater with fan
    Heater - 15 resistors 150Ohm 1W on parallel attached to 12V
             switching on/off through Infenion IRFZ44N mosfet
             drived from pin 24 with voltage boosted to 5V
             with double not gate of 74HCT14 chip
    Fan - PC fan 92mm on working on 5V regulated with L7805
          switched with same mosfet
 
