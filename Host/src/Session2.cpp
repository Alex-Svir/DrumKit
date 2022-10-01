#include "Session2.h"

#include <iostream>
#include <fstream>
#include "ResInit.h"

bool sess::Session::start(StartCallback callback) {
    if (!ready_to_start()) return false;
    status = STARTING;
    if (callback) scbck = callback;
    std::thread th(do_start, this);
    th.detach();
    return true;
}

bool sess::Session::stop(EndCallback callback) {
    if (!ready_to_stop()) return false;
    status = STOPPING;
    if (callback) ecbck = callback;
    return do_stop();
}
//===============================================================================================
bool sess::RecordSession::do_stop() {
    status = STOPPING;
    return true;
}

void sess::RecordSession::routine() try {
    std::cout << "Thread routine started\n";
    resin::InputPort ip(portname);
    resin::TerminalConfiguration termConfig(ip.fd());
    status = ON;//////////////////////////////////////////////////////////////////////////^^^^^^^^^^^^vvvvvvvvvvvvvvvORDER???????????????????????
    if (scbck) scbck(nullptr);///////////////////////////////////////////////////////////////////^^^^^^^^^^^^^^^^^^^^ORDER???????????????????????

    midi::RawMidiRecord *record = rec();

    if (ecbck) ecbck(record);
    else delete record;
    status = OFF;/////////////////////////////////////////////////////////////////////////^^^^^^^^^^^^vvvvvvvvvvvvvvvORDER???????????????????????
    std::cout << "Thread routine stopped\n";
} catch (resin::InputPort::PortUnavailable) {
    std::cout << "\nError opening port \"" << portname << "\". It may be unplugged or busy\n\n";
    status = OFF;
}

midi::RawMidiRecord* sess::RecordSession::rec() {
    std::cout << "Record started\n";
    midi::RawMidiRecord *record = new midi::RawMidiRecord();
    //Midi midi;
    char buffer[6];
    int read_count;
    std::ifstream ifs(portname);
    //  dump
    do {
        read_count = ifs.readsome(buffer, 6);
    } while (read_count > 0);

    while (status == ON)
    {
        read_count = ifs.readsome(buffer, 6);
        if (read_count > 0)
        {
            record->add((uint8_t*)buffer);

            if (read_count != 6) printf(">>>>>>>ERROR READING! READ %d BYTES!\n", read_count);
            else printf("Instr.: %d, Vel.: %3d\n", (uint8_t)buffer[0], (uint8_t)buffer[1]);

        }
    }
    ifs.close();
    //midi.process(prms);
    return record;
}
