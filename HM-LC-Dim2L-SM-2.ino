/*
 * DONE
 * - Test pir sensor
 * - Fix dimmer handling on FF FF ramp time -_> seems to relate to cmDimmer
 * - Created new cmStatusBoard for Status board
 * - reduced code for cmStatusBoard
 * - Ensure switch function called always when trigger11 is received (although value unchanged)
 * - 
 * TODO
 * 
 * store latest status in progmem
 * dim display on channel 1
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
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(8, 5, NEO_GRB + NEO_KHZ800);  
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(16, 4, NEO_GRB + NEO_KHZ800);  
//- Neopixel --------------------------------------------------------------------------------------------------------

#define MAX_BRIGHTNESS 128
#define FULL_RED 50
#define FULL_GREEN 70
#define FULL_BLUE 70

//--------------------

void myShow( uint8_t strip ) {
  if ( strip == 1 ) {
    strip1.show();
  } else {
    strip2.show();
  }
}

void mysetPixelColor( uint8_t strip, uint8_t led, uint8_t r, uint8_t g, uint8_t b ) {
  if ( strip == 3 ) {
    strip2.setPixelColor(led+8, r,g,b);
  } else if ( strip == 2 ) {
    strip2.setPixelColor(led, r,g,b);
  } else {
    strip1.setPixelColor(led, r,g,b);
  }
}
  


//- Neopixel -- Functions ------------------------------------------------------------------------------------------------------
/*
 * Concept
 *  Oben Strip 2 erste 8  = intChannel 1
 *  mitte strip 2 zweite 8 = intChannel 2
 *  unten Strip 1 - geteilt in 2 = intChannel 3/4
 *  
 *  Status set values in SW_01
 *  
 *  LED status are for 4 channels 
 *    3 = HM_606060_Sw1_V_01 = intChannel 1
 *    4 = HM_606060_Sw1_V_02 = intChannel 2
 *    5 = HM_606060_Sw2_V_01 = intChannel 3
 *    6 = HM_606060_Sw2_V_02 = intChannel 4
 *    
 *    intChannel 0 is for internal purposes
 *    
 *  Communication is coming through: HM_606060_Sw_01 and HM_606060_Sw_02
 *    HM_606060_Sw_01 - setting pct between 0 and 100 => 0 and 200 in HM
 *    
 *    ( intChannel * 20 ) + intValue
 *    
 *    intValue = 0 - off
 *    intValue = 1 - green 
 *    intValue = 2 - green / 2
 *    intValue = 5 - red 
 *    intValue = 6 - red / 2
 *    intValue = 11 - blue 
 *    intValue = 12 - blue / 2
 *    intValue = 15 - white
 *    intValue = 16 - white / 2
 *    
 *  Buttons are signaled as on/off on intChannel 1 to 3
 */

// Show Status of connection
void ledStripeStatusHM( uint8_t type, uint8_t stat ) {
  if ( type == 0 ) {
    strip1.setPixelColor(3, 0, 0, 50);
    strip1.setPixelColor(4, 0, 0, 50);
  } else if ( type == 1 ) {
    strip1.setPixelColor(3, ((stat!=0)?50:0), 0, 0);
    strip1.setPixelColor(4, 0, 0, 0);
  } else {
    strip1.setPixelColor(3, 0, 0, 0);
    strip1.setPixelColor(4, 0, ((stat!=0)?50:0), 0);
  }
  strip1.show();
}


void showLEDIC( uint8_t ic, uint8_t r, uint8_t g, uint8_t b ) {
  if ( ic == 0 )  {
    for(int i=0; i<16; i++) {
      strip2.setPixelColor(i, r,g,b);
    }
    strip2.show();
    for(int i=0; i<8; i++) {
      strip1.setPixelColor(i, r,g,b);
    }
    strip1.show();
  } else if ( ic < 3 ) {
    uint8_t offset = ((ic==2)?8:0);
    for(int i=0; i<8; i++) {
      strip2.setPixelColor(i+offset, r,g,b);
    }
    strip2.show();
  } else {
    uint8_t offset = ((ic==4)?5:0);
    for(int i=0; i<3; i++) {
      strip1.setPixelColor(i+offset, r,g,b);
    }
    strip1.show();
  }

}

