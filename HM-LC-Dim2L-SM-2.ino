/*
 * DONE
 * - Test pir sensor
 * - Fix dimmer handling on FF FF ramp time -_> seems to relate to cmDimmer
 * 
 * TODO
 * 
 * dim display
 * Check function for button 
 * Reduce power consumption on sleep periods
 * change from burst to on activitiy??
 * Channel 1 as signaling code
 *  dimmer sends ->
 *    request for status 1 to 4
 *    initiate action 1 to 4
 *  dimmer receives
 *    status set 1 to 4
 *    action 1 to 4 ok/failed
 * 
 * 
 * 
 * 
 */

#define MAX_BRIGHTNESS 128

#define SER_DBG                                        // serial debug messages

//- load library's --------------------------------------------------------------------------------------------------------
#include <AS.h>                                       // ask sin framework
#include "register.h"                                   // configuration sheet


//- Neopixel --------------------------------------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h> 
 
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(2, 5, NEO_GRB + NEO_KHZ800);  
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(2, 14, NEO_GRB + NEO_KHZ800);  
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(2, 15, NEO_GRB + NEO_KHZ800);  


void ledstripeStatus( uint8_t type, uint8_t stat ) {
  if ( type == 0 ) {
    strip1.setPixelColor(3, 0, 0, 255);
    strip1.setPixelColor(4, 0, 0, 255);
  } else if ( type == 1 ) {
    strip1.setPixelColor(3, ((stat!=0)?50:0), 0, 0);
    strip1.setPixelColor(4, 0, 0, 0);
  } else {
    strip1.setPixelColor(3, 0, 0, 0);
    strip1.setPixelColor(4, 0, ((stat!=0)?50:0), 0);
  }
  strip1.show();
}




void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip1.numPixels(); i++) {
      strip1.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip1.show();
    delay(wait);
  }
} 
 
void resetAllStrips() {

    for(int i=0; i<8; i++) {
      strip1.setPixelColor(i, 0,0,0);
      strip2.setPixelColor(i, 0,0,0);
      strip3.setPixelColor(i, 0,0,0);
    }
    strip1.show();
    strip2.show();
    strip3.show();
}


void showAStrip(Adafruit_NeoPixel *strip) {

    for(int i=0; i<8; i++) {
      strip->setPixelColor(i, 255,0,0);
    }
    strip->show();

    _delay_ms (1000);                                   // ...and some information
    
    for(int i=0; i<8; i++) {
      strip->setPixelColor(i, 0,255,0);
    }
    strip->show();

    _delay_ms (1000);                                   // ...and some information

    for(int i=0; i<8; i++) {
      strip->setPixelColor(i, 0,0,255);
    }
    strip->show();

    _delay_ms (1000);                                   // ...and some information
}

void setColor(uint8_t chann, uint8_t r, uint8_t g, uint8_t b) {
  uint16_t i;

  uint16_t s, e;

  if ( ( chann % 2 ) == 0 ) {
    s = 0;
    e = 3;
  } else {
    s = 5;
    e = 8;
  }
  
  
  if ( chann <= 2 ) {
    for(i=s; i<e; i++) {
      strip1.setPixelColor(i, r, g, b);
    }
    strip1.show();
  } else if ( chann <= 4 ) {
    for(i=s; i<e; i++) {
      strip2.setPixelColor(i, r, g, b);
    }
    strip2.show();
  } else {
    for(i=s; i<e; i++) {
      strip3.setPixelColor(i, r, g, b);
    }
    strip2.show();
  }



} 
 
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
  //-------------------------------------------------------------------------------------------------------------------


//- arduino functions -----------------------------------------------------------------------------------------------------
void setup() {

  // - Hardware setup ---------------------------------------
  // - everything off ---------------------------------------

  EIMSK = 0;                                        // disable external interrupts
  ADCSRA = 0;                                       // ADC off
  power_all_disable();                                  // and everything else
  
  DDRB = DDRC = DDRD = 0x00;                                // everything as input
  PORTB = PORTC = PORTD = 0x00;                             // pullup's off

  // todo: timer0 and SPI should enable internally
  power_timer0_enable();
  power_spi_enable();                                   // enable only needed functions

  // enable only what is really needed

  #ifdef SER_DBG                                      // some debug
    dbgStart();                                     // serial setup
    dbg << F("HM-LC-Dim2L-SM-2\n"); 
    dbg << F(LIB_VERSION_STRING);
    _delay_ms (50);                                   // ...and some information
  #endif

  
  // - AskSin related ---------------------------------------
  hm.init();                                        // init the asksin framework
  sei();                                          // enable interrupts


  // - user related -----------------------------------------
  strip1.begin();
  strip1.show(); // Initialize all pixels to 'off' 
  strip1.setBrightness(MAX_BRIGHTNESS);
  strip2.begin();
  strip2.show(); // Initialize all pixels to 'off' 
  strip2.setBrightness(MAX_BRIGHTNESS);

  _delay_ms (1000);
  showAStrip( &strip1 );
  _delay_ms (1000);
  showAStrip( &strip2 );
  _delay_ms (1000);
  showAStrip( &strip3 );
  _delay_ms (5000);
  
  
  pinInput(DDRD,7); // init PIR pin

  setPinHigh(PORTD,7); // init PIR pin

  #ifdef SER_DBG
    dbg << F("HMID: ") << _HEX(HMID,3) << F(", MAID: ") << _HEX(MAID,3) << F("\n\n"); // some debug
  #endif
}

