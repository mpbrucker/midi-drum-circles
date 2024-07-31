#include "MIDIUSB.h"

// NEOPIXEL CONTENT
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))




const int FILTER_VAL = 50;
const int DEBOUNCE_MS = 50;
const float THRESHOLD[] = {80.0, 50.0, 40.0, 30.0, 30.0, 50.0};
//const float THRESHOLD = 25.0;
const int VEL_WINDOW = 10;

int sensorValue = 0;
int sumVals = 0;
float avgVal = 0.0;

int sensorValues[6][FILTER_VAL];
float samples[6][VEL_WINDOW];
int sensorPins[] = {A0, A1, A2, A3, A4, A5};
//int sensorPins[] = {A5, A1, A4, A0, A3, A2};
bool isTriggered[] = {false, false, false, false, false, false};
bool isNoteOn[] = {false, false, false, false, false, false};
int lastTriggered[] = {0, 0, 0, 0, 0, 0}; // To keep track of timing for MIDI signals
long lastTriggerTime[] = {0, 0, 0, 0, 0, 0};
float lastPeak[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float lastValley[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float thresholds[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
long lastMeasure = micros();
int velo = 0;

float valDiff = 0.0;

// Map the inputs to notes
int instruments[] = {0, 2, 4, 6, 9, 10};


// Taken from MIDIUSB_write example
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  analogReference(EXTERNAL);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  
  for (int j = 0; j < 6; j++) {
    for (int i = 0; i < FILTER_VAL; i++) {
      sensorValues[j][i] = 0;
    }
  }
}

void loop() {

//  second_strip.rainbow((millis()*10) % 65536);
//  second_strip.show();
//  rainbow(second_strip);
//  rainbow(strip);
  
//  Serial.println(micros() - lastMeasure);s
  lastMeasure = micros();

  for (int j = 0; j < 6; j++) {
    // Update the row of the array corresponding to this sensor's input values
    sensorValue = analogRead(sensorPins[j]); // We read twice, and discard the first value, to avoid accidental input coupling
//    delayMicroseconds(50);
    sensorValue = analogRead(sensorPins[j]);
    for (int i = 0; i < FILTER_VAL - 1; i++) {
      sensorValues[j][i] = sensorValues[j][i + 1];
    }
    sensorValues[j][FILTER_VAL - 1] = sensorValue;

    // sum up all values from buffer
    sumVals = 0;
    for (int i = 0; i < FILTER_VAL; i++) {
      sumVals += sensorValues[j][i];
    }

    avgVal = float(sumVals) / FILTER_VAL;
    

    // Wait for note on
    if (avgVal > THRESHOLD[j] && !isTriggered[j]) {
      lastTriggerTime[j] = millis();
      Serial.println(j);
      Serial1.write(j);
      noteOn(0, instruments[j], 127);    // Send a Note (pitch 42, velo 127 on channel 1)
      MidiUSB.flush();   
      isTriggered[j] = true;
    } else if (isTriggered[j] && avgVal < THRESHOLD[j] && millis() - lastTriggerTime[j] > DEBOUNCE_MS) {
      noteOff(0, instruments[j], 127);
      MidiUSB.flush();
      isTriggered[j] = false;
    }
//    Serial.print(THRESHOLD);
//    Serial.print(",");
    Serial.print("A");
    Serial.print(j);
    Serial.print(": ");
    Serial.print(avgVal);
    Serial.print(","); 

  }
  
  Serial.print("\n");


}
