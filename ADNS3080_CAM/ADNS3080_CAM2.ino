#include <SPI.h> 
# include "ports.h"

#define BIT_0 0x01         // bit 0 (0000 0001)
#define BIT_1 0x02         // bit 1 (0000 0010)
#define BIT_2 0x04         // bit 2 (0000 0100)
#define BIT_3 0x08         // bit 3 (0000 1000)
#define BIT_4 0x10         // bit 4 (0001 0000)
#define BIT_5 0x20         // bit 5 (0010 0000)
#define BIT_6 0x40         // bit 6 (0100 0000)
#define BIT_7 0x80         // bit 7 (1000 0000)


#define ADNS3080_PRDT_ID 0x00
#define ADNS3080_PRDT_ID_INV 0x3f
#define ADNS3080_REV_ID 0x01

#define ADNS3080_MOTION_RD 0x01
#define ADNS3080_MOTION_CLR 0x12
#define ADNS3080_MOTION_BRST 0x50
#define ADNS3080_DELTA_X 0x03
#define ADNS3080_DELTA_Y 0x04
#define ADNS3080_SQUAL 0x05             // Surface Quality

#define ADNS3080_PIXELS_X 30
#define ADNS3080_PIXELS_Y 30
#define ADNS3080_PXL_SUM 0x06
#define ADNS3080_PXL_MAX 0x07
#define ADNS3080_PXL_BRST 0x40

#define ADNS3080_CONFIG_A 0x0a          // R_W
#define ADNS3080_CONFIG_B 0x0b          // R_W

#define ADNS3080_DATA_LW 0x0c
#define ADNS3080_DATA_UP 0x0d
#define ADNS3080_SHTR_LW 0x0e
#define ADNS3080_SHTR_UP 0x0f
#define ADNS3080_SHTR_MAX_LW 0x1d       // R_W
#define ADNS3080_SHTR_MAX_UP 0x1e       // R_W

#define ADNS3080_FCAPTURE 0x13          // R_W
#define ADNS3080_FPERIOD_LW 0x10
#define ADNS3080_FPERIOD_UP 0x11
#define ADNS3080_FPERIOD_MAX_LW 0x19    // R_W
#define ADNS3080_FPERIOD_MAX_UP 0x1a    // R_W
#define ADNS3080_FPERIOD_MIN_LW 0x1b    // R_W
#define ADNS3080_FPERIOD_MIN_UP 0x1c    // R_W

#define ADNS3080_SROM_EN 0x14
#define ADNS3080_SROM_ID 0x1f
#define ADNS3080_SROM_LD 0x60
#define ADNS3080_OBS 0x3d               // R_W

// SPI pin assignment
const int SPI_SCLK = 13;
const int SPI_MISO = 12;
const int SPI_MOSI = 11;
const int SPI_CS = 10;                  // chip-select
const int RESET_PIN = 9;

struct
{
  byte motion = 0;
  static int x, y;
  int dx, dy = 0;
  unsigned int sQual = 0;
  unsigned int shutterUp = 0;
  unsigned int shutterLw = 0;
  unsigned int maxPixel = 0;
}motionData;

SPISettings spiConfig(2000000, MSBFIRST, SPI_MODE3); //spi config 2MHz clk








