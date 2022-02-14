#ifndef __MIDI__H__
#define __MIDI__H__

#define NOTE_LENGTH 50
#define END_LEN 4
#define TEMPO_LEN 7
#define TMP_FILE_NAME "tmp"

#include <fstream>
#include "ChoisePanel.h"
#include "TopPanel.h"
#include "BottomPanel.h"

struct params
{
  ChoisePanel *cp;
  TopPanel *tp;
  BottomPanel *bp;
  void (*func)(gpointer);
  gpointer arg;
};

typedef struct _midi_event {
    enum Status_Byte {
        SB_ON = 0x99,
        SB_OFF = 0x89
    };
    _midi_event *next = nullptr;
    uint8_t note;
    uint8_t velocity;
    uint32_t moment;
    uint8_t status_byte;

    _midi_event(uint8_t note, uint8_t velocity, uint32_t moment, uint8_t status_byte=SB_ON);
} midi_event;

class Midi {
    midi_event *midi_start = nullptr;
    midi_event *midi_end = nullptr;

    int preprocess_midi_chain(uint32_t time_0, int bpm, int ppqn);
    void close_event(midi_event *event);

    uint32_t extract_moment(uint8_t* buf);
    uint32_t calc_abs_ticks(uint32_t delta_moment, int bpm, int ppqn);
    int ticks_to_vlq(uint8_t* buf, uint32_t ticks);

    uint32_t save_temp_file(std::fstream&, uint8_t *channels_map);
    void write_mthd(std::fstream& fs, int ppqn);
    void write_mtrk_hdr(std::fstream& fs, uint32_t len);
    void write_tempo(std::fstream& fs, int bpm);
    void copy_mtrk(std::fstream& tmp, std::fstream& midi);
    void write_end(std::fstream& fs);

public:
    ~Midi();
    void add(uint8_t* buf);
    void process(struct params *prms);
};

#endif
