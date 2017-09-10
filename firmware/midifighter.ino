#include "MIDIUSB.h"
#include <Adafruit_NeoPixel.h>

#define NEO_PIN 7
#define NUM_PXL 32
#define debounceDelay 5

//button states for debouncing
int state = 0;
int prev_state = 0;
unsigned long lastDebounceTime = 0;

//keep button states so we're only sending midi signals when there's a change
int previousButtons = 0;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PXL, NEO_PIN, NEO_GRB + NEO_KHZ800);

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}


void setPixels(int pinStates){
  for(int i=0; i<16; i++){
    if(bitRead(pinStates, i)){ //button down
      pixels.setPixelColor((i*2), pixels.Color(127,127,127)); //white
      pixels.setPixelColor((i*2)+1, pixels.Color(127,127,127));
      
    }else{ //button up
      pixels.setPixelColor((i*2), pixels.Color(30,10,10)); //pink
      pixels.setPixelColor((i*2)+1, pixels.Color(30,10,10));      
    }
    //
    if (bitRead(pinStates, i) != bitRead(previousButtons, i))
    {
      if (bitRead(pinStates, i))
      {
        bitWrite(previousButtons, i , 1);
        noteOn(0, (40+i), 127); //Note range 40-56
        MidiUSB.flush();
      }
      else
      {
        bitWrite(previousButtons, i , 0);
        noteOff(0, (40+i), 0);
        MidiUSB.flush();
      }
    }
  }
  pixels.show();
}

void setup() {

  /* Pin Configuration
   * btn: 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
   * pin: D0  B7  F4  F5  D1  D5  F7  F6  D6  D4  C7  C6  B4  D7  B6  B5
   */
    
  DDRB  &= ~(1<<7 | 1<<6 | 1<< 5 | 1<<4);
  PORTB |=  (1<<7 | 1<<6 | 1<< 5 | 1<<4);
  DDRC  &= ~(1<<7 | 1<<6);
  PORTC |=  (1<<7 | 1<<6);
  DDRD  &= ~(1<<7 | 1<<6 | 1<< 5 | 1<<4 | 1<<1 | 1<<0);
  PORTD |=  (1<<7 | 1<<6 | 1<< 5 | 1<<4 | 1<<1 | 1<<0);
  DDRF  &= ~(1<<7 | 1<<6 | 1<< 5 | 1<<4);
  PORTF |=  (1<<7 | 1<<6 | 1<< 5 | 1<<4);

  pixels.begin();
  Serial.begin(115200);
  
}

void loop(){

  int pinStates =   (PIND&(1<<0) ? 0 : (1<<0)) |
                    (PINB&(1<<7) ? 0 : (1<<1)) |
                    (PINF&(1<<4) ? 0 : (1<<2)) |
                    (PINF&(1<<5) ? 0 : (1<<3)) |
                    (PIND&(1<<1) ? 0 : (1<<4)) |
                    (PIND&(1<<5) ? 0 : (1<<5)) |
                    (PINF&(1<<7) ? 0 : (1<<6)) |
                    (PINF&(1<<6) ? 0 : (1<<7)) |
                    (PIND&(1<<6) ? 0 : (1<<8)) |
                    (PIND&(1<<4) ? 0 : (1<<9)) |
                    (PINC&(1<<7) ? 0 : (1<<10)) |
                    (PINC&(1<<6) ? 0 : (1<<11)) |
                    (PINB&(1<<4) ? 0 : (1<<12)) |
                    (PIND&(1<<7) ? 0 : (1<<13)) |
                    (PINB&(1<<6) ? 0 : (1<<14)) |
                    (PINB&(1<<5) ? 0 : (1<<15));

  if (pinStates != prev_state) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  if((millis()-lastDebounceTime) > debounceDelay){
    if(pinStates !=state){
      state = pinStates;
      setPixels(pinStates);
    }
  }

  prev_state = pinStates;  

}
