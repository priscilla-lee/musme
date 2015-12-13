/*******************************************************************************
Authors: Amal Tidjani, Eileen Cho, Priscilla Lee
Class: CS 320 Tangible User Interfaces
Project: Musme

Credit: based off of sketchbook example code Midi_Piano.ino from Bare Conductive
*******************************************************************************/

// serial rate
#define baudRate 57600

// include the relevant libraries
#include <MPR121.h>
#include <Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(12, 10); // Soft TX on 10, we don't use RX in this code

// Touch Board Setup variables
#define firstNote 2
#define lastNote 9

// VS1053 setup
byte note = 0; // The MIDI note value to be played
byte resetMIDI = 8; // Tied to VS1053 Reset line
byte ledPin = 13; // MIDI traffic inidicator
byte count = 0;

//define notes
const byte notes[] = {67,69,71,72,74,76,78,79}; //55, 57, 59, 60, 62, 64, 66, 67}; 
//const byte notes[] = {60, 62, 64, 65, 67, 69, 71, 72}; 

void setup(){
  Serial.begin(baudRate);
  
  // Setup soft serial for MIDI control
  mySerial.begin(31250);
  Wire.begin();
   
  // 0x5C is the MPR121 I2C address on the Bare Touch Board
  if(!MPR121.begin(0x5C)){ 
    Serial.println("error setting up MPR121");  
    switch(MPR121.getError()){
      case NO_ERROR:
        Serial.println("no error");
        break;  
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;      
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;      
    }
    while(1);
  }
  
  // pin 4 is the MPR121 interrupt on the Bare Touch Board
  MPR121.setInterruptPin(4);
  // initial data update
  MPR121.updateTouchData();

  // Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  
  // initialise MIDI
  setupMidi();
}

void loop(){
   if(MPR121.touchStatusChanged()){
    //Serial.println("touched");
     MPR121.updateAll();
     
     //tap notes
     for(int i=firstNote; i<=lastNote; i++){    
       if(MPR121.isNewTouch(i)){
        Serial.print("touched ");
        Serial.println(count);
        count = count+1;
         note = notes[i-firstNote];
         //channel 1 (0x90), note value, 75% velocity (0x60):
         noteOn(0, note, 0x50);
         //Serial.println(note);
       } else if(MPR121.isNewRelease(i)) {   
         //noteOff(0, note, 0x60); //turn off note with off/release velocity
       }
     }
   }
}

// functions below are little helpers based on using the SoftwareSerial
// as a MIDI stream input to the VS1053 - all based on stuff from Nathan Seidle

// Send a MIDI note-on message.  Like pressing a piano key.
// channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

// Send a MIDI note-off message.  Like releasing a piano key.
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

// Sends a generic MIDI message. Doesn't check to see that cmd is greater than 127, 
// or that data values are less than 127.
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  // Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  // (sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}

// SETTING UP THE INSTRUMENT:
// The below function "setupMidi()" is where the instrument bank is defined. Use the VS1053 instrument library
// below to aid you in selecting your desire instrument from within the respective instrument bank


void setupMidi(){
  
  // Volume - don't comment out this code!
  talkMIDI(0xB0, 0x07, 127); //0xB0 is channel message, set channel volume to max (127)
  
  // ---------------------------------------------------------------------------------------------------------
  // Melodic Instruments GM1 
  // ---------------------------------------------------------------------------------------------------------
  // To Play "Electric Piano" (5):
  talkMIDI(0xB0, 0, 0x00); // Default bank GM1  
  // We change the instrument by changing the middle number in the brackets 
  // talkMIDI(0xC0, number, 0); "number" can be any number from the melodic table below
  talkMIDI(0xC0, 13, 0); // Set instrument number. 0xC0 is a 1 data byte command(55,0) 
  // ---------------------------------------------------------------------------------------------------------
  // Percussion Instruments (Drums, GM1 + GM2) 
  // ---------------------------------------------------------------------------------------------------------  
  // uncomment the two lines of code below to use - you will also need to comment out the two "talkMIDI" lines 
  // of code in the Melodic Instruments section above 
  // talkMIDI(0xB0, 0, 0x78); // Bank select: drums
  // talkMIDI(0xC0, 0, 0); // Set a dummy instrument number
  // ---------------------------------------------------------------------------------------------------------
  
}

