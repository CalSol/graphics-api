#include "St7735s.h"

St7735s::St7735s(SPI& spi, DigitalOut& cs, DigitalOut& rs, DigitalOut& reset) :
  spi_(spi), cs_(cs), rs_(rs), reset_(reset) {
  reset_ = 0;
  cs_ = 1;
}

void St7735s::init(uint8_t width, uint8_t height, uint8_t start_x, uint8_t start_y) {
  spi_.frequency(15 * 1000 * 1000);  // write up to 15 MHz (66ns cycle)
  reset_ = 0;
  wait_us(10);  // reset pulse
  reset_ = 1;
  wait_ms(120);  // wait for LCD to reset

  cmd(Cmd::SLPOUT, 0, {});

  uint8_t colmod_data[] = {PixelFormat::IFPF_12B};
  cmd(Cmd::COLMOD, 1, colmod_data);
  uint8_t madctl_data[] = {MemoryAccess::MADCTL_MY | MemoryAccess::MADCTL_MV | MemoryAccess::MADCTL_BGR};
  cmd(Cmd::MADCTL, 1, madctl_data);
  set_window(width, height, start_x, start_y);

  cmd(Cmd::DISPON, 0, {});

  cmd(Cmd::INVON, 0, {});
}

void St7735s::set_window(uint8_t width, uint8_t height, uint8_t start_x, uint8_t start_y) {
  uint8_t caset_data[] = {0, start_x, 0, (uint8_t)(start_x + width - 1)};
  cmd(Cmd::CASET, 4, caset_data);

  uint8_t raset_data[] = {0, start_y, 0, (uint8_t)(start_y + height - 1)};
  cmd(Cmd::RASET, 4, raset_data);
}
