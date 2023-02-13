#include "Midi.h"

namespace midi {
    const uint32_t NOTE_LENGTH = 50;
    const int END_LEN = 4;
    const int TEMPO_LEN = 7;

    uint32_t calc_abs_ticks(uint32_t, int, int);
    void close_event(midievent*);
    int ticks_to_vlq(uint8_t*, uint32_t);

    void write_mthd(std::ostream&, int);
    void write_mtrk_header(std::ostream&, uint32_t);
    void write_tempo(std::ostream&, int);
    void write_mtrk_body(std::ostream&, midievent*);
    void write_end(std::ostream&);
}
//-------------------------------------------------------------------------------------------------
midi::RawMidiRecord::event::event(uint8_t *buffer) {
    next = nullptr;
    tmstmp = *(uint32_t*)(buffer + 2);
    instr = buffer[0];
    level = buffer[1];
}

void midi::RawMidiRecord::push(uint8_t *buffer) {
    event *new_event = new event(buffer);
    if (head) tail->next = new_event;
    else head = new_event;
    tail = new_event;
}

midi::RawMidiRecord::~RawMidiRecord() {
    event *current = head;
    while (current) {
        event *ev = current;
        current = current->next;
        delete ev;
    }
}
//-------------------------------------------------------------------------------------------------
midi::MidiSong::MidiSong(int bpm, int ppqn, midievent *head, int tracklen)
        : track({head, tracklen}), meta({bpm, ppqn}) {}

midi::MidiSong::~MidiSong() {
    midievent *event = track.head;
    while (event) {
        midievent *ev = event;
        event = event->next;
        delete ev;
    }
}
//-------------------------------------------------------------------------------------------------
midi::midievent::midievent(uint32_t moment, uint8_t note, uint8_t vel, Status_Byte sb)
        : next(nullptr), moment(moment), note(note), velocity(vel), status_byte(sb) {}
//-------------------------------------------------------------------------------------------------
midi::InstrumentsMap::InstrumentsMap(const InstrumentsMap& im) {
    sz = im.sz;
    instr = new uint8_t[sz];
    for (size_t i = 0; i < sz; i++) instr[i] = im.instr[i];
}
midi::InstrumentsMap& midi::InstrumentsMap::operator=(const InstrumentsMap& im) {
    if (sz != im.sz) {
        delete[] instr;
        instr = new uint8_t[sz = im.sz];
    }
    for (size_t i = 0; i < sz; i++) instr[i] = im.instr[i];
    return *this;
}
//-------------------------------------------------------------------------------------------------
midi::MidiSong* midi::create_song(RawMidiRecord *rec, int bpm, int ppqn, const InstrumentsMap& imap) {
    RawMidiRecord::event *next_src = rec->head;
    if (!next_src) return nullptr;////////////////////////////////////////////  or empty song?!?!?!?!

    uint32_t time_0 = next_src->tmstmp;
    midievent *mhead = nullptr;
    midievent **next_dest = &mhead;
    while (next_src) {
        *next_dest = new midievent(
            calc_abs_ticks(next_src->tmstmp - time_0, bpm, ppqn),
            imap[next_src->instr],
            next_src->level >> 1
        );
        next_src = next_src->next;
        next_dest = &(*next_dest)->next;
    }

    int mtrk_body_len = 0;
    uint32_t last_abs_ticks = 0;
    midievent *event = mhead;
    while (event) {
        close_event(event);

        uint32_t cur_abs_ticks = event->moment;
        event->vlq_sz = ticks_to_vlq((uint8_t*)&event->moment, cur_abs_ticks - last_abs_ticks);
        last_abs_ticks = cur_abs_ticks;

        mtrk_body_len += event->vlq_sz + 3;
        event = event->next;
    }

    return new MidiSong(bpm, ppqn, mhead, mtrk_body_len);
}

uint32_t midi::calc_abs_ticks(uint32_t delta_moment, int bpm, int ppqn) {
    uint64_t res = (uint64_t)delta_moment * (uint64_t)bpm * (uint64_t)ppqn /6000;
    return (uint32_t)(res%10 > 4 ? res/10 + 1 : res/10);
}

/**
*   Adds a closing event in appropriate position.
*/
void midi::close_event(midievent *event) {
    if (event->status_byte == midievent::SB_OFF) return;

    uint32_t new_time = event->moment + NOTE_LENGTH;

    midievent *iter = event;
    while (iter->next && iter->next->moment <= new_time) {
        if (iter->next->note == event->note) {
            new_time = iter->next->moment;
            break;
        }
        iter = iter->next;
    }
    midievent *close_event = new midievent(new_time, event->note, 0, midievent::SB_OFF);
    close_event->next = iter->next;
    iter->next = close_event;
}

int midi::ticks_to_vlq(uint8_t* buf, uint32_t ticks) {
    int count = 1;
    for (;count < 5; count++) {
        buf[4 - count] = uint8_t(ticks) | 0x80;
        if ( !(ticks >>= 7) ) break;
    }
    buf[3] &= (~0x80);
    return (count > 4) ? 4 : count;
}
//-------------------------------------------------------------------------------------------------
void midi::write_song(std::ostream& out, MidiSong* song) {
    write_mthd(out, song->meta.ppqn);
    write_mtrk_header(out, TEMPO_LEN + song->track.length + END_LEN);
    write_tempo(out, song->meta.bpm);
    write_mtrk_body(out, song->track.head);
    write_end(out);
}

void midi::write_mthd(std::ostream& out, int ppqn) {
    char start[] = { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1 };
    out.write(start, 12);
    if (ppqn > 0x7fff) ppqn = 24576;
    char *ppqn_ptr = (char*)&ppqn;
    out << *(ppqn_ptr + 1);
    out << *ppqn_ptr;
}

void midi::write_mtrk_header(std::ostream& out, uint32_t len) {
    char hdr[] = "MTrk";
    out.write(hdr, 4);
    char *len_ptr = (char*)&len;
    for (int i=3; i>=0; i--)
        out << *(len_ptr + i);
}

void midi::write_tempo(std::ostream& out, int bpm) {
    //  LITTLE ENDIAN
    uint32_t hdr = 0x0351ff00;
    out.write( (char*)&hdr, 4 );
    uint32_t tempo = uint32_t(600000000) / uint32_t(bpm);
    tempo = tempo%10 > 4 ? tempo/10 + 1 : tempo/10;
    char* tempo_ptr = (char*)&tempo;
    for (int i=2; i>=0; i--)
        out << *(tempo_ptr + i);
}

void midi::write_mtrk_body(std::ostream& out, midievent *event) {
    while (event) {
        char *vlq = ((char*) &event->moment) + 4 - event->vlq_sz;
        out.write(vlq, event->vlq_sz);

        out << event->status_byte;
        out << event->note;
        out << event->velocity;

        event = event->next;
    }
}

void midi::write_end(std::ostream& out) {
    //  LITTLE ENDIAN!!!!
    uint32_t ending = 0x002fff00;
    out.write( (char*)&ending, 4 );
}
