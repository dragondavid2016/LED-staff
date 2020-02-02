// For Individual White Tube 6/27/2015, D. Kent
// Arduino Nano, data pin for APA104 (or WS2812B) LED strip, Microphone analog input A7, Microphone digital input D13
// supports:
//    Microphone analog input on A7
//    Microphone comparator input on D2
//    Potentiometer on A0
//    Switch on D12
//    Uses D13 (on board LED) for debugging
//    144 LED WS2812B strip data on D7
//
// revision history
//  rev 1.4 - added mode 6 to test colored markers
//  rev 1.3 - added more gain on microphone, removed averaging on mode 2, increased max brightness to 255
//  was 96

// Use if you want to force the software SPI subsystem to be used for some reason (generally, you don't)
// #define FASTLED_FORCE_SOFTWARE_SPI
// Use if you want to force non-accelerated pin access (hint: you really don't, it breaks lots of things)
// #define FASTLED_FORCE_SOFTWARE_SPI
// #define FASTLED_FORCE_SOFTWARE_PINS
#include "FastLED.h"

///////////////////////////////////////////////////////////////////////////////////////////
//
// Move a white dot along the strip of leds.  This program simply shows how to configure the leds,
// and then how to turn a single pixel white and then off, moving down the line of pixels.
// 

// How many leds are in the strip?
#define NUM_LEDS 144

// Data pin that led data will be written out over
#define DATA_PIN 7

// Clock pin only needed for SPI based chipsets when not using hardware SPI
//#define CLOCK_PIN 8

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// start microphone definitions
const unsigned int microphoneInPin = 7;
#define AUDIO_COMPARATOR_PIN 2
#define POT_PIN 0
int switchPin = 12;  // this is the pushbutton switch

unsigned int microphoneValue = 0;
unsigned int mic_min = 1023;
unsigned int mic_max = 0;
unsigned int mic_amplitude = 0;
unsigned int mic_amplitude1 = 0;
unsigned int mic_amplitude_max = 1023;
unsigned int mic_threshold = 0;

// change this to change microphone amplitude measurement sample time
unsigned int mic_loop_count = 100, mic_num_times_looped = 0;  
unsigned int currentPosition = 0, peakPosition = 0;
// end microphone definitions

//-----------------------------------------------------------------

// pixel values in loop()
byte r1, g1, b1;

// ----------------------------------------------------------------
// start common definitions
unsigned int i, j=0;
unsigned long time_ms = 0;
int firstTime = 1;
int mode = 1;
byte audio;

// FastLED demo reel stuff
#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120



// This function sets up the ledsand tells the controller about them
void setup() {
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(3000);

      // Uncomment one of the following lines for your leds arrangement.
      // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
      FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); //use for APA104
      // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
      // FastLED.addLeds<WS2811_400, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);

      // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<SM16716, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<LPD8806, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<P9813, RGB>(leds, NUM_LEDS);
      
      // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
      // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
      
      // set master brightness control
      FastLED.setBrightness(BRIGHTNESS);

      //microphone stuff
      pinMode(AUDIO_COMPARATOR_PIN, INPUT);
      pinMode(13, OUTPUT);
      
      // setup the pushbutton switch input pin
      pinMode(switchPin, INPUT_PULLUP);
      mode = 1;
}  // end setup


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns


// This function runs over and over, and is where you do the magic to light
// your leds.

