
#include <avr/io.h>

#include "button.h"


void initADC()
{
  ADMUX=(1<<REFS0) | (1<<REFS1);                         // For Aref=AVcc;
  ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}

void stopADC()
{
  ADMUX=0;
  ADCSRA = 0;                                       // ADC off
}

uint16_t ReadADC(uint8_t ch)
{
   //Select ADC Channel ch must be 0-7
   ch=ch&0b00000111;
   ADMUX|=ch;

   //Start Single conversion
   ADCSRA|=(1<<ADSC);

   //Wait for conversion to complete
  _delay_ms (2);     
//   while(!(ADCSRA & (1<<ADIF)));

   //Clear ADIF by writing one to it
   //Note you may be wondering why we have write one to clear it
   //This is standard way of clearing bits in io as said in datasheets.
   //The code writes '1' but it result in setting bit to '0' !!!

   ADCSRA|=(1<<ADIF);

   return(ADC);
}


uint16_t readButton()
{
  initADC();
  
  _delay_ms (2);     
  
  uint16_t value =  ReadADC( 0 );
  
  stopADC();
  
  return value;
}