// signal is the value received (already divided by 2)
void showSignal( uint8_t signal ) {
  uint8_t ic = signal / 20;
  uint8_t iv = signal % 20;

  uint8_t div = (iv%2==0)?2:1;

  if ( iv == 0 ) {
    showLEDIC( ic, 0,0,0);
  } else if ( iv < 5 ) {
    showLEDIC( ic, 0,FULL_GREEN/div,0 );
  } else if ( iv < 10 ) {
    showLEDIC( ic, FULL_RED/div,0,0 );
  } else if ( iv < 15 ) {
    showLEDIC( ic, 0,0,FULL_BLUE/div );
  } else if ( iv < 20 ) {
    showLEDIC( ic, FULL_RED/div,FULL_GREEN/div,FULL_BLUE/div );
  }
}

void showExternalChannel( uint8_t channel, int status ) {
  if ( channel < 5 ) {
    uint8_t offset = ((channel==4)?8:0);
    strip2.setPixelColor(0+offset, 0,0,(status?FULL_BLUE:0));
    strip2.setPixelColor(7+offset, 0,0,(status?FULL_BLUE:0));
    strip2.show();
  } else {
    uint8_t offset = ((channel==6)?7:0);
    strip1.setPixelColor(offset, 0,0,(status?FULL_BLUE:0));
    strip1.show();
  }    
}


//-------------------------------------------------------------------------------------------------------------------




void showAStrip( uint8_t strip  ) {

    for(int i=0; i<8; i++) {
      mysetPixelColor(strip, i, 255,0,0);
    }
    myShow( strip );

    _delay_ms (500);                                   // ...and some information
    
    for(int i=0; i<8; i++) {
      mysetPixelColor(strip, i, 0, 255,0);
    }
    myShow( strip );

    _delay_ms (500);                                   // ...and some information

    for(int i=0; i<8; i++) {
      mysetPixelColor(strip, i, 0,0,255);
    }
    myShow( strip );

    _delay_ms (500);                                   // ...and some information
}

void resetAllStrips() {

    for(int i=0; i<8; i++) {
      strip1.setPixelColor(i, 0,0,0);
      strip2.setPixelColor(i, 0,0,0);
      strip2.setPixelColor(i+8, 0,0,0);
    }
    strip1.show();
    strip2.show();
}
 

void setColor(uint8_t chann, uint8_t r, uint8_t g, uint8_t b) {
  uint16_t i;

  uint16_t s, e;
  int strip;
  
  if ( ( chann % 2 ) == 0 ) {
    s = 0;
    e = 3;
  } else {
    s = 5;
    e = 8;
  }
  
  
  if ( chann <= 2 ) {
    strip = 1;
  } else if ( chann <= 4 ) {
    strip = 2;
  } else {
    strip = 3;
  }

  for(i=s; i<e; i++) {
    mysetPixelColor( strip, i, r, g, b );
  }
  myShow( strip );


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

  
  _delay_ms (500);
  showAStrip( 1 );
  _delay_ms (500);
  showAStrip( 2 );
  _delay_ms (500);
  showAStrip( 3 );
  _delay_ms (500);
  resetAllStrips();
  
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
  
  setColor( channel, 0, 0, 255 );

}


void ledStatusSwitch(uint8_t channel, uint8_t status) {

  #ifdef SER_DBG
    dbg << F("ledStatusSwitch: ") << channel << "  status: " << status << "\n";
  #endif
  if ( channel == 1 ) {
    // signalling is coming in channel 0 only
    uint8_t signal = status/2;

    // minimum sanity check
    if ( signal < 100 ) {
      #ifdef SER_DBG
        dbg << F("ledStatusSwitch: signaling ") << signal << "\n";
      #endif
      showSignal( signal );
    }      
  }
  
}

void channelSwitch(uint8_t channel, uint8_t status) {
// switching the relay, could be done also by digitalWrite(3,HIGH or LOW)
  #ifdef SER_DBG
    dbg << F("channelSwitch: ") << channel << ", " << status << "\n";
  #endif

  showExternalChannel( channel, status );
  

}

int mode_ch1 = 0;


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


