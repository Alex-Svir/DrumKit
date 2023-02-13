#ifndef __SESSION2__H__
#define __SESSION2__H__
#include <thread>
#include <string>
#include "Midi.h"

namespace sess {
    class Session;
    typedef void (*EndCallback)(void*);
    typedef void (*StartCallback)(void*);

    class Session {
    protected:
        enum Status {OFF, STARTING, ON, STOPPING};
        Status status;
        StartCallback scbck;
        EndCallback ecbck;
        bool ready_to_start() {return status == OFF;}
        bool ready_to_stop() {return status == ON || status == STARTING;}
        virtual bool do_stop() = 0;
        virtual void routine() = 0;
    public:
        virtual ~Session() {}
        bool start(StartCallback = nullptr);
        bool stop(EndCallback = nullptr);
    private:
        static void do_start(Session* s) {s->routine();}
    };

    class RecordSession : public Session {
        std::string portname;
        bool do_stop();
        void routine();
        midi::RawMidiRecord* rec();
    public:
        ~RecordSession() {stop();}
        void port(const std::string& p) {portname = p;}
    };
}

#endif
