#include "Midi.h"

midi_event::_midi_event(uint8_t note, uint8_t velocity, uint32_t moment, uint8_t status_byte)
                        : note(note), velocity(velocity/2), moment(moment), status_byte(status_byte) {}

Midi::~Midi()
{
    midi_event *current = midi_start, *del;
    while (current)
    {
        del = current;
        current = current->next;
        delete del;
    }
}

void Midi::process(struct params *prms)
{
    if (!midi_start) return;

    uint32_t time_0 = midi_start->moment;
    int bpm = prms->tp->get_tempo();
    int ppqn = prms->tp->get_ppqn();

    int channels_number = preprocess_midi_chain(time_0, bpm, ppqn);
    uint8_t channels_map[channels_number];
    prms->cp->get_notes_map(channels_map, channels_number);

    std::fstream fs_tmp(TMP_FILE_NAME, std::fstream::in | std::fstream::out | std::fstream::trunc);
    uint32_t track_len = save_temp_file(fs_tmp, channels_map);

    std::fstream fs_midi(prms->bp->get_file_name_with_path(), std::fstream::in | std::fstream::out | std::fstream::trunc);

    write_mthd(fs_midi, ppqn);
    write_mtrk_hdr(fs_midi, TEMPO_LEN + track_len + END_LEN);
    write_tempo(fs_midi, bpm);
    copy_mtrk(fs_tmp, fs_midi);
    write_end(fs_midi);

    fs_midi.close();
    fs_tmp.close();
    remove(TMP_FILE_NAME);
}

/**
*   Processings midi-events one by one: adds closing event; turns absolute milliseconds
*   into absolute ticks; counts the number of channels.
*   @returns Number of channels in the record.
*/
int Midi::preprocess_midi_chain(uint32_t time_0, int bpm, int ppqn)
{
    uint8_t max_channel_number = 0;
    midi_event *current = midi_start;

    while (current)
    {
        close_event(current);
        //  milliseconds => abs_ticks
        current->moment = calc_abs_ticks(current->moment - time_0, bpm, ppqn);
        if (current->note > max_channel_number)
            max_channel_number = current->note;
        current = current->next;
    }
    return (int)max_channel_number + 1;
}

/**
*   Adds a closing event in appropriate position.
*/
void Midi::close_event(midi_event *event)
{
    if (event->status_byte == midi_event::SB_OFF) return;

    uint32_t new_time = event->moment + NOTE_LENGTH;

    midi_event *iter = event;
    while (iter->next && iter->next->moment <= new_time)
    {
        if (iter->next->note == event->note)
        {
            new_time = iter->next->moment;
            break;
        }
        iter = iter->next;
    }
    midi_event *close_event = new midi_event(event->note, 0, new_time, midi_event::SB_OFF);
    close_event->next = iter->next;
    iter->next = close_event;
}

void Midi::add(uint8_t *buffer)
{
    midi_event *new_event = new midi_event(buffer[0], buffer[1], extract_moment(buffer));
    if (midi_start) midi_end->next = new_event;
    else midi_start = new_event;
    midi_end = new_event;
}

uint32_t Midi::extract_moment(uint8_t* buf)
{
    uint32_t l = 0;
    l |= buf[2];
    l = l<<8;
    l |= buf[3];
    l = l<<8;
    l |= buf[4];
    l = l<<8;
    l |= buf[5];
    return l;
}

uint32_t Midi::calc_abs_ticks(uint32_t delta_moment, int bpm, int ppqn)
{
    uint64_t res = (uint64_t)delta_moment * (uint64_t)bpm * (uint64_t)ppqn /6000;
    return (uint32_t)(res%10 > 4 ? res/10+1 : res/10);
}

int Midi::ticks_to_vlq(uint8_t* buf, uint32_t ticks)
{
    int count = 1;

    for (;count<5;count++)
    {
        buf[4-count] = (uint8_t) ticks;
        buf[4-count] |= 0x80;

        if ( !(ticks /= 128) )
        {
            break;
        }
    }
    buf[3] &= (~0x80);

    if (count > 4) return 4;
    return count;
}

uint32_t Midi::save_temp_file(std::fstream& tmp_fs, uint8_t *channels_map)
{
    uint32_t last_abs_ticks = 0;
    uint8_t delta_buf[4];
    int delta_len;
    uint32_t mtrk_len = 0;

    midi_event *event = midi_start;
    while (event)
    {
        delta_len = ticks_to_vlq(delta_buf, event->moment - last_abs_ticks);
        last_abs_ticks = event->moment;
        for (int i=4-delta_len; i<4; i++)
        {
            tmp_fs << delta_buf[i];
            mtrk_len++;
        }
        tmp_fs << event->status_byte;
        tmp_fs << channels_map[event->note];
        tmp_fs << event->velocity;
        mtrk_len += 3;

        event = event->next;
    }
    return mtrk_len;
}

void Midi::write_mthd(std::fstream& fs, int ppqn)
{
    uint8_t start[] = { 0x4d, 0x54, 0x68, 0x64,
                        0x00, 0x00, 0x00, 0x06,
                        0x00, 0x00, 0x00, 0x01 };
    for (int i = 0; i < 12; i++)
    {
        fs << start[i];
    }
    if (ppqn > 0x7fff) ppqn = 24576;
    fs << (uint8_t)(ppqn >> 8);
    fs << (uint8_t)ppqn;
}

void Midi::write_mtrk_hdr(std::fstream& fs, uint32_t len)
{
    uint8_t hdr[] = { 0x4d, 0x54, 0x72, 0x6b };
    for (int i=0; i<4; i++)
        fs << hdr[i];

    for (int i=3; i>=0; i--)
        fs << (uint8_t) (len >> (i*8));
}

void Midi::write_tempo(std::fstream& fs, int bpm)
{
    uint8_t hdr[] = { 0x00, 0xff, 0x51, 0x03 };
    for (int i=0; i<4; i++)
        fs << hdr[i];

    uint32_t tempo = uint32_t(600000000) / uint32_t(bpm);
    tempo = tempo%10>4 ? tempo/10+1 : tempo/10;

    for (int i=2; i>=0; i--)
        fs << (uint8_t) (tempo >> (i*8));
}

void Midi::copy_mtrk(std::fstream& tmp, std::fstream& midi)
{
    int buf_size=1024;
    char buf[buf_size];
    tmp.seekg(0);
    do {
        tmp.read(buf, buf_size);
        midi.write(buf, tmp.gcount());
    } while (tmp.gcount()>0);
}

void Midi::write_end(std::fstream& fs)
{
    uint8_t ending[] = {0x00, 0xff, 0x2f, 0x00};
    for (int i=0; i<4; i++)
    {
        fs << ending[i];
    }
}
