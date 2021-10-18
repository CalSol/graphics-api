#ifndef _EINK_H_
#define _EINK_H_

#include <cassert>
#include "mbed.h"
#include "GraphicsApi.h"

class EInk152 {
public:
  EInk152(SPI& spi, DigitalOut& cs, DigitalOut &dc, DigitalOut& reset, DigitalIn& busy) :
      spi_(spi), cs_(cs), dc_(dc), reset_(reset), busy_(busy) {
    reset_ = 0;
    cs_ = 1;
  }

  const uint32_t kBusyTimeoutUs = 100 * 1000;

  void init() {
    cs_ = 0;
    dc_ = 0;
    reset_ = 0;
    wait_us(5 * 1000);
    reset_ = 1;
    cs_ = 1;

    command(0x00, (const uint8_t[]){0x0e}, 1);  // soft reset

  //  eink_command(0x06, (const uint8_t[]){0x17, 0x17, 0x17}, 3);  // booster soft start
  //  eink_command(0x63, (const uint8_t[]){0x80, 0x01, 0x28}, 3);  // resolution
  //  eink_command(0x50, (const uint8_t[]){0x87}, 1);  // Vcom / data interval
    command(0xe0, (const uint8_t[]){0x02}, 1);  // active temperature
    command(0xe5, (const uint8_t[]){0x19}, 1);  // input temperature: 25c
  }

  void draw(uint8_t blackFrame[], uint8_t redFrame[]) {
    Timer busyTimeout;

    if (blackFrame == NULL) {
      commandConstPayload(0x10, 0x00, 2888);
    } else {
      command(0x10, blackFrame, 2888);
    }

    if (redFrame == NULL) {
      commandConstPayload(0x13, 0x00, 2888);
    } else {
      command(0x13, redFrame, 2888);
    }


    // TODO check busy = 0

    command(0x04, NULL, 0);  // power on
    busyTimeout.start();
    while ((busy_ == 0) && ((unsigned int)busyTimeout.read_us() < kBusyTimeoutUs));
//    wait_us(kBusyTimeoutUs);

    command(0x12, NULL, 0);  // refresh
//    while (busy_ == 0);
  }

protected:
  void command(uint8_t index, const uint8_t payload[], size_t len) {
    spi_.format(8, 0);
    spi_.frequency(10 * 1000 * 1000);
    dc_ = 0;
    cs_ = 0;
    wait_us(1);
    spi_.write(index);
    wait_us(1);
    dc_ = 1;
    cs_ = 1;
    for (size_t i=0; i<len; i++) {
      wait_us(1);
      cs_ = 0;
      wait_us(1);
      spi_.write(payload[i]);
      wait_us(1);
      cs_  = 1;
    }
  }

  void commandConstPayload(uint8_t index, uint8_t payloadRepeat, size_t len) {
    spi_.format(8, 0);
    spi_.frequency(10 * 1000 * 1000);
    dc_ = 0;
    cs_ = 0;
    wait_us(1);
    spi_.write(index);
    wait_us(1);
    dc_ = 1;
    cs_ = 1;
    for (size_t i=0; i<len; i++) {
      wait_us(1);
      cs_ = 0;
      wait_us(1);
      spi_.write(payloadRepeat);
      wait_us(1);
      cs_  = 1;
    }
  }

  SPI& spi_;
  DigitalOut& cs_;
  DigitalOut& dc_;
  DigitalOut& reset_;
  DigitalIn& busy_;
};

class EInk152Graphics : public EInk152, public PixelGraphics {
public:
  EInk152Graphics(SPI& spi, DigitalOut& cs, DigitalOut &dc, DigitalOut& reset, DigitalIn& busy) :
      EInk152(spi, cs, dc, reset, busy) {
  }


  void update() {
    draw(NULL, frameBuffer_);
  }

  uint16_t getWidth() {
    return 152;
  }
  uint16_t getHeight() {
    return 152;
  }


  void drawPixel(uint16_t x, uint16_t y, uint8_t contrast) {
    if ((x >= 152) || (y >= 152)) {  // out of bounds
      return;
    }
    uint8_t* bufferByte = frameBuffer_ + (y * (152/8) + (x / 8));
    uint8_t bufferBitMask = 1 << (7 - (x%8));
    if (contrast < 127) {
      *bufferByte = *bufferByte & ~bufferBitMask;
    } else {
      *bufferByte = *bufferByte | bufferBitMask;
    }
  }


protected:
  uint8_t frameBuffer_[2888] = {0};
};

#endif
