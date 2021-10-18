#include "DefaultFonts.h"
#include "mbed.h"


struct Size {
  Size(uint16_t width=0, uint16_t height=0) : width(width), height(height) {
  }

  uint16_t width;
  uint16_t height;
};


class Widget {
public:
  virtual Size layout() = 0;  // returns the size of this widget, derived from size of children
  virtual void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) = 0;  // draw at specified top-left x, y
};


template <typename T>
class ValueContrastWidget: public Widget {
public:
  virtual void setValue(T value) = 0;
  virtual void setContrast(uint8_t contrast) = 0;
};


class TextWidget: public ValueContrastWidget<const char*> {
public:
  TextWidget(const char* value, uint8_t maxLength = 0, GeneratorFont& font = Font5x7, uint8_t contrast = 255) :
      value_(value), maxLength_(maxLength), font_(font), contrast_(contrast) {
  }

  Size layout() {
    uint8_t realMaxLength = strlen(value_);
    if (maxLength_ > realMaxLength) {
      realMaxLength = maxLength_;
    }
    size_ = Size(font_.getCharWidth('8') * realMaxLength + realMaxLength - 1, font_.getFontHeight());  // TODO don't assume 1 px space
    return size_;
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    gfx.rectFilled(x, y, size_.width, size_.height, 0);  // TODO configurable background
    gfx.text(x, y, value_, font_, contrast_);
  }

  void setValue(const char* value) {
    value_ = value;
  }
  void setContrast(uint8_t contrast) {
    contrast_ = contrast;
  }

protected:
  const char* value_;
  uint8_t maxLength_;
  GeneratorFont& font_;
  uint8_t contrast_;

  Size size_;
};


class NumericTextWidget: public ValueContrastWidget<int32_t> {
public:
  NumericTextWidget(int32_t value, uint8_t maxDigits = 0,
      GeneratorFont& font = Font5x7, uint8_t contrast = 255,
      GeneratorFont& fractionalFont = Font3x5, int32_t fractional = 1, uint8_t fractionalDigits = 0) :
      maxDigits_(maxDigits), fractionalDigits_(fractionalDigits),
      fractional_(fractional), 
      contrast_(contrast),
      text_("", maxDigits_, font, contrast),
      fractionalText_("", fractionalDigits_, fractionalFont, contrast) {
    setValue(value);
  }

  void setValue(int32_t value) {
    // TODO rounding
    itoa(value / fractional_, digitsBuf_, 10, maxDigits_);
    text_.setValue(digitsBuf_);

    if (fractionalDigits_ > 0) {
      uint64_t fractionalValue;
      if (value < 0) {
        fractionalValue = -value % fractional_;
      } else {
        fractionalValue = value % fractional_;
      }
      uint32_t fractionalMax = 10;
      for (size_t i=0; (i + 1)<fractionalDigits_; i++) {
        fractionalMax *= 10;
      }

      fractionalValue = fractionalValue * fractionalMax / fractional_;

      itoa(fractionalValue, fractionalBuf_, 10, fractionalDigits_, '0');
      fractionalText_.setValue(fractionalBuf_);
    }
  }
  void setContrast(uint8_t contrast) {
    contrast_ = contrast;
    text_.setContrast(contrast);
    fractionalText_.setContrast(contrast);
  }

  Size layout() {
    if (fractionalDigits_ > 0) {
      textSize_ = text_.layout();
      fractionalSize_ = fractionalText_.layout();

      return Size(textSize_.width + 3 + fractionalSize_.width,
          textSize_.height);
    } else {
      return text_.layout();
    }
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    text_.draw(gfx, x, y);
    if (fractionalDigits_ > 0) {
      gfx.line(x + textSize_.width + 1, y + textSize_.height - 1, 1, 0, contrast_);
      fractionalText_.draw(gfx, x + textSize_.width + 3, y + textSize_.height - fractionalSize_.height);
    }
  }

protected:
  void itoa(int32_t val, char* out, uint8_t base, size_t padLength = 0, char padChar = ' ') {
    char buf[32];  // TODO could be shorter

    bool isNeg = val < 0;
    if (isNeg) {
      val = -val;
    }

    int32_t i = 31;
    if (val == 0) {
      buf[i] = '0';
      i--;
    } else {
      for (; i >= 0 && val != 0; i--) {
        buf[i] = "0123456789ABCDEF"[val % base];
        val /= base;
      }
    }

    if (isNeg) {
      *out = '-';
      out++;
      if (padLength > 0) {
        padLength--;
      }
    }
    size_t length = 31 - i;
    if (padLength > length) {
      for (size_t pad=0; pad < padLength - length; pad++) {
        *out = padChar;
        out++;
      }
    }

    i++;
    for (; i < 32; i++) {
      *out = buf[i];
      out++;
    }
    *out = '\0';
  }

