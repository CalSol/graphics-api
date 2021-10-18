#ifndef _SSD1322_SPI_GRAPHICS_H_
#define _SSD1322_SPI_GRAPHICS_H_

#include <utility>

#include "Ssd1322Spi.h"
#include "GraphicsApi.h"

/**
 * SSD1322 over SPI using a framebuffer to expose a high level graphics API.
 *
 * TODO: don't make it a fixed 256*64 width
 */
class Ssd1322SpiGraphics: public Ssd1322Spi, public PixelGraphics {
public:
  Ssd1322SpiGraphics(SPI& spi, DigitalOut& cs, DigitalOut &dc, DigitalOut& reset) :
    Ssd1322Spi(spi, cs, dc, reset) {
  }

  void update() {
    beginWrite(0, 255, 0, 63);
    dc_ = 1;
    for (size_t i=0; i<sizeof(framebuffer_); i++) {
      spi_.write(framebuffer_[i]);
    }
    endWrite();
  }

  uint16_t getWidth() {
    return 256;
  }
  uint16_t getHeight() {
    return 64;
  }

  void drawPixel(uint16_t x, uint16_t y, uint8_t contrast) {
    bool writeMsNibble = (x % 2) == 0;
    contrast = contrast >> 4;
    framebuffer_[(y*128)+(x/2)] &= writeMsNibble ? 0x0f : 0xf0;  // unset pixel
    framebuffer_[(y*128)+(x/2)] |= contrast << (writeMsNibble ? 4 : 0);  // set pixel
  }

protected:
  uint8_t framebuffer_[256*64/2];  // in display memory order, byte=(MsNibble=1, LsNibble=0), x (row), y (col)
};

#endif
