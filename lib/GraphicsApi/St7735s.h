#ifndef _ST7735S_H_
#define _ST7735S_H_

#include <cstdint>
#include <mbed.h>

/**
 * ST7735S LCD controller driver module
 */

class St7735s {
public:
  St7735s(SPI& spi, DigitalOut& cs, DigitalOut& rs, DigitalOut& reset);
  void init(uint8_t width, uint8_t height, uint8_t start_x, uint8_t start_y);

protected:
  inline void cmd(uint8_t command, size_t data_len, uint8_t* data) {
    cs_ = 0;

    rs_ = 0;  // 0 for command data, 1 for display data
    spi_.write(command);
    rs_ = 1;
    for (size_t i=0; i<data_len; i++) {
      spi_.write(data[i]);
    }

    cs_ = 1;
  }

  void set_window(uint8_t width, uint8_t height, uint8_t start_x, uint8_t start_y);

  SPI &spi_;
  DigitalOut &cs_, &rs_, &reset_;

  enum Cmd {
    SLPIN = 0x10,
    SLPOUT = 0x11,
    PTLON = 0x12,
    NORON = 0x13,
    INVOFF = 0x20,
    INVON = 0x21,
    GAMSET = 0x26,
    DISPOFF = 0x28,
    DISPON = 0x29,
    CASET = 0x2A,
    RASET = 0x2B,
    RAMWR = 0x2C,
    RGBSET = 0x2D,
    RAMRD = 0x2E,

    MADCTL = 0x36,
    COLMOD = 0x3A,  // pixel mode
  };

  enum PixelFormat {
    IFPF_12B = 3,
    IFPF_16B = 5,
    IFPF_18B = 6,
    IFPF_UNUSED = 7,
  };

  enum MemoryAccess {
    MADCTL_MX = 1 << 6,  // mirror X
    MADCTL_MY = 1 << 7,  // mirror Y
    MADCTL_MV = 1 << 5,  // transpose
    MADCTL_ML = 1 << 4,  // transpose
    MADCTL_BGR = 1 << 3,  // transpose
    MADCTL_MH = 1 << 2,  // transpose
  };
};

#endif
