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

static const bool withFonts = 1;

Adafruit_SSD1306 oled(128, 64, &Wire, 4);

void bench(Print &out, void (*f)(unsigned long)) {
  unsigned long minDuration = 2000;

  out.println("Running benchmark");
  for (unsigned long iters = 10;;) {
    out.print("iters=");
    out.println(iters);
    unsigned long t0 = millis();
    f(iters);
    unsigned long t1 = millis();
    unsigned long duration = t1 - t0;
    if (duration > minDuration) {
      if (0) {
        out.print("duration:");
        out.println(duration);
        out.print("iters:");
        out.println(iters);
      }
      out.print("msec/iter:");
      out.println(duration / iters);
      return;
    }
    // improve the estimate of iters
    iters = iters * (1.2 * minDuration / duration);
  }
}

void flushDisplay(Adafruit_GFX &display) {
  if (&display == &oled) {
    oled.display();
  }
}

void clearDisplay(Adafruit_GFX &display) {
  if (&display == &oled) {
    oled.clearDisplay();
  } else {
    display.fillRect(0, 0, display.width(), display.height(), SSD1306_BLACK);
  }
}

void classicFontCheckerBoard(Adafruit_GFX &display, uint8_t scale,
                             uint16_t pageMillis) {
  clearDisplay(display);
  static const uint8_t kGlyphWidth = 6;
  static const uint8_t kGlyphHeight = 8;
  const uint8_t kScaledGlyphWidth = kGlyphWidth * scale;
  const uint8_t kScaledGlyphHeight = kGlyphHeight * scale;
  uint16_t kPerRow = display.width() / kScaledGlyphWidth;
  uint16_t kRows = display.height() / kScaledGlyphHeight;
  for (uint16_t i = 0;; ++i) {
    uint16_t x = 0 + kScaledGlyphWidth * (i % kPerRow);
    uint16_t y = 0 + kScaledGlyphHeight * ((i / kPerRow) % kRows);
    if ((i && x == 0 && y == 0) || i == 256) {
      flushDisplay(display);
      delay(pageMillis);
      clearDisplay(display);
      if (i == 256)
        return;
    }
    uint16_t fg = SSD1306_WHITE;
    uint16_t bg = SSD1306_BLACK;
    if (false && i & 1) {
      uint16_t t = fg;
      fg = bg;
      bg = t;
    }
    display.drawChar(x, y, (i & 0xff), fg, bg, scale);
  }
}

void textDemo(Adafruit_GFX &display) {
  struct {
    const char *name;
    const GFXfont *font;
    int scale;
  } fontSpecs[] = {
      {"Classic", nullptr, 1},
      {"Classic", nullptr, 2},
      //{"Classic", nullptr, 3},
      //{"Classic", nullptr, 4},
      {"Org_01", &Org_01, 1},
      {"FreeMonoBold12pt7b", &FreeMonoBold12pt7b, 1},
      //{"FreeMonoBold12pt7b", &FreeMonoBold12pt7b, 2},
      //{"FreeSerifItalic12pt7b", &FreeSerifItalic12pt7b, 1},
      //{"FreeSerifItalic12pt7b", &FreeSerifItalic12pt7b, 2},
      {"FreeMonoOblique9pt7b", &FreeMonoOblique9pt7b, 1},
      //{"FreeMonoOblique9pt7b", &FreeMonoOblique9pt7b, 2},
  };
  for (const auto &spec : fontSpecs) {

    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
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
        display.drawChar(x, y, ch, SSD1306_WHITE, SSD1306_BLACK, spec.scale);
        x += left + w;
      }
    }
  }
}

void drawAlphabet(unsigned long niter) {
  GFXcanvas1 canvas(32, 32);
  while (niter--) {
    for (unsigned char c = 0;; ++c) {
      canvas.drawChar(0, 0, c, 1, 0, 1);
      if (c == 0xff)
        break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3d)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  oled.cp437(true);
  flushDisplay(oled);
  delay(2000);
  clearDisplay(oled);

  if (1) {
    uint16_t basicPageMillis = 40;
    uint16_t scales[] = {1, 8};
    for (uint16_t sc : scales) {
      classicFontCheckerBoard(oled, sc, basicPageMillis / sc);
    }
  }
  clearDisplay(oled);
  oled.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  oled.setCursor(0, 0);
  bench(oled, &drawAlphabet);
  flushDisplay(oled);
  delay(2000);
  for (;;)
    ;
}

// comment out the textDemo() call

void loop() {
  if (withFonts)
    textDemo(oled);
}