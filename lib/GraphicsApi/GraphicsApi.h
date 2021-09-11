#ifndef _GRAPHICS_API_H_
#define _GRAPHICS_API_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <stdlib.h>

// TODO font
class GraphicsFont {
public:
  virtual uint8_t getFontHeight() = 0;  // return the font height, in pixels

  virtual const uint8_t* getCharData(char in) = 0;  // returns a pointer to the character bit data
  virtual uint8_t getCharWidth(char in) = 0;  // returns the width of the character, in pixels
};

class GeneratorFont : public GraphicsFont {
public:
  GeneratorFont(const uint8_t** charData, const uint8_t* charWidths, uint8_t height, uint8_t maxWidth) :
    charData_(charData), charWidths_(charWidths), height_(height), maxWidth_(maxWidth) {
  }

  uint8_t getFontHeight() {
    return height_;
  }

  const uint8_t* getCharData(char in) {
    if (in < 32 || in > 126) {
      return NULL;
    }
    return charData_[in - 32];
  }
  uint8_t getCharWidth(char in) {
    if (in < 32 || in > 126) {
      return 0;
    }
    return charWidths_[in - 32];
  }

private:
  const uint8_t** const charData_;
  const uint8_t* const charWidths_;
  const uint8_t height_;
  const uint8_t maxWidth_;
};

/**
 * Simple API for graphical displays, supporting color, monochrome, and one-bit displays.
 *
 * TODO: color display API
 *
 * Conventions:
 * Color is an optional argument on functions:
 * - if not specified: it defaults to the highest contrast color
 * - if specified as 8-bit int: used as the monochrome contrast, with 0 being lowest and 255 being highest
 * - if specified as a color: TODO
 * - colors are quantized as necessary, and colors are converted to monochrome by averaging the R, G, B components
 * Coordinates are defined with (0, 0) being the top left, increasing downwards and rightwards
 * - operations that exceed these bounds wrap
 */
class GraphicsApi {
public:
  /**
   * System-level commands
   */
  // Sends the framebuffer to the display
  virtual void update() = 0;

  /**
   * Parameters
   */
  // Returns the display width, in pixels
  virtual uint16_t getWidth() = 0;

  // Returns the display height, in pixels
  virtual uint16_t getHeight() = 0;

  /**
   * High level graphics operations
   */
  // Clears the framebuffer
  virtual void clear() {
    rectFilled(0, 0, getWidth() - 1, getHeight() - 1, 0);
  }

  // Draw a rectangle, coordinates are inclusive
  void rect(uint16_t x, uint16_t y, int16_t w, int16_t h) {
    rect(x, y, w, h, 255);
  }
  virtual void rect(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t contrast) = 0;

  // Draw a filled rectangle, coordinates are inclusive
  void rectFilled(uint16_t x, uint16_t y, int16_t w, int16_t h) {
    rectFilled(x, y, w, h, 255);
  }
  virtual void rectFilled(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t contrast) = 0;

  // Draw a line, using a fast line algorithm
  void line(uint16_t x, uint16_t y, int16_t w, int16_t h) {
    line(x, y, w, h, 255);
  }
  virtual void line(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t contrast) = 0;

  // Draws text (null-terminated string), the specified location is the top left of the text drawn
  // Returns the horizontal size, in pixels, of the text drawn
  uint16_t text(uint16_t x, uint16_t y, const char* string, GraphicsFont& font) {
    return text(x, y, string, font, 255);
  }
  virtual uint16_t text(uint16_t x, uint16_t y, const char* string, GraphicsFont& font, uint8_t contrast) = 0;

  // TODO: bitmapped image operations
};

class PixelGraphics : public GraphicsApi {
public:
  void rect(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t contrast = 255) {
    uint16_t x2 = x + w;
    uint16_t y2 = y + h;
    if (x2 < x) {
      std::swap(x, x2);
    }
    if (y2 < y) {
      std::swap(y, y2);
    }

    for (uint16_t xPos=x; xPos<x2; xPos++) {
      drawPixel(xPos, y, contrast);
      drawPixel(xPos, y2, contrast);
    }

    if ((y2 - y) > 1) {
      for (uint16_t yPos=y; yPos<y2; yPos++) {
        drawPixel(x, yPos, contrast);
        drawPixel(x2, yPos, contrast);
      }
    }
  }

  void rectFilled(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t contrast = 255) {
    uint16_t x2 = x + w;
    uint16_t y2 = y + h;
    if (x2 < x) {
      std::swap(x, x2);
    }
    if (y2 < y) {
      std::swap(y, y2);
    }

    for (uint16_t yPos=y; yPos<y2; yPos++) {
      for (uint16_t xPos=x; xPos<x2; xPos++) {
        drawPixel(xPos, yPos, contrast);
      }
    }
  }

  void line(uint16_t x, uint16_t y, int16_t w, int16_t h, uint8_t contrast = 255) {
    int16_t dx = abs(w);
    int16_t dy = abs(h);

    int16_t xIncr = w < 0? -1 : 1;
    int16_t yIncr = h < 0? -1 : 1;

    if (dx >= dy) {
      // in x-major
      int16_t d = 2 * dy - dx;
      uint16_t yPos = y;
      uint16_t x2 = x + w;
      for (uint16_t xPos=x; xPos!=x2; xPos+=xIncr) {
        drawPixel(xPos, yPos, contrast);
        if (d > 0) {
          yPos += yIncr;
          d -= 2 * dx;
        }
        d += 2 * dy;
      }
    } else {
      // in y-major
      int16_t d = 2 * dx - dy;
      uint16_t xPos = x;
      uint16_t y2 = y + h;
      for (uint16_t yPos=y; yPos!=y2; yPos+=yIncr) {
        drawPixel(xPos, yPos, contrast);
        if (d > 0) {
          xPos += xIncr;
          d -= 2 * dy;
        }
        d += 2 * dx;
      }
    }
  }

  uint16_t text(uint16_t x, uint16_t y, const char* string, GraphicsFont& font, uint8_t contrast = 255) {
    uint16_t origx = x;
    for (; *string != 0; string++) {
      const uint8_t* charData = font.getCharData(*string) - 1;
      uint8_t charWidth = font.getCharWidth(*string);
      if (charData != NULL) {
        for (uint8_t col=0; col<charWidth; col++) {
          if (x >= getWidth()) {
            return getWidth() - origx;
          }

          for (uint8_t row=0; row<font.getFontHeight(); row++) {
            uint8_t bitPos = row % 8;
            if (bitPos == 0) {
              charData++;
            }
            if (*charData & (1 << bitPos)) {
              if (y + row < getHeight()) {
                drawPixel(x, y + row, contrast);
              }
            }
          }

          x++;
        }
        x++;  // inter-character space
      }
    }
    return x - origx - 1;  // don't count the trailing space
  }

protected:
  virtual void drawPixel(uint16_t x, uint16_t y, uint8_t contrast) = 0;
};

#endif
