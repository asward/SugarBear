// #include "file.hpp"
#include <iostream>
// #include "common.hpp"
// #include "event.hpp"
#include "midi.hpp"
#include <Arduino.h>
//MUST stay in status until changed
//MAY ignore unimplemented status bytes
//SHOULD wait for all databytes for a given status message to take action
//MUST end systems message on any other status code
//status bytes are followed by one OR two data bytes
namespace midi{

  File::File(fs::File *file){
    midiFile = file;
    
    ReadHeader();
  };
  void File::ReadHeader(){
    midiFile->seek(0);

    header = ReadInt();
    Serial.print("Header: ");
    Serial.println(header, HEX);

    header_len = ReadInt();
    Serial.print("Header Len: ");
    Serial.println(header_len);

    format_spec = ReadShort();
    Serial.print("Format: ");
    Serial.println(format_spec);
    
    num_tracks = ReadShort();
    Serial.print("Tracks: ");
    Serial.println(num_tracks);

    delta_time_len = ReadShort();
    Serial.print("Delta Time: ");
    Serial.println(delta_time_len);
  }
  void File::Play(){
    play = true;
    stop = false;
    pause = false;
  }
  void File::Stop(){
    stop = true;
    play = false;
    pause = false;
    Serial.println("Stop!");
  }
  void File::QueueTrack(uint8_t track){
    Serial.println("Playing track");

    if(!SeekTrack(track)){
        Serial.println("Error seeking to track");
        return;
    }

    //Read the HEADER
    ReadInt(); //MTrk
    uint32_t trackLenRemaining = ReadInt(); //Track length
    Serial.print("Track Len: ");
    Serial.println(trackLenRemaining);
  }
  void File::PlayTrack(uint8_t track, const midi::EventHandler* eventHandler){
    QueueTrack(track);
    Play();

    uint32_t deltaTime;
    // Start parsing the events
    while(!stop){
      deltaTime = ReadVarLen(); //TODO handle this - process events on a separate thread with delayed interupt?
      Event_t* event = new Event_t;
      if(TryGetEvent(*event)){
        if(event->statusByte == 0xFF){
          //Ignore delta time and handle immedietly
          HandleMetaEvent(event);
        } else{
          // delay, I guess :-( TODO
          delay((double) deltaTime / 2.0); //Figure out the real timing here
          midi::ProcessEvent(event, eventHandler);
          // eventProcessor->ProcessEvent(event);
        }
      } else{
        Serial.println("No Event!");
      }
      delete event; //TODO do this someplace else?
      event = NULL;
    }
    Stop();
  }
  void File::HandleMetaEvent(Event* event){

    Serial.println("Meta Event!");
    // 00 02 ss ss - Sequence Number
    // 01 len text - Text Event
    // 02 len text - Copyright
    // 03 len text - Sequence/Track Name
    // 04 len text - Instrument Name
    // 05 len text - Lyric
    // 06 len text - Marker
    // 07 len text - Cue Point
    // 20 01 cc - MIDI channel prefix
    // 2F 00 - End Of Track
    // 51 03 tt tt tt - Set Temp
    // 54 05 hr mn se fr ff - SMPTE Offset
    // 58 04 nn dd cc bb - Time Signature
    // 59 02 sf mi - Key Signature
    // 7F len data -Sequence-Specific Meta Event
    if(event->buf[0] == 0x2F){
      Stop();
    }
  }
  bool File::TryGetEvent(Event& event){

    event.statusByte = ReadByte();
    
    if(!CHECK_BIT(event.statusByte,7)) return false; //Not a status byte

    if(event.statusByte < 0xF0){ // Channel Messages
        event.len = ((event.statusByte < 0xC0) || (event.statusByte == 0xE0)) ? 2 : 1 ;
        event.buf = (uint8_t*)malloc(event.len*sizeof(uint8_t));
        ReadBytesToBuffer(event.buf, event.len);
        return true;
    } else if(event.statusByte == 0xF0 || event.statusByte == 0xF7){ // System Exlusive Messages
        SkipBytes(ReadVarLen()); //Ignore these - for now
        return false;
        // event.len = ReadVarLen();
        // event.buf = (uint8_t*)malloc(event.len*sizeof(uint8_t));
        // ReadBytesToBuffer(event.buf, event.len);
        // return true;
    // } else if(event.statusByte <= 0xF7 && event.statusByte >= 0xF1){ // System Common Messages
    //     event.len = (event.statusByte <= 0xF3 ? 1 : 0) + (event.statusByte == 0xF2 ? 1 : 0);
    //     event.buf = (uint8_t*)malloc(event.len*sizeof(uint8_t));
    //     ReadBytesToBuffer(event.buf, event.len);
    //     return true;
    }else if(event.statusByte == 0xFF){ // Meta Messages
        // TODO read something
        uint8_t metaEventType = ReadByte();
        event.len = ReadVarLen() + 1;
        event.buf = (uint8_t*)malloc(event.len*sizeof(uint8_t));
        event.buf[0] = metaEventType;
        ReadBytesToBuffer(event.buf+1, event.len-1);
        return true;
    }
    return false; //Didn't match a known event
  }

