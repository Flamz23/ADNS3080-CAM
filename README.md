# ADNS3030-CAM
This project turns the ADNS 3080 optical-flow sensor into a 30 by 30 pixel camera. It was inspired by example code provided by [Lauszus](https://github.com/Lauszus) in this [repository](https://github.com/Lauszus/ADNS3080).

## To Use

The ADNS 3080 communicates via Serial Peripheral Interface (SPI) and the sensor responds with the pixel data for each frame.

 1. Clone the repository and run the arduino code. The arduino writes frames over serial at 115200 baud.
 2. Run the executable and select the COM port to use.

For more info on how to use this device refer to its [documentation](https://github.com/Flamz23/ADNS3080-CAM/blob/main/docs/adns_3080.pdf).