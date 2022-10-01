#ifndef __RESINIT__H__
#define __RESINIT__H__

#include <termios.h>
#include <string>
#include <dirent.h>

namespace resin {
    class TerminalConfiguration {
        struct termios oldset, newset;
        int fd;
    public:
        TerminalConfiguration(int fd);
        ~TerminalConfiguration();
    };

    class InputPort {
        int fildesc;
    public:
        class PortUnavailable {};
        InputPort(const std::string&);
        ~InputPort();
        int fd() const {return fildesc;}
    };

    class OpenDirectory {
        DIR *dir;
    public:
        OpenDirectory(const std::string&);
        ~OpenDirectory();
        struct dirent* next() {return readdir(dir);}
    };
}

#endif

