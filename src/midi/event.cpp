// #include "event.hpp"
#include "midi.hpp"

namespace midi{
    void ProcessEvent(const Event* event, const EventHandler_t* eventHandler){
        if(!(event->statusByte & 0x90)) return; // not a status byte;

        uint8_t byte1 = event->len >= 1 ? event->buf[0] : 0;
        uint8_t byte2 = event->len >= 2 ? event->buf[1] : 0;
        if(event->statusByte < 0xF0){ // Channel Messages
            switch(event->statusByte & 0xF0){
                case 0x80:
                    eventHandler->NoteOffHandler(event->statusByte & 0x0F, byte1 & 0x7F, byte2 & 0x7F);
                    return;
                case 0x90:
                    eventHandler->NoteOnHandler(event->statusByte & 0x0F, byte1 & 0x7F, byte2 & 0x7F);
                    return;
                case 0xA0:
                    eventHandler->AfterTouchHandler(event->statusByte & 0x0F, byte1 & 0x7F, byte2 & 0x7F);
                    return;
                case 0xB0:
                    eventHandler->ControlChangeHandler(event->statusByte & 0x0F, byte1 & 0x7F, byte2 & 0x7F);
                    return;
                case 0xC0:
                    eventHandler->ProgramChangeHandler(event->statusByte & 0x0F, byte1 & 0x7F);
                    return;
                case 0xD0:
                    eventHandler->ChannelPressureHandler(event->statusByte & 0x0F, byte1 & 0x7F);
                    return;
                case 0xE0:
                    eventHandler->PitchBendHandler(event->statusByte & 0x0F, byte1 & 0x7F << 7 + byte2 & 0x7F);
                    return;
                default:
                    return;
            }
        } else { //SYSTEM MESSAGE
            switch(event->statusByte){
                case 0xF1:
                    // MTC
                    eventHandler->MTCQFHandler(byte1 & 0x70 >> 4, byte1 & 0x0F);
                    return;
                case 0xF2:
                    eventHandler->SongPositionPointerHandler(byte1 & 0x7F << 7 + byte2 & 0x7F);
                    return;
                case 0xF3:
                    eventHandler->SongSelectHandler(byte1 & 0x7F);
                    return;
                case 0xF4:
                case 0xF5:
                    //UNDEFINED
                    return;
                case 0xF6:
                    eventHandler->TuneRequestHandler();
                    return;
                case 0xF7:
                    eventHandler->EndOfSystemExclusiveHandler();
                    return;
                case 0xF8:
                    // MTC
                    eventHandler->TimingClockHandler();
                    return;
                case 0xF9:
                    //UNDEFINED
                    return;
                case 0xFA:
                    eventHandler->StartHandler();
                    return;
                case 0xFB: 
                    eventHandler->ContinueHandler();
                    return;
                case 0xFC:
                    eventHandler->StopHandler();
                    return;
                case 0xFD:
                    //UNDEFINED
                    return;
                case 0xFE:
                    eventHandler->ActiveSensingHandler();
                    return;
                case 0xFF:
                    eventHandler->SystemResetHandler();
                default:
                    return;
            }
        }
    }
}