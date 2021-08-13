#include <SPI.h>

// configure spi for 2MHz max speed, data order and spi mode
SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE3);

// register for motion burst(fastest capture to motion mode)
#define ADNS3080_CONFIGURATION_BITS 0x0a
#define ADNS3080_DATA_OUT_LOWER 0x0c
#define ADNS3080_DATA_OUT_UPPER 0x0d
#define ADNS3080_DELTA_X  0x03
#define ADNS3080_DELTA_Y  0x04
#define ADNS3080_EXTENDED_CONFIG  0x0b
#define ADNS3080_FRAME_CAPTURE  0x13
#define ADNS3080_FRAME_PERIOD_LOWER 0x10
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER 0x19
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER 0x1a
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER 0x1b
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER 0x1c
#define ADNS3080_FRAME_PERIOD_UPPER 0x11
#define ADNS3080_INVERSE_PRODUCT_ID 0x3f
#define ADNS3080_MAXIMUM_PIXEL  0x07
#define ADNS3080_MOTION 0x02
#define ADNS3080_MOTION_BURST 0x50
#define ADNS3080_MOTION_CLEAR 0x12
#define ADNS3080_OBSERVATION 0x3d
#define ADNS3080_PIXEL_BURST 0x40
#define ADNS3080_PIXEL_SUM 0x06
#define ADNS3080_PRODUCT_ID 0x00
#define ADNS3080_PRODUCT_ID_VALUE 0x17 // ADNS3080_PRODUCT_ID register
#define ADNS3080_REVISION_ID 0x01
#define ADNS3080_SHUTTER_LOWER 0x0e
#define ADNS3080_SHUTTER_MAX_BOUND_LOWER 0x1e
#define ADNS3080_SHUTTER_MAX_BOUND_UPPER 0x1e
#define ADNS3080_SHUTTER_UPPER 0x0f
#define ADNS3080_SQUAL 0x05
#define ADNS3080_SROM_ENABLE 0x14
#define ADNS3080_SROM_ID 0x1f
#define ADNS3080_SROM_LOAD 0x60

#define ADNS3080_PIXELS_X 30
#define ADNS3080_PIXELS_Y 30

/* refer to https://www.arduino.cc/en/reference/SPI for spi connection conventions */
const int RESET_PIN = 9;
const int SS_PIN = SS; // Pin 10 Slave select pin (NCS on board) [Active low]
int x, y;







void sensor_reset() {
  digitalWrite(RESET_PIN, HIGH);
  delay(1); // reset pulse >10us
  digitalWrite(RESET_PIN, LOW);
  delay(35); // 35ms from reset to functional
}

//Verify that the serial communications link is functional
int sensor_init() {
  unsigned int pid = spiRead(ADNS3080_PRODUCT_ID); // Send dummy value

  if (pid == ADNS3080_PRODUCT_ID_VALUE)
    Serial.println("ADNS-3080 found");
  else {
    Serial.print("Could not find ADNS-3080: ");
    Serial.println(pid, HEX);
    while (1) {}; // Halt program
  }

  unsigned int conf = spiRead(ADNS3080_CONFIGURATION_BITS); // default value [0x09]
  spiWrite(ADNS3080_CONFIGURATION_BITS, conf | 0x10); // Set resolution to 1600 counts per inch
  return 0;
}

int sensor_frame_capture() {
  bool isFirstPixel = true;
  spiWrite(ADNS3080_FRAME_CAPTURE, 0x83); // Write 0x83 to frame capture register to start capturing frame

  // Wait 3 frame periods + 10 nanoseconds for frame to be captured
  delayMicroseconds(1510); // Minimum frame speed is 2000 frames/second so 1 frame = 500 nano seconds. So 500 x 3 + 10 = 1510

  // write pixel data to serial
  for (int i = 0; i < ADNS3080_PIXELS_Y; i++) {
    for (int j = 0; j < ADNS3080_PIXELS_X; j++) {
      // read frame capture register (register returns zero byte if frame capture is not complete)
      uint8_t regValue = spiRead(ADNS3080_FRAME_CAPTURE);
      
      if (isFirstPixel && !(regValue & 0x40)) { 
        Serial.println("Failed to find first pixel");
        return -1;
      }

      isFirstPixel = false;
      uint8_t pixelValue = regValue << 2; // Only lower 6 bits have data ("& 0x3f" nulls last 2 digita and "<< 2" shifts)
      Serial.write(pixelValue);
    }
    Serial.flush();
  }
  return 0;
}


unsigned int spiRead(byte reg) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(reg);          // Send register address
  delayMicroseconds(75);      // Wait minimum 75 us in case writing to Motion or Motion_Burst registers
  const int response = SPI.transfer(0xff); // Transfer data and read response
  digitalWrite(SS_PIN, HIGH);
  SPI.endTransaction();
  return response;
}

void spiWrite(byte reg, int data) {
  SPI.beginTransaction(spiSettings);
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(reg | 0x80); // Send register address
  delayMicroseconds(1);  // Wait minimum 1 us
  SPI.transfer(data); // Transfer data
  digitalWrite(SS_PIN, HIGH);
  SPI.endTransaction();
}








void setup() {
  pinMode(RESET_PIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {}; // Wait for serial port to open
  SPI.begin();

  sensor_reset();
  sensor_init();
  Serial.println();
}

void loop() {
  sensor_frame_capture();
  Serial.println();
}