void setup(void)
{
  Serial.begin(115200);
  while (!Serial);                  // Wait for serial port to open

  SPI.begin();

  // Set CS and reset pin as output
  pinMode(SPI_CS, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  SensorInit();
}


void loop(void)
{

}


// writes byte to device register
void SpiWrite(byte addr, byte data) 
{
  SPI.beginTransaction(spiConfig);
  digitalWrite(SPI_CS, LOW);

  SPI.transfer(addr |= BIT_7);      // set bit 7 to write
  delayMicroseconds(1);             // wait minimum 1 us
  SPI.transfer(data);               // write data

  digitalWrite(SPI_CS, HIGH);
  SPI.endTransaction();
}


// reads byte from device register
void SpiRead(byte addr, byte *data, int length) 
{
  SPI.beginTransaction(spiConfig);
  digitalWrite(SPI_CS, LOW);

  SPI.transfer(addr &= ~BIT_7);     // Clear bit 7 to read
  delayMicroseconds(75);            // wait minimum 75 us in case writing to Motion or Motion_Burst registers 
  memset(data, 0, length);          // Make sure data buffer is 0
  SPI.transfer(data, length);       // Write data

  digitalWrite(SPI_CS, HIGH);
  SPI.endTransaction();
}


int SensorInit(void)
{
  byte productID, configA, configB;

  SpiRead(ADNS3080_PRDT_ID, productID, 1);
  if (productID == 0x17)
  {
    Serial.println("ADNS-3080 found");
  } else 
  {
    Serial.print("Could not find ADNS-3080: ");
    Serial.println(productID, HEX);
    while (1) {};                   // Halt program
  }

  // save current config
  SpiRead(ADNS3080_CONFIG_A, configA, 1);
  SpiRead(ADNS3080_CONFIG_B, configB, 1);
}


// resets the sensor by pulling the reset pin high
void SensorReset(void) 
{
  digitalWrite(RESET_PIN, HIGH);
  delay(1);                         // reset pulse >10us
  digitalWrite(RESET_PIN, LOW);
  delay(35);                        // 35ms from reset to functional
}


// returns Motion, Delta_X, Delta_Y, SQUAL, Shutter_Upper, 
// Shutter_Lower and Maximum_Pixel very quickly
void BurstMotion(void)
{
  byte buf[7]; 
  SpiRead(ADNS3080_MOTION_BRST, buf, 7);

  motionData.motion = buf[0];
  motionData.dx = buf[1];
  motionData.dy = buf[2];
  motionData.x += motionData.dx;
  motionData.y += motionData.dy;
  motionData.sQual = buf[3];
  motionData.shutterUp = buf[4];
  motionData.shutterLw = buf[5];
  motionData.maxPixel = buf[6];
  delay(10);                        // tbexit >10 to leave burst mode
}


// clears Delta_X, Delta_Y, and internal motion registers  ?check timing
void ClearMotion() {
  SpiWrite(ADNS3080_MOTION_CLR, 0xFF); // Writing anything to this register will clear the sensor's motion registers
  motionData.x = motionData.y = 0;
}


// download a full array of pixel values from a single frame
void FrameCapture(void)
{
  byte buf;
  bool isFirstPixel = true;
  // write to the Frame_Capture register to trigger capture
  SpiWrite(ADNS3080_FCAPTURE, 0x83);
  
  // Wait 3 frame periods + 10 nanoseconds for frame to be captured
  delayMicroseconds(1510); // tCAPTURE = 10µs + 3 frame period

  // each frame is 30 x 30 pixels
  int i, j;
  for (i = 0; i < 30; i++)
  {
    for (i = 0; j < 30; j++)
    {
      SpiRead(ADNS3080_FCAPTURE, buf, 1);
      if (isFirstPixel && !(buf & BIT_6)) // validate first pixel 
      {
        Serial.println("Failed to find first pixel");
        SensorReset();
        return;
      }
      isFirstPixel = false;
      Serial.print(buf &= ~(BIT_7|BIT_6), HEX); // clear bit 6 and 7; only used for error checking
    }
  }
  SensorReset();  // hardware reset is required to restore navigation
  Serial.println();
  Serial.flush();
}


// load firmwire file to the device (incomplete)
void SROMDownload (void)
{
  SensorReset();
  SpiWrite(0x20, 0x44);
  SpiWrite(0x23, 0x07);
  SpiWrite(0x24, 0x88);
  // ? Wait at least 1 frame period + tncs sclk
  SpiWrite(ADNS3080_SROM_EN, 0x18);           // SROM enable

  delay(3); // ?
  int i;
  byte data = 0;
  for (i = 0; i < 1986; i++)
  {
    SpiWrite(data, 0x18);
  }
  delay(4); // tbexit >= 4us
  // raise the NCS line for at least tBEXIT to terminate burst mode.
  // The read may be aborted at any time by raising NCS.
  // ? read to confirm write was successful
}