void loop() {

    // read mode pushbutton switch
  if ( digitalRead(switchPin) == 0 )
  {
    if (mode < 6) {
      mode = mode + 1;
      for (int n=1; n<=mode; n++) {
        fill_solid( &(leds[i]), NUM_LEDS /*number of leds*/, CRGB( 0, 64, 0) );
        FastLED.show();
        digitalWrite(13, HIGH);
        delay(200);
        fill_solid( &(leds[i]), NUM_LEDS /*number of leds*/, CRGB( 0, 0, 0) );
        FastLED.show();
        digitalWrite(13, LOW);
        delay(200);
      }
    } else {
      mode = 1;
      for (int n=1; n<=mode; n++) {
        fill_solid( &(leds[i]), NUM_LEDS /*number of leds*/, CRGB( 0, 64, 0) );
        FastLED.show();
        digitalWrite(13, HIGH);
        delay(200);
        fill_solid( &(leds[i]), NUM_LEDS /*number of leds*/, CRGB( 0, 0, 0) );
        FastLED.show();
        digitalWrite(13, LOW);
        delay(200);
      }
    }

    delay (1000);
    
    // figure out the mode change algorithm
    //programSelSetup(mode);
  }

  
    // microphone input, no equalizer
  if (mode == 1)
    {
    // read the microphone value.  This will be an AC coupled waveform
    //   centered about 1/2 Vref.  Want to sample as fast as possible
    //   and determine the spectral content so we can respond 
    //   differently to different frequency bands (like an equalizer).

    // for first test, lets sample for 10 msec and store min & max values
  
    // 675 = 3.3V which I used for pot reference voltage
    //mic_amplitude_max = map(analogRead(POT_PIN), 0, 675, 0, 1023);
    mic_amplitude_max = analogRead(POT_PIN);
    
    // reset min & max values for sample period
    mic_min = 1023;
    mic_max = 0;
  
    // sample loop
    for (i=1; i<mic_loop_count; i++)
    {
      microphoneValue = analogRead(microphoneInPin);
      if (microphoneValue > mic_max) mic_max = microphoneValue;
      if (microphoneValue < mic_min) mic_min = microphoneValue;
    }

    // calculate the microphone signal amplitude and map to 0-numPixels for
    //   display.  Calculated max range based on max microphone board output 
    //   voltage (3.3V).  5.0V/1024 = 0.00488V, 3.3V/0.00488V = 676 counts
    mic_amplitude = map(mic_max - mic_min, 0, mic_amplitude_max, 0, NUM_LEDS);
    if (currentPosition < mic_amplitude) currentPosition++;
    if (currentPosition > mic_amplitude) currentPosition--;
    if (peakPosition < currentPosition) peakPosition = currentPosition;

    // add time constant to the peak position
    mic_num_times_looped++;
    if ( (mic_num_times_looped > 25) && (peakPosition > currentPosition) ) 
    {
      peakPosition--;
      mic_num_times_looped = 0;
    }

    // create image for display on next counter interrupt
    // this image will move a red pixel to a position corresponding
    //   to the microphone amplitude
    for (i=1; i<=NUM_LEDS; i++) {
      // this one moves a single pixel to mic amplitude position
      //  if (i == currentPosition) r1 = gamma(64); else r1 = gamma(0);

      // this one moves a solid bar from zero to mic amplitude
      //if (i <= currentPosition) r1 = gamma(64); else r1 = gamma(0);
      if (i <= currentPosition) r1 = 64; else r1 = 0;
      // save the peak position so we can put a different color there

      g1 = 0;
      //if (i == peakPosition) b1 = gamma(64); else b1 = gamma (0);
      if (i == peakPosition) b1 = 64; else b1 = 0;
      //strip.setPixelColor(i, r1, g1, b1);
      leds[i].setRGB(r1, g1, b1); // for FastLEDs library
    }
    FastLED.show();
  } // end mode = 1
  
  

  //comparator read
  if (mode == 2) {
    
    audio = digitalRead(AUDIO_COMPARATOR_PIN);
  //leds[NUM_LEDS-2].setRGB(255,255,255);
    if ( audio == 0) {
      fill_solid( &(leds[i]), NUM_LEDS /*number of leds*/, CRGB( 0, 0, 25) );
      digitalWrite(13, LOW);
    } else {
      fill_solid( &(leds[i]), NUM_LEDS /*number of leds*/, CRGB( 255, 0, 0) );
      digitalWrite(13, HIGH);
    }
    FastLED.show();
  } // end mode = 2

// FastLED demo reel
  if (mode == 3) {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();  
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND); 

    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS( 20 ) { nextPattern(); } // change patterns periodically
  } // end mode = 3

// FastLED rainbow solid color rainbow
  if (mode == 4) {

    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS( 20 ) { nextPattern(); } // change patterns periodically

    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue, 0);
    FastLED.show();  

  } // end mode = 4

// FastLED rainbow two colors at a time
  if (mode == 5) {

    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS( 20 ) { nextPattern(); } // change patterns periodically

    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue, 1);
    FastLED.show();  

  } // end mode = 5

// FastLED Red - green - blue slow fade
  if (mode == 6) {

    fill_solid( &(leds[0]), NUM_LEDS, CRGB( 0, 255, 0) );
    FastLED.show();
    delay(180000);
    fill_solid( &(leds[0]), NUM_LEDS, CRGB( 255, 0, 0) );
    FastLED.show();
    delay(180000);
    fill_solid( &(leds[0]), NUM_LEDS /*number of leds*/, CRGB( 0, 0, 255) );
    FastLED.show();
    delay(180000);

  } // end mode = 6


  
/*
  // Move a single white led 
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Set all LEDs to red, brightness based on ultrasonic distance
      leds[whiteLed] = CHSV( h, s, v );



      // Wait a little bit
      //delay(5);

      // Turn our current led back to black for the next loop around
      //leds[whiteLed] = CRGB::Black;
   }
   
   // Show the leds
   FastLED.show();
   
   
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Green;

      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(5);

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Blue;

      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(5);

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
      // Turn our current led on to white, then show the leds
      leds[whiteLed] = CRGB::Goldenrod;

      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(5);

      // Turn our current led back to black for the next loop around
      leds[whiteLed] = CRGB::Black;
   }
   
   */
}


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 2);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

//------------------------------------------------------------------------------------------
void programSelSetup(int mode)
{
  // rolling marble mode
  if (mode == 1)
  {
    
  } // end mode 1 setup

  // microphone, no equalizer mode
  if (mode == 2)
  {
  } // end mode 2 setup

  // Advanced LED belt demo mode
  if (mode == 3)
  {
    
  } // end mode 3 setup

  // all blue intensity modulated by mic volume mode
  if (mode == 4)
  {
    mic_loop_count = 100;
    mic_amplitude_max = 1023;
    mic_threshold = 0;
  } // end mode 4 setup
}
