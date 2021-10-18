#ifndef _SSD1322_SPI_H_
#define _SSD1322_SPI_H_

#include <cassert>
#include "mbed.h"

class Ssd1322Spi {
public:
  Ssd1322Spi(SPI& spi, DigitalOut& cs, DigitalOut &dc, DigitalOut& reset) :
      spi_(spi), cs_(cs), dc_(dc), reset_(reset) {
    reset_ = 0;
    cs_ = 1;
  }

  /**
   * Initialize the display. Call before any other commands.
   */
  void init();

  /**
   * Begins a write to the display RAM, specifying the boundaries at which columns
   * and rows wrap around.
   * col_start and col_end must be divisible by 4, otherwise will truncate.
   * Call writeTwoPixels after this to stream data to the display.
   * Holds the CS line asserted until endWrite() is called.
   */
  void beginWrite(uint8_t col_start, uint8_t col_end, uint8_t row_start, uint8_t row_end);

  /**
   * Ends a write, releases the CS line.
   */
  void endWrite();

protected:
  // Send a command byte
  void command(uint8_t payload);

  // Send a data byte
  void data(uint8_t payload);

  SPI& spi_;
  DigitalOut& cs_;
  DigitalOut& dc_;
  DigitalOut& reset_;

  enum Command {
    SET_COLUMN_ADDRESS = 0x15,
    WRITE_RAM = 0x5C,
    READ_RAM = 0x5D,
    SET_ROW_ADDRESS = 0x75,

    SET_REMAP = 0xA0,
    SET_DISPLAY_START_LINE = 0xA1,
    SET_DISPLAY_OFFSET = 0xA2,

    DISPLAY_MODE_OFF = 0xA4,
    DISPLAY_MODE_ON = 0xA5,
    DISPLAY_MODE_NORMAL = 0xA6,
    DISPLAY_MODE_INVERSE = 0xA7,

    FUNCTION_SELECTION = 0xAB,
    DISPLAY_OFF = 0xAE,
    DISPLAY_ON = 0xAF,
    SET_PHASE_LENGTH = 0xB1,
    SET_DISPLAY_CLOCK = 0xB3,
    DISPLAY_ENHANCEMENT_A = 0xB4,
    SET_GPIO = 0xB5,
    SET_SECOND_PRECHARGE_PERIOD = 0xB6,
    SELECT_DEFAULT_LINEAR_GRAYSCALE_TABLE = 0xB9,

    SET_PRECHARGE_VOLTAGE = 0xBB,
    SET_VCOMH_VOLTAGE = 0xBE,
    SET_CONSTRAST_CONTROL = 0xC1,
    MASTER_CONSTRAST_CONTROL = 0xC7,
    SET_MULTIPLEX_RATIO = 0xCA,
    SET_COMMAND_LOCK = 0xFD,

  };
};

#endif
