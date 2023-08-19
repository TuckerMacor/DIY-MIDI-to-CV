/*
Tucker Macor 2023

Libraries needed:
Arduino MIDI Library by Francois Best
https://github.com/FortySevenEffects/arduino_midi_library

MCP DAC by Rob Tillaart
https://github.com/RobTillaart/MCP_DAC
*/

//MIDI stuff
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();


//DAC Stuff
#define dacCount 6  // Number of DAC chips
const int dacChipSelectPins[dacCount] = { 3, 4, 5, 6, 7, 8 };
#include "MCP_DAC.h"
MCP4922 dac[dacCount];


//output shift register(s)
#define shiftOutDataPin A0
#define shiftOutClockPin A1
#define shiftOutLatchPin A2
#define shiftOutCount 5
#define shiftOutPinCount shiftOutCount * 8
boolean shiftOutStates[shiftOutPinCount];


//input shift register(s)
#define shiftInDataPin A3
#define shiftInClockPin A4
#define shiftInLatchPin A5
#define shiftInCount 1
#define shiftInPinCount shiftInCount * 8
boolean shiftInStates[shiftInPinCount];


const boolean segmentNumbers[31][7] = {
  { 1, 1, 1, 1, 1, 1, 0 },  //0
  { 0, 1, 1, 0, 0, 0, 0 },  //1
  { 1, 1, 0, 1, 1, 0, 1 },  //2
  { 1, 1, 1, 1, 0, 0, 1 },  //3
  { 0, 1, 1, 0, 0, 1, 1 },  //4
  { 1, 0, 1, 1, 0, 1, 1 },  //5
  { 1, 0, 1, 1, 1, 1, 1 },  //6
  { 1, 1, 1, 0, 0, 0, 0 },  //7
  { 1, 1, 1, 1, 1, 1, 1 },  //8
  { 1, 1, 1, 1, 0, 1, 1 },  //9
  { 0, 0, 0, 0, 0, 0, 0 },  //off
  { 1, 1, 1, 0, 1, 1, 1 },  //A
  { 0, 0, 1, 1, 1, 1, 1 },  //b
  { 1, 0, 0, 1, 1, 1, 0 },  //C
  { 0, 1, 1, 1, 1, 0, 1 },  //d
  { 1, 0, 0, 1, 1, 1, 1 },  //E
  { 1, 0, 0, 0, 1, 1, 1 },  //F
  { 1, 0, 1, 1, 1, 1, 0 },  //G
  { 0, 1, 1, 0, 1, 1, 1 },  //H
  { 0, 0, 0, 0, 1, 1, 0 },  //I
  { 0, 1, 1, 1, 1, 0, 0 },  //J
  { 0, 0, 0, 1, 1, 1, 0 },  //L
  { 0, 0, 1, 0, 1, 0, 1 },  //n
  { 0, 0, 1, 1, 1, 0, 1 },  //o
  { 1, 1, 0, 0, 1, 1, 1 },  //P
  { 0, 0, 0, 0, 1, 0, 1 },  //r
  { 0, 0, 0, 1, 1, 1, 1 },  //t
  { 0, 1, 1, 1, 1, 1, 0 },  //U
  { 0, 1, 1, 1, 0, 1, 1 },  //y
  { 1, 1, 1, 1, 0, 1, 1 },  //g
  { 0, 0, 1, 1, 1, 0, 0 },  //u
};


//buttons and trigger inputs
#define buttonCount 16
bool buttonState[buttonCount];                //what state is the button at
bool buttonWasPressed[buttonCount];           //was the button pressed
bool lastButtonState[buttonCount];            //previous state of the button
unsigned long lastDebounceTime[buttonCount];  //timestamp of debounce
unsigned long debounceDelay = 30;             //how long to wait before a button is considered pressed
const int ButtonGroupSelectPins[2] = { 2, 9 };

const int noteLetters[12] = { 13, 13, 14, 14, 15, 16, 16, 17, 17, 11, 11, 12 };
const int noteValues[128] = { -532, -489, -446, -403, -360, -317, -274, -231, -188, -145, -102, -59, -16, 26, 69, 112, 155, 198, 241, 284, 327, 370, 413, 456, 499, 542, 585, 628, 671, 714, 757, 800, 843, 886, 929, 972, 1015, 1058, 1101, 1144, 1187, 1230, 1273, 1316, 1359, 1402, 1445, 1488, 1531, 1574, 1617, 1661, 1704, 1747, 1790, 1833, 1876, 1919, 1962, 2005, 2048, 2091, 2133, 2176, 2219, 2261, 2304, 2347, 2389, 2432, 2475, 2517, 2560, 2603, 2645, 2688, 2731, 2773, 2816, 2859, 2901, 2944, 2987, 3029, 3072, 3115, 3157, 3200, 3243, 3285, 3328, 3371, 3413, 3456, 3499, 3541, 3584, 3627, 3669, 3712, 3755, 3797, 3840, 3883, 3925, 3968, 4011, 4053, 4095, 4138, 4181, 4224, 4266, 4309, 4352, 4394, 4437, 4480, 4522, 4565, 4608, 4650, 4693, 4736, 4778, 4821, 4864, 4906 };

