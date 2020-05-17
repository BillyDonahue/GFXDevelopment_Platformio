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

// An SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(128, 32, &Wire, 4);

void classicFontCheckers() {
  display.clearDisplay();
  display.setFont(nullptr);
  display.cp437();
  static const uint8_t kGlyphWidth = 6;
  static const uint8_t kGlyphHeight = 8;
  static const uint16_t kPerRow = 128 / kGlyphWidth;
  static const uint16_t kRows = 32 / kGlyphHeight;
  for (uint16_t i = 0; i < 256; ++i) {
    uint16_t x = 0 + kGlyphWidth * (i % kPerRow);
    uint16_t y = 0 + kGlyphHeight * ((i / kPerRow) % kRows);
    if (x == 0 && y == 0) {
      display.display();
      delay(1000);
    }
    uint16_t fg = SSD1306_WHITE;
    uint16_t bg = SSD1306_BLACK;
    if (i & 1) {
      uint16_t t = fg;
      fg = bg;
      bg = t;
    }
    display.drawChar(x, y, (i & 0xff), fg, bg, 1);
    // display.printf("%d,", i);
    Serial.printf("%d\n", i);
  }
}

void textDemo() {
  const GFXfont *fonts[] = {
      // nullptr, // classic
      //&FreeMonoBold12pt7b,    //
      //&FreeSerifItalic12pt7b, //
      //&FreeMonoOblique9pt7b, //
      //&Org_01,
  };
  for (const GFXfont *font : fonts) {
    // Set a font
    display.clearDisplay();
    display.setFont(font);
    display.cp437();
    uint16_t x = 0;

    uint16_t yAdv = font ? font->yAdvance : 8;
    uint16_t first = font ? font->first : 0;
    uint16_t last = font ? font->last : 255;

    uint16_t y = yAdv;

    for (uint16_t ch = first; ch <= last; ++ch) {
      char s[2] = {(unsigned char)ch, 0};
      int16_t left = 0;
      int16_t top = 0;
      uint16_t w = 0;
      uint16_t h = 0;
      display.getTextBounds(s, 0, y, &left, &top, &w, &h);
      Serial.printf("TextBounds(\"%s\",%u,%u)=(%d,%d,%u,%u)\n", //
                    s, x, y, left, top, w, h);
      if (x + left + w > display.width()) {
        display.display();
        delay(1000);
        x = 0;
        Serial.print("clearing display\n");
        display.clearDisplay();
        // display.display();
      }
      if (top < 0) {
        Serial.printf("adjust y=%u by %d !!!\n", y, -top);
        delay(1000);
        y += -top;
      }
      display.drawChar(x, y, ch, SSD1306_WHITE, SSD1306_BLACK, 1);
      x += left + w;
      // delay(20);
    }
  }
}

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  delay(1000);

  classicFontCheckers();
}

void loop() { textDemo(); }