  const uint8_t maxDigits_, fractionalDigits_;
  int32_t fractional_;
  uint8_t contrast_;

  char digitsBuf_[11 + 1], fractionalBuf_[11 + 1];

  TextWidget text_, fractionalText_;
  Size textSize_, fractionalSize_;
};


class LabelFrameWidget: public Widget {
public:
  LabelFrameWidget(Widget* contents, const char* title, GeneratorFont& font = Font3x5, uint8_t contrast = 255) :
  contents_(contents), title_(title), font_(font), contrast_(contrast) {
  }

  Size layout() {
    innerSize_ = contents_->layout();
    return Size(innerSize_.width + 4, innerSize_.height + 2 + 1 + font_.getFontHeight());
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    gfx.text(x + 2, y, title_, font_, contrast_);

    gfx.line(x + 1,
        y + font_.getFontHeight() + 1 + innerSize_.height + 1,
        innerSize_.width + 2, 0, contrast_);
    size_t titleLength = strlen(title_);
    uint16_t topStart = x + 2 + titleLength * font_.getCharWidth('8') + (titleLength - 1) + 1;  // TODO don't assume 1 px space
    uint16_t topEnd = x + 2 + innerSize_.width + 1;
    if (topEnd > topStart) {
      gfx.line(topStart,
          y + font_.getFontHeight() - 1,
          topEnd - topStart, 0, contrast_);
    }

    gfx.line(x,
        y + font_.getFontHeight(),
        0, innerSize_.height + 2, contrast_);
    gfx.line(x + 2 + innerSize_.width + 1,
        y + font_.getFontHeight(), 0,
        innerSize_.height + 2, contrast_);

    contents_->draw(gfx, x + 2, y +font_.getFontHeight() + 1);
  }

protected:
  Widget* const contents_;
  const char* title_;
  GeneratorFont& font_;
  uint8_t contrast_;

  Size innerSize_;
};


template <size_t N>
class HGridWidget: public Widget {
public:
  HGridWidget(Widget* contents[], uint16_t space = 1) :
      space_(space) {
    for (size_t i=0; i < N; i++) {
      contents_[i] = contents[i];
    }
  }

  Size layout() {
    uint16_t maxHeight = 0, widthSum = 0;
    for (size_t i=0; i<N; i++) {
      innerSize_[i] = contents_[i]->layout();
      if (innerSize_[i].height > maxHeight) {
        maxHeight = innerSize_[i].height;
      }
      widthSum += innerSize_[i].width;
    }
    widthSum += space_ * (N - 1);
    return Size(widthSum, maxHeight);
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    uint16_t currX = x;
    for (size_t i=0; i<N; i++) {
      contents_[i]->draw(gfx, currX, y);
      currX += innerSize_[i].width + space_;
    }
  }

protected:
  Widget* contents_[N];
  const uint16_t space_;

  Size innerSize_[N];
};


template <size_t N>
class VGridWidget: public Widget {
public:
  VGridWidget(Widget* contents[], uint16_t space = 1) :
      space_(space) {
    for (size_t i=0; i < N; i++) {
      contents_[i] = contents[i];
    }
  }

  Size layout() {
    uint16_t maxWidth= 0, heightSum = 0;
    for (size_t i=0; i<N; i++) {
      innerSize_[i] = contents_[i]->layout();
      if (innerSize_[i].width > maxWidth) {
        maxWidth= innerSize_[i].width;
      }
      heightSum += innerSize_[i].height;
    }
    heightSum += space_ * (N - 1);
    return Size(maxWidth, heightSum);
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {  // TODO allow top/center/bottom Y
    uint16_t currY = y;
    for (size_t i=0; i<N; i++) {
      contents_[i]->draw(gfx, x, currY);
      currY += innerSize_[i].height + space_;
    }
  }

protected:
  Widget* contents_[N];
  const uint16_t space_;

  Size innerSize_[N];
};


/**
 * Widget where elements are arranged on a 3x3 grid, aligned:
 * [  top-left    top-mid    top-right
 *    cen-left    cen-mid    cen-right
 *    bot-left    bot-mid    bot-right
 * ]
 * No checks are done to ensure subelements do not interfere with each other, or stay within
 * this container's width and height bounds.
 * NULL may be passed in to ignore a particular element.
 */
class FixedGridWidget: public Widget {
public:
  FixedGridWidget(Widget* contents[9], uint16_t width, uint16_t height) :
      width_(width), height_(height) {
    for (size_t i=0; i < 9; i++) {
      contents_[i] = contents[i];
    }
  }

