#include "Ssd1322Spi.h"

void Ssd1322Spi::init() {
  spi_.frequency(10000000);

  reset_ = 1;

  cs_ = 0;

  command(Command::SET_COMMAND_LOCK);
  data(0x12);

  command(Command::DISPLAY_OFF);

  command(Command::SET_DISPLAY_CLOCK);
  data(0x91);
  command(Command::SET_MULTIPLEX_RATIO);
  data(0x3F);
  command(Command::SET_DISPLAY_OFFSET);
  data(0x00);
  command(Command::SET_DISPLAY_START_LINE);
  data(0x00);
  command(Command::SET_REMAP);
  data(0x14);
  data(0x00);

  command(Command::SET_GPIO);
  data(0x00);
  command(Command::FUNCTION_SELECTION);
  data(0x01);

  command(Command::DISPLAY_ENHANCEMENT_A);
  data(0xA0);
  data(0xFD);

  command(Command::SET_CONSTRAST_CONTROL);
  data(0xFF);
  data(0xFF);
  data(0xFF);
  command(Command::MASTER_CONSTRAST_CONTROL);
  data(0x0F);

  command(Command::SELECT_DEFAULT_LINEAR_GRAYSCALE_TABLE);

  command(Command::SET_PHASE_LENGTH);
  data(0xE2);
  command(Command::SET_PRECHARGE_VOLTAGE);
  data(0x1F);
  command(Command::SET_SECOND_PRECHARGE_PERIOD);
  data(0x08);
  command(Command::SET_VCOMH_VOLTAGE);
  data(0x07);

  command(Command::DISPLAY_MODE_NORMAL);
  command(Command::DISPLAY_ON);

  cs_ = 1;
}

void Ssd1322Spi::beginWrite(uint8_t col_start, uint8_t col_end, uint8_t row_start, uint8_t row_end) {
  assert(row_end < 64);

  cs_ = 0;

  command(Command::SET_COLUMN_ADDRESS);
  data(0x1c + col_start/4);
  data(0x1c + col_end/4);
  command(Command::SET_ROW_ADDRESS);
  data(row_start);
  data(row_end);

  command(Command::WRITE_RAM);
}

void Ssd1322Spi::endWrite() {
  cs_ = 1;
}

void Ssd1322Spi::command(uint8_t payload) {
  dc_ = 0;
  // technically, there should be a 15ns delay here
  spi_.write(payload);
}

void Ssd1322Spi::data(uint8_t payload) {
  dc_ = 1;
  // technically, there should be a 15ns delay here
  spi_.write(payload);
}
