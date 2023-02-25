#include <SPI.h> 

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
#define SPI_SCLK 13
#define SPI_MISO 12
#define SPI_MOSI 11
#define SPI_CS 10                       // chip-select


SPISettings spiConfig(2000000, MSBFIRST, SPI_MODE3);      //spi config 2MHz clk




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
byte SpiRead(byte addr) 
{
  const byte response;
  SPI.beginTransaction(spiConfig);
  digitalWrite(SPI_CS, LOW);

  SPI.transfer(addr &= ~BIT_7);     // Clear bit 7 to read
  delayMicroseconds(75);            // wait minimum 75 us
  response = SPI.transfer(0xff);    // write dummy and read response

  digitalWrite(SPI_CS, HIGH);
  SPI.endTransaction();
}


int DeviceInit(void)
{

}

// return Motion, Delta_X, Delta_Y, SQUAL, Shutter_Upper, 
// Shutter_Lower and Maximum_Pixel very quickly
byte* BurstMotion(void)
{
  const defVal = SpiRead(ADNS3080_MOTION_BRST);
}



void setup(void)
{

}

void loop(void)
{

}
