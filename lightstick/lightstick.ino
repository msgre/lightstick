/*
 * Lightstick for Museum Night 2016 event in Valasske Mezirici, Czech Republic.
 * 
 * Description:
 * 
 * - lightstick is thin wooden batten with 120 NeoPixel LED's
 * - lighstick is connected to Arduino microcontroler with SD card
 * - on SD card are stored two images in 256 color pallete
 * - there are 2 controls: two position switch for selecting one or 
 *   other image, and play button
 * - after pressing play button simple countdown is displayed 
 *   (5 red LEDs, 4, 3, 2, ...)
 * - lightstick starts with image drawing
 * - operator must walk slowly from left to right side during lightstick 
 *   drawing (from photographer point of view)
 * - photographer must take picture in manual mode with some long exposure 
 *   times (~10 seconds and more according to image size)
 */
 
#include <SPI.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>

// controls
#define BUTTON_PLAY 10
#define SWITCH_A 11
#define SWITCH_B 12
#define CONTROL_POWER 13    // power for button and switch

// neopixels parameters
#define NEOPIXELS_COUNT  120
#define NEOPIXELS_PIN    3
Adafruit_NeoPixel bar = Adafruit_NeoPixel(NEOPIXELS_COUNT, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);

// sd card
#define SD_CARD_CS 4

// general constants
#define DELAY 10
const uint32_t BLACK = bar.Color(0, 0, 0);
const uint32_t RED = bar.Color(32, 0, 0);

// global variables
char filename[20];                // selected file to draw
unsigned int WIDTH, HEIGHT;       // source image dimensions 
unsigned char PALETTE[256 * 3];   // source image 256 color RGB palette


/*
 * Prepare lighstick.
 */
void setup() {
  // control switch & button inputs
  pinMode(BUTTON_PLAY, INPUT);
  pinMode(SWITCH_A, INPUT);
  pinMode(SWITCH_B, INPUT);

  // power for control buttons (switch + play)
  pinMode(CONTROL_POWER, OUTPUT);
  digitalWrite(CONTROL_POWER, HIGH);

  // neopixels init
  bar.begin();

  // sd card initialization
  if (!SD.begin(SD_CARD_CS)) {
    signalise_error();
    return;
  }

  // show that we are ready
  demo();
}

/*
 * Main loop.
 */
void loop() {
  // picture selection according to switch position
  if (digitalRead(SWITCH_A) == HIGH) {
    sprintf(filename, "muzeum.lig");
  }
  else if (digitalRead(SWITCH_B) == HIGH) {
    sprintf(filename, "modrasek.lig");
  }

  // play button 
  if (digitalRead(BUTTON_PLAY) == HIGH) {
    countdown();
    if (SD.exists(filename)) {
      draw_image(filename);
    }
    else {
      signalise_error();
      delay(2000);
    }
    clear_bar();
  }
  
  delay(DELAY);
}

/*
 * Simple effect to signalize problem.
 */
void signalise_error() {
   bar.setPixelColor(0, RED);
   bar.setPixelColor((NEOPIXELS_COUNT / 2) - 1, RED);
   bar.setPixelColor(NEOPIXELS_COUNT - 1, RED);
   bar.show();
}

/*
 * Clear NeoPixel bar (set its colors to black).
 */
void clear_bar() {
  for (int i = 0; i < NEOPIXELS_COUNT; i++) {
    bar.setPixelColor(i, BLACK);
  }
  bar.show();
}

/*
 * Initial demo effect with Neopixels (to be sure that everything just works).
 */
void demo() {
  float x, angle, delta;
  int i, r, g, b;
  
  delta = 0.5;
  
  for (int y = 0; y < 90; y++) {
    for (int i = 0; i < NEOPIXELS_COUNT; i++) {
      angle = y * delta + i * delta;
      x = (angle / (2 * 3.141592654));
      r = round((1 + sin(x)) / 2.0 * 32);
      g = round((1 + sin(x + 1.5707963)) / 2.0 * 32);
      b = round((1 + sin(x + 0.78539815)) / 2.0 * 32);
      bar.setPixelColor(i, bar.Color(r, g, b));
    }
    bar.show();
    delay(DELAY);
  }
  clear_bar();
  delay(800);
}

/*
 * Countdown for easier synchronisation with walking operator and photographer.
 */
void countdown() {
  for (int y = 5; y > 0; y--) {
    if (y > 2) {
      for (int i = 0; i < y; i++) {
        bar.setPixelColor(i, RED);
      }
      bar.show();
    }
    delay(800);

    clear_bar();
    delay(200);
  }
}

/*
 * Read information about image dimension from file and store it in global
 * variables WIDTH/HEIGHT.
 */
void read_size(File file) {
  int idx = 0;
  unsigned char buff[4];
  
  while ((file.available()) && (idx < 4)) {
    buff[idx] = file.read();
    idx++;
  }
  
  WIDTH = (unsigned int)buff[0];
  WIDTH |= buff[1] << 8;

  HEIGHT = (unsigned int)buff[2];
  HEIGHT |= buff[3] << 8;
}

/*
 * Read palette information from file and store it in global array
 * PALETTE.
*/
void read_palette(File file) {
  int idx = 0;
  
  while ((file.available()) && (idx < (3 * 256))) {
    PALETTE[idx] = file.read();
    idx++;
  } 
}

/*
 * Read one vertical line from file and send it to NeoPixels.
 */
int draw_bar(File file) {
  int idx = 0, pidx, height = HEIGHT, remainder;
  unsigned char pixel, r, g, b;
  
  // remainder
  if (height > NEOPIXELS_COUNT) {
    height = NEOPIXELS_COUNT;
    remainder = HEIGHT - NEOPIXELS_COUNT;
  }
  else {
    remainder = -1;
  }
  
  // read one bar from file and set NeoPixels buffer
  while ((file.available()) && (idx < height)) {
    pixel = file.read();
    pidx = pixel * 3;
    r = PALETTE[pidx + 0];
    g = PALETTE[pidx + 1];
    b = PALETTE[pidx + 2];
    bar.setPixelColor(idx, bar.Color(r, g, b));
    idx++;
  } 
  
  // update NeoPixels
  bar.show();  
  delay(DELAY);
  
  // move position in file
  if (remainder > 0) {
    file.seek(file.position() + remainder);
  }
  
  return file.available();
}

/*
 * Draw given filename.
 */
void draw_image(char *filename) {
  File file = SD.open(filename);
  if (file) {
    file = SD.open(filename);
    read_size(file);
    read_palette(file);
    while (draw_bar(file)) {
    }
    file.close();
    clear_bar();
  }  
  else {
    signalise_error();
  } 
}
