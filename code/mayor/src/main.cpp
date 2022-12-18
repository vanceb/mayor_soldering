#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LDR_PIN A3
#define TILT_PIN 4
#define PIXEL_PIN 2
#define NUM_PIXELS 13

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Variables to hold the max and min values of the LDR
int  max_light;
int  min_light;
int  light;
// Variable for last state of the tilt switch and time
int tilt;
int  last_tilt;
long tilt_change_millis;
// Current mode
#define MAX_MODES 7
int mode;
// Loop counter
long loop_counter;
// Current Colour
long colour;

int set_brightness(bool first_read = false) {
  light = map(analogRead(LDR_PIN), 0, 1024, 255, 1);

  if (first_read) {
    max_light = light + 10;
    min_light = light - 10;
  } else {
    min_light = light < min_light ? light : min_light;
    max_light = light > max_light ? light : max_light;
  }
  min_light = min_light < 0 ? 0 : min_light;
  max_light = max_light > 255 ? 255 : max_light;
  strip.setBrightness(map(light, min_light, max_light, 1, 255));  
  return light;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void brightness() {
  int step = 255 / NUM_PIXELS;
  int pixel_val = 1;
  strip.clear();
  for (int i=0; i<NUM_PIXELS; i++) {
    pixel_val = step * i;
    if (min_light >= pixel_val) {
      strip.setPixelColor(i, 0x000000FF);
    } else if (light > pixel_val and light < pixel_val + step) {
      strip.setPixelColor(i, 0x0000FF00);
    } else if (max_light <= pixel_val) {
      strip.setPixelColor(i, 0x00FF0000);
    }
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  if ((loop_counter % NUM_PIXELS) == 0) {
    strip.clear();
  }
  strip.setPixelColor(loop_counter % NUM_PIXELS, c);
}

void rainbow() {
  uint16_t i, j;

  j = loop_counter % 256;
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+j) & 255));
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i, j;

  j = loop_counter % 256;
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c) {
  strip.clear();
  for (int i=0; i< NUM_PIXELS; i++) {
    if (((i + loop_counter) % 3) == 0) {
      strip.setPixelColor(i, c);
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow() {
  int j = loop_counter % 256;
  strip.clear();
  for (int i=0; i< NUM_PIXELS; i++) {
    if (((i + loop_counter) % 3) == 0) {
      strip.setPixelColor(i, Wheel((i+j) % 255));
    }
  }
}

void walk(long colour) {
  strip.clear();
  strip.setPixelColor(loop_counter % NUM_PIXELS, colour);
}

void setup() {
  loop_counter = 0;
  mode = 0;
  last_tilt = 0;
  tilt = 0;

  colour = random(0xFFFFFF);

  // Configure the LED Pixels
  strip.begin();
  strip.setPixelColor(0,colour);
  
  // Setup and read the LDR
  pinMode(LDR_PIN, INPUT);
  strip.setBrightness(100);
  //set_brightness(true);
  
  strip.show();
}

void loop() {

  // Do something if we detect a tilt
  tilt = digitalRead(TILT_PIN);

  if (tilt == 0 && tilt != last_tilt) {
    mode = ++mode % MAX_MODES;
  }

  last_tilt = tilt;

  switch (mode)
  {
  case 0:
    brightness();
    break;
  
  case 1:
    walk(colour);
    break;
  
  case 2:
    colorWipe(colour);
    break;

  case 3:
    rainbow();
    break;

  case 4:
    rainbowCycle();
    break;

  case 5:
    theaterChase(colour);
    break;

  case 6:
    theaterChaseRainbow();
    break;

  default:
    break;
  }

  set_brightness();
  strip.show();

  loop_counter++;
  // Change Random Colour
  if ((loop_counter % NUM_PIXELS) == 0) {
    colour = random(0xFFFFFF);
  }
  delay(50);
}