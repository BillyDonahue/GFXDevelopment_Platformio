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
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16
const unsigned char PROGMEM logo_bmp[] = {B00000000, B11000000, //
                                          B00000001, B11000000, //
                                          B00000001, B11000000, //
                                          B00000011, B11100000, //
                                          B11110011, B11100000, //
                                          B11111110, B11111000, //
                                          B01111110, B11111111, //
                                          B00110011, B10011111, //
                                          B00011111, B11111100, //
                                          B00001101, B01110000, //
                                          B00011011, B10100000, //
                                          B00111111, B11100000, //
                                          B00111111, B11110000, //
                                          B01111100, B11110000, //
                                          B01110000, B01110000, //
                                          B00000000, B00110000};

void textDemo() {
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

  if (0) {
    static const uint8_t kGlyphWidth = 6;
    static const uint8_t kGlyphHeight = 8;
    static const uint16_t kPerRow = 128 / kGlyphWidth;
    static const uint16_t kRows = 32 / kGlyphHeight;
    for (uint16_t i = 0; i < 256; ++i) {
      uint16_t x = 0 + kGlyphWidth * (i % kPerRow);
      uint16_t y = 0 + kGlyphHeight * ((i / kPerRow) % kRows);
      display.drawChar(x, y, (i & 0xff),
                       (i & 1) == 0 ? SSD1306_WHITE : SSD1306_BLACK,
                       (i & 1) == 1 ? SSD1306_WHITE : SSD1306_BLACK, 1);
      // display.printf("%d,", i);
      Serial.printf("%d\n", i);
      display.display();
      delay(10);
    }
  }

  for (;;) {
    const GFXfont *fonts[] = {
        nullptr,                // classic
        &FreeMonoBold12pt7b,    //
        &FreeSerifItalic12pt7b, //
        &FreeMonoOblique9pt7b,  //
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
        char s[2] = {(char)ch, 0};
        int16_t left = 0;
        int16_t top = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        display.getTextBounds(s, 0, y, &left, &top, &w, &h);
        Serial.printf("TextBounds(\"%s\",%u,%u)=(%d,%d,%u,%u)\n", //
                      s, x, y, left, top, w, h);
        if (x + left + w > display.width()) {
          display.display();
          delay(200);
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
}

void setup() { textDemo(); }

void loop() {}