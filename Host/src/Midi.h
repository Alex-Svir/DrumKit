#ifndef __MIDI2__H__
#define __MIDI2__H__

#include<ostream>

namespace midi {
class MidiSong;

class InstrumentsMap {
    uint8_t *instr;
    size_t sz;
public:
    InstrumentsMap(uint8_t instr[], size_t sz) : instr(instr), sz(sz) {}
    InstrumentsMap(const InstrumentsMap&);
    InstrumentsMap& operator=(const InstrumentsMap&);
    ~InstrumentsMap() {delete[] instr;}
    uint8_t operator[](size_t i) const {if (i < sz) return instr[i]; return 0;}
};

class RawMidiRecord {
    public:
        struct event {
            event *next;
            uint32_t tmstmp;
            uint8_t instr;
            uint8_t level;
            event(uint8_t*);
        };
        ~RawMidiRecord();
        void push(uint8_t*);
    private:
        event *head = nullptr;
        event *tail = nullptr;
        friend MidiSong* create_song(RawMidiRecord*,int,int, const InstrumentsMap&);
};

struct midievent {
    enum Status_Byte {SB_ON = 0x99, SB_OFF = 0x89};
    midievent *next;
    int vlq_sz;
    uint32_t moment;
    uint8_t note;
    uint8_t velocity;
    uint8_t status_byte;
    midievent(uint32_t, uint8_t, uint8_t, Status_Byte = SB_ON);
};

struct MidiTrack {
    midievent *head;
    int length;
};

//MidiChannel
class MidiSong {
public:
    struct MetaData {int bpm; int ppqn;};
private:
public:
    MetaData meta;
    MidiTrack track;
    MidiSong(int,int,midievent*,int);
    ~MidiSong();
};

MidiSong* create_song(RawMidiRecord*,int,int, const InstrumentsMap&);
void write_song(std::ostream&,MidiSong*);
}

#endif