int listenChannel[6] = { 1, 2, 3, 4, 5, 6 };

bool channelGateMute[6] = { false, false, false, false, false, false };

int displayValue[6] = { 0, 0, 0, 0, 0, 0 };
int displayOctave[6] = { 0, 0, 0, 0, 0, 0 };
int octaveShiftAmount[6] = { 0, 0, 0, 0, 0, 0 };
int notesOn[6] = { 0, 0, 0, 0, 0, 0 };
//int polyMode = 0;
//byte gateOnNote[6] = { 0, 0, 0, 0, 0, 0 };

int selectedChannel = 0;

int interfaceState = 0;  // 0 = notes 1 = channel select 2 = octave shift 3 = "off"

void setup() {
  // put your setup code here, to run once:

  setupShiftRegisterPins();

  for (int i = 0; i < dacCount; i++) {
    dac[i].begin(dacChipSelectPins[i]);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(ButtonGroupSelectPins, OUTPUT);
  }

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  bootSequence();
}


void loop() {
  MIDI.read();

  readButtons();

  for (int i = 0; i < buttonCount; i++) {  //go through all buttons
    if (buttonWasPressed[i] == true) {     //if pressed
      buttonWasPressed[i] = false;
      if (i == 6) {
        selectedChannel = 0;
      } else if (i > 7 && i < 13 ) {
        selectedChannel = i - 7;
      } else if (i == 0) {
        interfaceState = 1;
      } else if (i == 1) {
        interfaceState = 2;
      } else if (i == 2) {
        channelGateMute[selectedChannel] = !channelGateMute[selectedChannel];
        if (channelGateMute[selectedChannel]) {
          notesOn[selectedChannel] = 0;
          shiftOutStates[32 + selectedChannel] = LOW;
          shiftStatesOut();
          interfaceState = 3;
        } else {
          interfaceState = 0;
        }
      } else if (i == 3 && interfaceState == 1) {
        listenChannel[selectedChannel]--;
      } else if (i == 3 && interfaceState == 2) {
        octaveShiftAmount[selectedChannel]--;
      } else if (i == 4) {
        interfaceState = 0;
      } else if (i == 5 && interfaceState == 1) {
        listenChannel[selectedChannel]++;
      } else if (i == 5 && interfaceState == 2) {
        octaveShiftAmount[selectedChannel]++;
      }
    }
  }

  for (int i = 0; i < 6; i++) {
    shiftOutStates[i] = LOW;
  }
  shiftOutStates[selectedChannel] = HIGH;

  if (interfaceState == 0) {
    writeShiftSegment(1, listenChannel[selectedChannel]);
    if (displayValue[selectedChannel] > 0 && noteLetters[displayValue[selectedChannel]] == noteLetters[displayValue[selectedChannel] - 1]) {
      shiftOutStates[23] = HIGH;
    } else {
      shiftOutStates[23] = LOW;
    }
    writeShiftSegment(2, noteLetters[displayValue[selectedChannel]]);
    writeShiftSegment(3, displayOctave[selectedChannel]);
  } else if (interfaceState == 1) {
    writeShiftSegment(1, 21);
    writeShiftSegment(2, 13);
    writeShiftSegment(3, listenChannel[selectedChannel]);
  } else if (interfaceState == 2) {
    writeShiftSegment(1, 23);
    writeShiftSegment(2, 10);
    if (octaveShiftAmount[selectedChannel] < 0) {
      shiftOutStates[22] = HIGH;
      writeShiftSegment(3, 0 - octaveShiftAmount[selectedChannel]);
    } else {
      shiftOutStates[22] = LOW;
      writeShiftSegment(3, octaveShiftAmount[selectedChannel]);
    }
  } else if (interfaceState == 3) {
    writeShiftSegment(1, 23);
    writeShiftSegment(2, 16);
    writeShiftSegment(3, 16);
  }
  shiftStatesOut();
}