  Size layout() {
    for (size_t i=0; i<9; i++) {
      if (contents_[i] != NULL) {
        innerSize_[i] = contents_[i]->layout();
      }
    }

    return Size(width_, height_);
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {  // TODO allow top/center/bottom Y
    if (contents_[0] != NULL) {
      contents_[0]->draw(gfx,
          x,
          y);
    }
    if (contents_[1] != NULL) {
      contents_[1]->draw(gfx,
          x + (width_ - innerSize_[1].width) / 2,
          y);
    }
    if (contents_[2] != NULL) {
      contents_[2]->draw(gfx,
          x + width_ - innerSize_[2].width,
          y);
    }

    if (contents_[3] != NULL) {
      contents_[3]->draw(gfx,
          x,
          y + (height_ - innerSize_[3].height) / 2);
    }
    if (contents_[4] != NULL) {
      contents_[4]->draw(gfx,
          x + (width_ - innerSize_[4].width) / 2,
          y + (height_ - innerSize_[4].height) / 2);
    }
    if (contents_[5] != NULL) {
      contents_[5]->draw(gfx,
          x + width_ - innerSize_[5].width,
          y + (height_ - innerSize_[5].height) / 2);
    }

    if (contents_[6] != NULL) {
      contents_[6]->draw(gfx,
          x,
          y + height_ - innerSize_[6].height);
    }
    if (contents_[7] != NULL) {
      contents_[7]->draw(gfx,
          x + (width_ - innerSize_[7].width) / 2,
          y + height_ - innerSize_[7].height);
    }
    if (contents_[8] != NULL) {
      contents_[8]->draw(gfx,
          x + width_ - innerSize_[8].width,
          y + height_ - innerSize_[8].height);
    }
  }

protected:
  Widget* contents_[9];
  const uint16_t width_, height_;

  Size innerSize_[9];
};


/**
 * Widget that changes the contrast of the contained widget when stale
 */
template <typename T>
class StaleContainerWidget {
public:
  StaleContainerWidget(ValueContrastWidget<T>* widget, uint32_t timeoutUs,
      uint8_t freshContrast, uint8_t staleContrast) :
        widget_(widget), timeoutUs_(timeoutUs), freshContrast_(freshContrast), staleContrast_(staleContrast) {
    widget_->setContrast(staleContrast);
    timer_.start();
  }

  Size layout() {
    // layout is also used to do the staleness updates
    if ((unsigned int)timer_.read_us() >= timeoutUs_) {
      widget_->setContrast(staleContrast_);
    }
    return widget_->layout();
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    widget_->draw(gfx, x, y);
  }

  void setValue(T value) {
    fresh();
    widget_->setValue(value);
  }

  void setValueStale(T value) {
    widget_->setValue(value);
  }

  void fresh() {
    timer_.reset();
    widget_->setContrast(freshContrast_);
  }

protected:
  ValueContrastWidget<T>* widget_;
  const uint32_t timeoutUs_;
  uint8_t freshContrast_, staleContrast_;

  Timer timer_;
};


/**
 * Convenience container combinations
 */
class StaleTextWidget: public Widget {
public:
  StaleTextWidget(const char* value, size_t maxLength = 0,
      uint32_t staleUs = 1000 * 1000,
      GeneratorFont& font = Font5x7,
      uint8_t freshContrast = 255, uint8_t staleContrast = 63) :
      textWidget_(value, maxLength, font, staleContrast),
      staleContainer_(&textWidget_, staleUs, freshContrast, staleContrast) {
  }

  void fresh() {
    staleContainer_.fresh();
  }

  void setValue(const char* value) {
    staleContainer_.setValue(value);
  }

  void setValueStale(const char* value) {
    staleContainer_.setValueStale(value);
  }

  Size layout() {
    return staleContainer_.layout();
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    staleContainer_.draw(gfx, x, y);
  }

protected:
  TextWidget textWidget_;
  StaleContainerWidget<const char*> staleContainer_;
};


class StaleNumericTextWidget: public Widget {
public:
  StaleNumericTextWidget(int32_t value, uint8_t maxDigits = 0,
      uint32_t staleUs = 1000 * 1000,
      GeneratorFont& font = Font5x7,
      uint8_t freshContrast = 255, uint8_t staleContrast = 63,
      GeneratorFont& fractionalFont = Font3x5,
      int32_t fractional = 1, uint8_t fractionalDigits = 0) :
      textWidget_(value, maxDigits, font, staleContrast, fractionalFont, fractional, fractionalDigits),
      staleContainer_(&textWidget_, staleUs, freshContrast, staleContrast) {
  }

  void fresh() {
    staleContainer_.fresh();
  }

  void setValue(int32_t value) {
    staleContainer_.setValue(value);
  }

  void setValueStale(int32_t value) {
    staleContainer_.setValueStale(value);
  }

  Size layout() {
    return staleContainer_.layout();
  }

  void draw(GraphicsApi& gfx, uint16_t x, uint16_t y) {
    staleContainer_.draw(gfx, x, y);
  }

protected:
  NumericTextWidget textWidget_;
  StaleContainerWidget<int32_t> staleContainer_;
};
