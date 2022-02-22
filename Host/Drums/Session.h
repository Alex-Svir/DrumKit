#ifndef __SESSION__H__
#define __SESSION__H__

#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <fcntl.h>
#include <gtk/gtk.h>
#include "Midi.h"

#define SEA 6
#define SEB 0

class Session
{
    enum Status
    {
        DISCONNECTED,
        CONNECTED,
        DISCONNECTING
    };
    Status status = DISCONNECTED;
    struct termios oldset, newset;
    struct params *prms;

    void set_terminal_configuration(int filedescr);
    void restore_terminal_configuration(int filedescr);
    void rec();
public:
    void start(struct params *prms);
    void stop();
    bool ready_for_start();
    bool ready_for_stop();
};

#endif
