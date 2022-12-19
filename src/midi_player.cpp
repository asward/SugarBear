#ifdef ESP32
  #include "SPIFFS.h"
#endif
#include "FS.h"
#include "midi/midi.hpp"

void NoteOn(uint8_t c, uint8_t n, uint8_t v){
    tone(BUZZER_PIN, (uint32_t) MIDI_HZ(n));
}

void NoteOff(uint8_t c, uint8_t n, uint8_t v){
    noTone(BUZZER_PIN);
}

fs::File file;

void PlayMidi(void * pvParameters){

  #ifdef ESP32
    file = SPIFFS.open("/sugar_bear.mid","r", false);
  #else
    file = SPIFFS.open("/sugar_bear.mid","r");
  #endif

  if(!file){
      Serial.println("Failed to open file for reading");
      return;
  } 
    
  midi::EventHandler_t sugarBear;
  sugarBear.NoteOnHandler = NoteOn;
  sugarBear.NoteOffHandler = NoteOff;

  midi::File midiFile =  midi::File(&file);
  for(int track = 1; track<= midiFile.GetNumTracks()  ; track++) {
    midiFile.PlayTrack(track, &sugarBear);
  }

  file.close();
}
void SetupMidiPlayer(){
}