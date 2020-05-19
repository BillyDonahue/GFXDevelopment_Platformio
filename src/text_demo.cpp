#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/Org_01.h>
#include <SPI.h>
#include <Wire.h>

// Test the effect of leaving setFont calls out of a sketch
// in Adafruit-GFX 1.8.2, it doesn't save much.

// AVR (Uno) flash space:
//  withFonts     , false , true
// 1.8.2          , 13944 , 19376
// glcdfont_unify , 13526 , 19610

static const bool withFonts = 0;

#ifdef ADAFRUIT_GFX_GFXUNIFY
//#error "unify branch building"
#endif

namespace {
template <size_t N> struct SizeTag {};
template <typename T> T pgmRead(const T *p, SizeTag<1>) {
  return (T)pgm_read_byte(p);
}
template <typename T> T pgmRead(const T *p, SizeTag<2>) {
  return (T)pgm_read_word(p);
}
template <typename T> T pgmRead(const T *p, SizeTag<4>) {
  return (T)pgm_read_dword(p);
}
template <typename T> T pgmRead(const T *p) {
  return pgmRead(p, SizeTag<sizeof(T)>());
}

template <typename T> T minFunc(T a, T b) { return a < b ? a : b; }

} // namespace

// Abstraction to contain the pgm_read discipline.
struct FontHandler {
  struct Glyph {
    uint8_t width() const { return pgmRead(&g->width); }
    uint8_t height() const { return pgmRead(&g->height); }
    uint8_t xAdvance() const { return pgmRead(&g->xAdvance); }
    int8_t xOffset() const { return pgmRead(&g->xOffset); }
    int8_t yOffset() const { return pgmRead(&g->yOffset); }
    const GFXglyph *g;
  };

  explicit FontHandler(const GFXfont *font) : font(font) {}

  uint16_t first() const { return font ? pgmRead(&font->first) : 0; }
  uint16_t last() const { return font ? pgmRead(&font->last) : 255; }
  uint8_t yAdvance() const { return font ? pgmRead(&font->yAdvance) : 8; }
  int8_t yAdjust() const { return font ? 0 : 6; }

  GFXglyph *getGlyph(uint16_t c) const {
    if (c < first() || c > last())
      return nullptr;
    return pgmRead(&font->glyph) + (c - first());
  }

  uint8_t xAdvance(uint16_t c) const {
    if (!font)
      return 6;
    Glyph g = {getGlyph(c)};
    if (!g.g)
      return 0;
    return g.xAdvance();
  }

  const GFXfont *font;
};

struct {
  const char *name;
  const GFXfont *font;
  int scale;
} fontSpecs[] = {
    //{"Classic", nullptr, 1}, //
    //{"Classic", nullptr, 2}, //
    //{"Classic", nullptr, 3},
    //{"Classic", nullptr, 4},
    //{"Org_01", &Org_01, 1},
    //{"FreeMonoBold12pt7b", &FreeMonoBold12pt7b, 1},
    //{"FreeMonoBold12pt7b", &FreeMonoBold12pt7b, 2},
    //{"FreeSerifItalic12pt7b", &FreeSerifItalic12pt7b, 1},
    //{"FreeSerifItalic12pt7b", &FreeSerifItalic12pt7b, 2},
    //{"FreeMonoOblique9pt7b", &FreeMonoOblique9pt7b, 1},
    //{"FreeMonoOblique9pt7b", &FreeMonoOblique9pt7b, 2},
};

Adafruit_SSD1306 oled(128, 64, &Wire, 4);

void flushDisplay(Adafruit_GFX &display) {
  if (&display == &oled) {
    oled.display();
  }
}

void clearDisplay(Adafruit_GFX &display) {
  if (&display == &oled) {
    oled.clearDisplay();
  } else {
    display.fillScreen(0);
  }
}

void fontShow(Adafruit_GFX &display, const FontHandler &font, uint8_t scale,
              uint16_t pageMillis) {
  clearDisplay(display);
  display.setFont(font.font);
  display.setTextSize(scale);

  int16_t x = 0;

  int row = 0;

  for (uint16_t i = 0;;) {
    char s[2] = {(char)i, 0};
    int16_t left = 0, top = 0;
    uint16_t w = 0, h = 0;

    display.getTextBounds(s, 0, 0, &left, &top, &w, &h);
    if (1) {
      Serial.print("i=");
      Serial.print(i);
      Serial.print(", plot(");
      Serial.print((unsigned char)s[0], HEX);
      // Serial.print(",");
      // Serial.print(x);
      // Serial.print(",");
      // Serial.print(y);
      Serial.print(")=");
      Serial.print("{left:");
      Serial.print(left);
      Serial.print(",top:");
      Serial.print(top);
      Serial.print(",w:");
      Serial.print(w);
      Serial.print(",h:");
      Serial.print(h);
      Serial.println("}");
    }

    if (0) {
      flushDisplay(display);
      Serial.print("[halt]");
      for (;;) {
      }
    }

    if (w && (x + left + int16_t(w) > display.width())) {
      Serial.print("[hwrap]");
      if (x == 0) { // won't fit
        ++i;
      } else {
        ++row;
        x = 0;
      }
      continue;
    }

    // y: baseline of `row` , adjusted by the yAdjust offset.
    int16_t y = scale * (font.yAdvance() * row + font.yAdvance() * 6 / 8 -
                         font.yAdjust());

    if ((i && y + top + int16_t(h) > display.height()) || i == 256) {
      Serial.print(", flush");
      flushDisplay(display);
      delay(pageMillis);
      clearDisplay(display);
      if (i == 256) {
        Serial.println(", bye");
        return;
      } else {
        Serial.print("[vwrap]");
        if (x == 0 && row == 0) // won't fit
          ++i;
        x = 0;
        row = 0;
        continue;
      }
    }

    if (w && h) {
      display.drawChar(x, y, i, 1, 0, scale);
    }
    x += scale * font.xAdvance(i);
    ++i;
  }
}

