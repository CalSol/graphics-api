#ifndef _SSD1322_SPI_GRAPHICS_H_
#define _SSD1322_SPI_GRAPHICS_H_

#include "St7735s.h"
#include "GraphicsApi.h"

/**
 * ST7735S using a framebuffer to expose a high level graphics API.
 *
 * templated pixel format, currently RGB444 to save memory
 */
template <uint8_t width, uint8_t height, uint8_t xOffs, uint8_t yOffs>
class St7735sGraphics: public St7735s, public PixelGraphics {
public:
  St7735sGraphics(SPI& spi, DigitalOut& cs, DigitalOut& rs, DigitalOut& reset) :
    St7735s(spi, cs, rs, reset) {
  }

  void init() {  // wrapper around St7735s::init that passes through template args
    St7735s::init(width, height, xOffs, yOffs);
  }

  void update() {
    cmd(Cmd::RAMWR, width * height * 3 / 2, framebuffer_);
  }

  uint16_t getWidth() {
    return width;
  }
  uint16_t getHeight() {
    return height;
  }

  void drawPixel(uint16_t x, uint16_t y, uint8_t contrast) {
    uint8_t contrast4 = contrast >> 4;

    if (x >= width || y >= height) {
      return;
    }

    if (x % 2 == 0) {
      framebuffer_[(y * width * 3 / 2)+(x * 3 / 2)] = contrast4 | (contrast4 << 4);
      framebuffer_[(y * width * 3 / 2)+(x * 3 / 2) + 1] &= 0xf0;
      framebuffer_[(y * width * 3 / 2)+(x * 3 / 2) + 1] |= contrast4 << 4;
    } else {
      framebuffer_[(y * width * 3 / 2)+(x * 3 / 2)] &= 0x0f;
      framebuffer_[(y * width * 3 / 2)+(x * 3 / 2)] |= contrast4;
      framebuffer_[(y * width * 3 / 2)+(x * 3 / 2) + 1] = contrast4 | (contrast4 << 4);
    }
  }

  void clear() {  // optimized clear that understands the framebuffer format
    memset(framebuffer_, 0, sizeof(framebuffer_));
  }

protected:
    uint8_t framebuffer_[width * height * 3 / 2];  // in display memory order, word=(RGB 444), x (row), y (col)
};

#endif