  bool File::SeekTrack(uint8_t track){
    if(track > num_tracks) return false;


    midiFile->seek(0);  //Goto start of file
    uint32_t nextTrackPosition = 0;
    nextTrackPosition += 8 + header_len; //MThd (4 bytes)  + File length (4 bytes) + Number of bytes in the header

    midiFile->seek(nextTrackPosition); //Seek to start of first track
    uint32_t current_track = 1;

    while(current_track != track){
      ReadInt(); //MTrk
      nextTrackPosition += 8 + ReadInt(); //MTrk (4 bytes)  + Track length (4 bytes) + Number of bytes in the track
      midiFile->seek(nextTrackPosition); //Seek to start of next track
      current_track++;
    }
    
    return true;
  }

  void File::TrackInfo(uint8_t track){
    SeekTrack(track);
  }


  uint32_t File::ReadBytesToBuffer(uint8_t* buf, uint32_t len) 
  { 
    if(len<=0) return 0;
    return midiFile->read(buf,len);
  }  
  uint32_t File::SkipBytes(uint32_t num) 
  { 
    midiFile->seek(num, SeekCur);
    return num ;
  }  
  
  uint32_t File::ReadVarLen() 
  { 
    uint32_t varLen;
    return ReadVarLen(varLen);
  }  
  uint32_t File::ReadVarLen(uint32_t &bytes_read) 
  { 
    //Read VarLen midi bytes. Each byte will have bit 7 set until the last byte
    uint8_t byte;
    bytes_read = 0;
    midiFile->read(&byte, 1); bytes_read++ ;

    uint32_t value = byte & 0x7f ;
    
    while(CHECK_BIT(byte,7)){  // Hi bit set means continue
      midiFile->read(&byte, 1); bytes_read++ ;
      value = value << 7;
      value += byte & 0x7f;
    }
    // if(byte & 0x80){
    //   do { 
        
    //     bytes_read++ ;
    //     value = (value << 7) + byte & 0x7f;
    //   } while(byte & 0x80); //High bit 0 means no additional bytes for this val
    // }

    return value; 
  }  

  uint8_t File::ReadByte(){
    uint8_t value;
    midiFile->read(&value, 1);
    return value;
  }
  uint16_t File::ReadShort(){
    uint8_t *bytes = new uint8_t[2];
    midiFile->read(bytes, 2);

    uint16_t tmp = *(reinterpret_cast<uint16_t*>(bytes));
    delete bytes;

    return BIGENDIAN ? tmp : SWAP_SHORT(tmp);;
  }
  uint32_t File::ReadInt(){
    uint8_t *bytes = new uint8_t[4];
    midiFile->read(bytes, 4);

    uint32_t tmp = *(reinterpret_cast<uint32_t*>(bytes));
    delete bytes;

    return BIGENDIAN ? tmp : SWAP_INT(tmp);;
  }
  uint32_t File::SkipDataBytes() 
  { 
    //Reads through all bytes until byte with MSB is set is found, return file in position before that byte. 
    uint8_t byte;
    uint32_t bytes_read = 0;
    midiFile->read(&byte, 1); bytes_read++ ;

    while(!(byte & 0x80)){
      midiFile->read(&byte, 1); bytes_read++ ;
    }

    midiFile->seek(-1,SeekCur);
    return bytes_read-- ;
  }  
}