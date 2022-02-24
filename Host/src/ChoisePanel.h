#ifndef __CHOISEPANEL__H__
#define __CHOISEPANEL__H__

#include <cstdio>
#include <string>
#include <sstream>
#include <gtk/gtk.h>

#define INSTRUMENTS_COUNT 47
#define CHOISE_PANEL_ROWS 16

#define CHOISE_PANEL_SPACE_V 5
#define CHOISE_PANEL_SPACE_H 10
#define CHOISE_PANEL_PADDING 2

class ChoisePanel {

const char* instruments[INSTRUMENTS_COUNT] = {
        "35 Acoustic Bass Drum",
        "36 Bass Drum 1",
        "37 Side Kick",
        "38 Acoustic Snare",
        "39 Hand Clap",
        "40 Electric Snare",
        "41 Low Floor Tom",
        "42 Closed High-Hat",
        "43 High Floor Tom",
        "44 Pedal High Hat",
        "45 Low Tom",
        "46 Open High Hat",
        "47 Low-Mid Tom",
        "48 High-Mid Tom",
        "49 Crash Cymbal 1",
        "50 High Tom",
        "51 Ride Cymbal 1",
        "52 Chinese Cymbal",
        "53 Ride Bell",
        "54 Tambourine",
        "55 Splash Cymbal",
        "56 Cowbell",
        "57 Crash Cymbal 2",
        "58 Vibraslap",
        "59 Ride Cymbal 2",
        "60 High Bongo",
        "61 Low Bongo",
        "62 Mute High Conga",
        "63 Open High Conga",
        "64 Low Conga",
        "65 High Timbale",
        "66 Low Timbale",
        "67 High Agogo",
        "68 Low Agogo",
        "69 Cabasa",
        "70 Maracas",
        "71 Short Whistle",
        "72 Long Whistle",
        "73 Short Guiro",
        "74 Long Guiro",
        "75 Claves",
        "76 High Wood Block",
        "77 Low Wood Block",
        "78 Mute Cuica",
        "79 Open Cuica",
        "80 Mute Triangle",
        "81 Open Triangle"
        };

    GtkWidget *frame;
    GtkWidget *combo[CHOISE_PANEL_ROWS];

public:
    ChoisePanel();
    GtkWidget* get_pointer();
    void get_notes_map(uint8_t *buf, int len);
};

#endif