unsigned long lasttime = 0;
void loop() {
  // - AskSin related ---------------------------------------
  hm.poll();                                        // poll the homematic main loop
  
  // - user related -----------------------------------------
/*
  if (( getMillis() - lasttime ) > 500 ) {

    int p = getPin(PIND,7);
    
    #ifdef SER_DBG
      dbg << F("PIR status ") << p << "\n";
    #endif
    lasttime = getMillis();
  }
*/
}


//- user functions --------------------------------------------------------------------------------------------------------
void initIt(uint8_t channel) {
// setting the relay pin as output, could be done also by pinMode(3, OUTPUT)
  #ifdef SER_DBG
    dbg << F("initIt: ") << channel << "\n";
  #endif
  
  pinOutput(DDRD,3);                                    // init the relay pins
  setPinLow(PORTD,3);                                   // set relay pin to ground
  setColor( channel, 0, 0, 255 );

}

/*
void switchIt(uint8_t channel, uint8_t status, uint8_t characteristic) {
  #ifdef SER_DBG
    dbg << F("switchIt: ") << channel << ", " << status << ", " << characteristic << "\n";
  #endif

  if ( status ) 
    setColor( channel, 0, 32, 0 );
  else
    setColor( channel, 32, 0, 0 );


  if (status) setPinHigh(PORTD,3);                            // check status and set relay pin accordingly
  else setPinLow(PORTD,3);
}
*/

int mode_ch1 = 0;

void switchIt2(uint8_t channel, uint8_t status) {
// switching the relay, could be done also by digitalWrite(3,HIGH or LOW)
  #ifdef SER_DBG
    dbg << F("switchIt: ") << channel << ", " << status << "\n";
  #endif

  if ( channel == 1 ) {
    mode_ch1++;
    if ( mode_ch1 > 2 ) mode_ch1 = 0;

    
    if ( mode_ch1 == 0 ) {
      strip1.setBrightness(MAX_BRIGHTNESS);
      strip2.setBrightness(MAX_BRIGHTNESS);
      strip3.setBrightness(MAX_BRIGHTNESS);
      setColor( 0, 0, 0, 255 );
      setColor( 1, 0, 0, 255 );
      setColor( 2, 0, 0, 255 );
      setColor( 3, 0, 0, 255 );
    } else if ( mode_ch1 == 1 ) {
      strip1.setBrightness(MAX_BRIGHTNESS/10);
      strip2.setBrightness(MAX_BRIGHTNESS/10);
      strip3.setBrightness(MAX_BRIGHTNESS/10);
      strip1.show();
      strip2.show();
      strip3.show();
      dbg << F("strip2 dark ") << "\n";
    } else {
      strip1.setBrightness(0);
      strip2.setBrightness(0);
      strip3.setBrightness(0);
      resetAllStrips();
    }
  }

  if ( status ) 
    setColor( channel, 0, 32, 0 );
  else
    setColor( channel, 32, 0, 0 );


  if (status) setPinHigh(PORTD,3);                            // check status and set relay pin accordingly
  else setPinLow(PORTD,3);
}

//- predefined functions --------------------------------------------------------------------------------------------------
void serialEvent() {
  #ifdef SER_DBG
  
  static uint8_t i = 0;                                 // it is a high byte next time
  while (Serial.available()) {
    uint8_t inChar = (uint8_t)Serial.read();                      // read a byte
    if (inChar == '\n') {                               // send to receive routine
      i = 0;
      hm.sn.active = 1;
    }
    
    if      ((inChar>96) && (inChar<103)) inChar-=87;                 // a - f
    else if ((inChar>64) && (inChar<71))  inChar-=55;                 // A - F
    else if ((inChar>47) && (inChar<58))  inChar-=48;                 // 0 - 9
    else continue;
    
    if (i % 2 == 0) hm.sn.buf[i/2] = inChar << 4;                   // high byte
    else hm.sn.buf[i/2] |= inChar;                            // low byte
    
    i++;
  }
  #endif
}