void textDemo(Adafruit_GFX &display) {
  for (const auto &spec : fontSpecs) {

    display.setTextColor(1, 0);
    display.setCursor(0, 20);

    display.setFont(nullptr);
    display.cp437();
    display.setTextSize(1);
    display.print("A");

    display.setFont(spec.font);
    display.cp437();
    display.setTextSize(spec.scale);
    display.print(spec.name);
    flushDisplay(display);

    delay(1000);

    if (0) {
      uint16_t x = 0;
      uint16_t yAdv = spec.font ? spec.font->yAdvance : 8;
      uint16_t first = spec.font ? spec.font->first : 0;
      uint16_t last = spec.font ? spec.font->last : 255;
      uint16_t y = yAdv * spec.scale;

      for (uint16_t ch = first; ch <= last; ++ch) {
        char s[2] = {(char)ch, 0};
        int16_t left = 0;
        int16_t top = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        display.getTextBounds(s, 0, y, &left, &top, &w, &h);
        // Serial.printf("TextBounds(\"%s\",%u,%u)=(%d,%d,%u,%u)\n", //
        //              s, x, y, left, top, w, h);
        if ((int16_t)(x + left + w) > (int16_t)display.width()) {
          flushDisplay(display);
          delay(100);
          x = 0;
          // Serial.print("clearing display\n");
          clearDisplay(display);
        }
        if (top < 0) {
          // Serial.printf("adjust y=%u by %d !!!\n", y, -top);
          delay(1000);
          y += -top;
        }
        display.drawChar(x, y, ch, 1, 0, spec.scale);
        x += left + w;
      }
    }
  }
}

void drawAlphabetTo(unsigned long niter, Adafruit_GFX &display,
                    const FontHandler &font, uint8_t scale) {
  int16_t x = display.width() / 2;
  int16_t y = display.height() / 2;
  unsigned char first = 0;
  unsigned char last = 255;
  if (font.font) {
    first = (unsigned char)minFunc(uint16_t(255), font.first());
    last = (unsigned char)minFunc(uint16_t(255), font.last());
  }

  while (niter--) {
    // delay(10);
    // continue;

    for (unsigned char ch = first;; ++ch) {
      if (0) {
        Serial.print("ch:");
        Serial.print((int)ch);
        Serial.print(",");
      }
      display.drawChar(x, y, ch, 1, 0, scale);
      if (ch == last)
        break;
    }
  }
}

void benchFont(Print &out, Adafruit_GFX &display, const FontHandler &font,
               uint8_t scale) {
  // out.println("Running benchmark");
  display.setFont(font.font);
  // display.setTextSize(scale);
  // display.setCursor(0, 20);

  static const unsigned long minDuration = 1000;
  out.print(", trials:[");
  unsigned long iters = 10;
  unsigned long duration = 0;
  for (;;) {
    out.print("{");
    out.print(iters);
    out.print(",");
    unsigned long t0 = millis();
    drawAlphabetTo(iters, display, font, scale);
    unsigned long t1 = millis();

    duration = t1 - t0;
    out.print(duration);
    out.print("},");
    if (duration > minDuration) {
      break;
    }

    // improve the estimate of iters
    {
      if (duration) {
        auto ni = iters * (1.2 * minDuration / duration);
        if (ni > iters) {
          iters = ni;
          continue;
        }
      }
      ++iters;
      iters *= 2;
    }
  }
  out.print("]");
  if (1) {
    out.print("], duration:");
    out.print(duration);
    out.print(", iters:");
    out.print(iters);
  }
  out.print(", msec/iter:");
  out.print(duration / iters);
}

void setup() {
  // Serial.begin(9600);
  Serial.begin(115200);
  if (!oled.begin(SSD1306_SWITCHCAPVCC,
                  0x3d)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  oled.cp437(true);
  oled.setTextColor(1, 0);
  if (1) {
    flushDisplay(oled);
    delay(2000);
    clearDisplay(oled);
  }

  if (1) {
    uint16_t basicPageMillis = 50;
    uint16_t scales[] = {1, 2, 3, 4};
    for (uint16_t sc : scales) {
      FontHandler fh(nullptr);
      fontShow(oled, fh, sc, basicPageMillis);
    }
  }
  if (1) {
    uint16_t basicPageMillis = 200;
    uint16_t scales[] = {1, 2};
    for (uint16_t sc : scales) {
      FontHandler fh(&FreeMonoBold12pt7b);
      fontShow(oled, fh, sc, basicPageMillis / sc);
    }
  }
  if (0) {
    Serial.println("Bench");
    for (const auto &spec : fontSpecs) {
      Serial.print("  font: ");
      Serial.print(spec.name);
      Serial.print(", scale: ");
      Serial.print(spec.scale);
      Serial.print(", display: oled");
      FontHandler fh(spec.font);
      benchFont(Serial, oled, fh, spec.scale);
      Serial.println("");
    }
  }
}

void loop() {
  if (withFonts)
    textDemo(oled);
}