void handleNoteOn(byte channel, byte pitch, byte velocity) {

  int velocityToDac = velocity;
  velocityToDac = map(velocityToDac, 0, 127, 0, 4095);

  for (int i = 0; i < dacCount; i++) {
    if (listenChannel[i] == channel) {
      displayValue[i] = pitch + (octaveShiftAmount[i] * 12);
      dac[i].fastWriteA(noteValues[displayValue[i]]);
      dac[i].fastWriteB(velocityToDac);

      displayOctave[i] = 0;
      while (displayValue[i] > 11) {
        displayValue[i] = displayValue[i] - 12;
        displayOctave[i]++;
      }
      while (displayValue[i] < 0) {
        displayValue[i] = displayValue[i] + 12;
        displayOctave[i]--;
      }
      if (!channelGateMute[i]) {
        notesOn[i]++;
        shiftOutStates[32 + i] = HIGH;
        shiftStatesOut();
      }
    }
  }
}


void handleNoteOff(byte channel, byte pitch, byte velocity) {
  for (int i = 0; i < 6; i++) {
    if (listenChannel[i] == channel) {
      if (notesOn[i] > 0) {
        notesOn[i]--;
      }
      if (notesOn[i] == 0) {
        shiftOutStates[32 + i] = LOW;
        shiftStatesOut();
      }
    }
  }
}



void readButtons() {  //check if buttons got pressed
  bool buttonReadings[buttonCount];
  digitalWrite(ButtonGroupSelectPins[0], HIGH);
  shiftStatesIn();
  for (int i = 0; i < 8; i++) {
    buttonReadings[i] = shiftInStates[i];
  }
  digitalWrite(ButtonGroupSelectPins[0], LOW);
  digitalWrite(ButtonGroupSelectPins[1], HIGH);
  shiftStatesIn();
  digitalWrite(ButtonGroupSelectPins[1], LOW);
  for (int i = 0; i < 8; i++) {
    buttonReadings[i + 8] = shiftInStates[i];
  }
  for (int i = 0; i < buttonCount; i++) {
    if (buttonReadings[i] != lastButtonState[i]) {
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (buttonReadings[i] != buttonState[i]) {
        buttonState[i] = buttonReadings[i];
        if (buttonState[i] == LOW) {
          buttonWasPressed[i] = true;
        }
      }
    }
    lastButtonState[i] = buttonReadings[i];
  }
}



void setupShiftRegisterPins() {
  pinMode(shiftOutDataPin, OUTPUT);
  pinMode(shiftOutClockPin, OUTPUT);
  pinMode(shiftOutLatchPin, OUTPUT);
  pinMode(shiftInDataPin, INPUT);
  pinMode(shiftInClockPin, OUTPUT);
  pinMode(shiftInLatchPin, OUTPUT);
}


void bootSequence() {
  for (int i = 0; i < 31; i++) {
    for (int o = 0; o < shiftOutCount; o++) {
      writeShiftSegment(o, i);
    }
    shiftStatesOut();
    delay(150);
  }
}


void writeShiftSegmentTripleDigit(int valueToDisplay) {
  if (valueToDisplay > 999 || valueToDisplay < 0) {
    writeShiftSegment(0, 12);
  } else {
    writeShiftSegment(1, (valueToDisplay) / 100);
    writeShiftSegment(2, (((valueToDisplay) % 100) / 10));
    writeShiftSegment(3, (((valueToDisplay) % 100) % 10));
  }
  shiftStatesOut();
}


void writeShiftSegmentDoubleDigit(int valueToDisplay) {
  if (valueToDisplay > 999 || valueToDisplay < 0) {
    writeShiftSegment(0, 12);
  } else {
    writeShiftSegment(2, (((valueToDisplay) % 100) / 10));
    writeShiftSegment(3, (((valueToDisplay) % 100) % 10));
  }
}


void writeShiftSegment(int whichDisplay, int Value) {
  for (int segment = 0; segment < 7; segment++) {
    shiftOutStates[segment + (whichDisplay)*8] = segmentNumbers[Value][segment];
  }
}


void clearshiftOutStates(bool stateToApply) {
  for (int i = shiftOutPinCount - 1; i >= 0; i--) {
    shiftOutStates[i] = stateToApply;
  }
}


void shiftStatesOut() {
  digitalWrite(shiftOutClockPin, LOW);
  for (int i = shiftOutPinCount - 1; i >= 0; i--) {
    digitalWrite(shiftOutLatchPin, LOW);
    int valueToShift = shiftOutStates[i];
    digitalWrite(shiftOutDataPin, valueToShift);
    digitalWrite(shiftOutLatchPin, HIGH);
  }
  digitalWrite(shiftOutClockPin, HIGH);
}


void shiftStatesIn() {
  digitalWrite(shiftInLatchPin, LOW);
  digitalWrite(shiftInLatchPin, HIGH);
  for (int i = shiftInPinCount - 1; i >= 0; i--) {
    int valueToShift = !digitalRead(shiftInDataPin);
    shiftInStates[i] = valueToShift;
    digitalWrite(shiftInClockPin, HIGH);
    digitalWrite(shiftInClockPin, LOW);
  }
}
