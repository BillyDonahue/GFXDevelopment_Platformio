/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

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

// An SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(128, 64, &Wire, 4);

void classicFontCheckerBoard(uint8_t scale, uint16_t pageMillis) {
  display.clearDisplay();
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
      display.display();
      delay(pageMillis);
      display.clearDisplay();
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

void textDemo() {
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
    display.clearDisplay();
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
    display.display();

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
          display.display();
          delay(100);
          x = 0;
          // Serial.print("clearing display\n");
          display.clearDisplay();
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
void bench() {
  unsigned long minDuration = 500;
  unsigned long iters = 0;

  GFXcanvas1 canvas(32, 32);
  Serial.println("benchmark:");
  for (unsigned long runBlock = 100;; runBlock *= 2) {
    unsigned long t0 = millis();
    for (unsigned long n = 0; n < runBlock; ++n) {
      for (unsigned char c = 0;; ++c) {
        canvas.drawChar(0, 0, c, 1, 0, 1);
        if (c == 0xff)
          break;
      }
      ++iters;
    }
    unsigned long t = millis() - t0;
    if (t > minDuration) {
      Serial.print("  {");
      Serial.print("t:");
      Serial.print(t);
      Serial.print(" msec, ");

      Serial.print("iters:");
      Serial.print(iters);

      Serial.print(", period:");
      Serial.print((uint32_t)t / iters);
      Serial.print(" msec/iter");

      Serial.print("}\n");
      return;
    }
  }
}

void setup() {
  display.cp437(true);
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3d)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  if (0) {
    uint16_t basicPageMillis = 100;
    uint16_t scales[] = {1, 2, 4, 8};
    for (uint16_t sc : scales) {
      classicFontCheckerBoard(sc, basicPageMillis / sc);
    }
  }
  if (1) {
    for (int i = 0; i < 5; ++i) {
      bench();
    }
  }
}

// comment out the textDemo() call

void loop() {
  if (withFonts) {
    textDemo();
  }
  // for (;;)
}