/* MIDI INSTRUMENT LIBRARY: 

MELODIC INSTRUMENTS (GM1) 
When using the Melodic bank (0x79 - same as default), open chooses an instrument and the octave to map 
To use these instruments below change "number" in talkMIDI(0xC0, number, 0) in setupMidi()


1   Acoustic Grand Piano       33  Acoustic Bass             65  Soprano Sax           97   Rain (FX 1)
2   Bright Acoustic Piano      34  Electric Bass (finger)    66  Alto Sax              98   Sound Track (FX 2)
3   Electric Grand Piano       35  Electric Bass (pick)      67  Tenor Sax             99   Crystal (FX 3)
4   Honky-tonk Piano           36  Fretless Bass             68  Baritone Sax          100  Atmosphere (FX 4)
5   Electric Piano 1           37  Slap Bass 1               69  Oboe                  101  Brigthness (FX 5)
6   Electric Piano 2           38  Slap Bass 2               70  English Horn          102  Goblins (FX 6)
7   Harpsichord                39  Synth Bass 1              71  Bassoon               103  Echoes (FX 7)
8   Clavi                      40  Synth Bass 2              72  Clarinet              104  Sci-fi (FX 8) 
9   Celesta                    41  Violin                    73  Piccolo               105  Sitar
10  Glockenspiel               42  Viola                     74  Flute                 106  Banjo
11  Music Box                  43  Cello                     75  Recorder              107  Shamisen
12  Vibraphone                 44  Contrabass                76  Pan Flute             108  Koto
13  Marimba                    45  Tremolo Strings           77  Blown Bottle          109  Kalimba
14  Xylophone                  46  Pizzicato Strings         78  Shakuhachi            110  Bag Pipe
15  Tubular Bells              47  Orchestral Harp           79  Whistle               111  Fiddle
16  Dulcimer                   48  Trimpani                  80  Ocarina               112  Shanai
17  Drawbar Organ              49  String Ensembles 1        81  Square Lead (Lead 1)  113  Tinkle Bell
18  Percussive Organ           50  String Ensembles 2        82  Saw Lead (Lead)       114  Agogo
19  Rock Organ                 51  Synth Strings 1           83  Calliope (Lead 3)     115  Pitched Percussion
20  Church Organ               52  Synth Strings 2           84  Chiff Lead (Lead 4)   116  Woodblock
21  Reed Organ                 53  Choir Aahs                85  Charang Lead (Lead 5) 117  Taiko
22  Accordion                  54  Voice oohs                86  Voice Lead (Lead)     118  Melodic Tom
23  Harmonica                  55  Synth Voice               87  Fifths Lead (Lead 7)  119  Synth Drum
24  Tango Accordion            56  Orchestra Hit             88  Bass + Lead (Lead 8)  120  Reverse Cymbal
25  Acoustic Guitar (nylon)    57  Trumpet                   89  New Age (Pad 1)       121  Guitar Fret Noise
26  Acoutstic Guitar (steel)   58  Trombone                  90  Warm Pad (Pad 2)      122  Breath Noise
27  Electric Guitar (jazz)     59  Tuba                      91  Polysynth (Pad 3)     123  Seashore 
28  Electric Guitar (clean)    60  Muted Trumpet             92  Choir (Pad 4)         124  Bird Tweet
29  Electric Guitar (muted)    61  French Horn               93  Bowed (Pad 5)         125  Telephone Ring
30  Overdriven Guitar          62  Brass Section             94  Metallic (Pad 6)      126  Helicopter
31  Distortion Guitar          63  Synth Brass 1             95  Halo (Pad 7)          127  Applause
32  Guitar Harmonics           64  Synth Brass 2             96  Sweep (Pad 8)         128  Gunshot  

PERCUSSION INSTRUMENTS (GM1 + GM2)

When in the drum bank (0x78), there are not different instruments, only different notes.
To play the different sounds, select an instrument # like 5, then play notes 27 to 87.
 
27  High Q                     43  High Floor Tom            59  Ride Cymbal 2         75  Claves 
28  Slap                       44  Pedal Hi-hat [EXC 1]      60  High Bongo            76  Hi Wood Block
29  Scratch Push [EXC 7]       45  Low Tom                   61  Low Bongo             77  Low Wood Block
30  Srcatch Pull [EXC 7]       46  Open Hi-hat [EXC 1]       62  Mute Hi Conga         78  Mute Cuica [EXC 4] 
31  Sticks                     47  Low-Mid Tom               63  Open Hi Conga         79  Open Cuica [EXC 4]
32  Square Click               48  High Mid Tom              64  Low Conga             80  Mute Triangle [EXC 5]
33  Metronome Click            49  Crash Cymbal 1            65  High Timbale          81  Open Triangle [EXC 5]
34  Metronome Bell             50  High Tom                  66  Low Timbale           82  Shaker
35  Acoustic Bass Drum         51  Ride Cymbal 1             67  High Agogo            83 Jingle bell
36  Bass Drum 1                52  Chinese Cymbal            68  Low Agogo             84  Bell tree
37  Side Stick                 53  Ride Bell                 69  Casbasa               85  Castanets
38  Acoustic Snare             54  Tambourine                70  Maracas               86  Mute Surdo [EXC 6] 
39  Hand Clap                  55  Splash Cymbal             71  Short Whistle [EXC 2] 87  Open Surdo [EXC 6]
40  Electric Snare             56  Cow bell                  72  Long Whistle [EXC 2]  
41  Low Floor Tom              57  Crash Cymbal 2            73  Short Guiro [EXC 3]
42  Closed Hi-hat [EXC 1]      58  Vibra-slap                74  Long Guiro [EXC 3]

*/


