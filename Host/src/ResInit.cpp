#include "ResInit.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

namespace resin {
    const int SEA = 6;
    const int SEB = 0;
}

resin::TerminalConfiguration::TerminalConfiguration(int fd) : fd(fd) {
    tcgetattr(fd, &oldset);
    newset = oldset;

    cfsetispeed(&newset, B115200);
    cfsetospeed(&newset, B115200);

    newset.c_cflag &= ~PARENB;
    newset.c_cflag &= ~CSTOPB;
    newset.c_cflag &= ~CSIZE;
    newset.c_cflag |= CS8;
    newset.c_cflag &= ~CRTSCTS;
    newset.c_cflag |= CREAD | CLOCAL;

    newset.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    newset.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN | ECHOK | ECHOCTL | ECHOKE);
    newset.c_oflag &= ~OPOST;

    newset.c_cc[VMIN] = SEA;
    newset.c_cc[VTIME] = SEB;
    tcsetattr(fd, TCSANOW, &newset);
}
resin::TerminalConfiguration::~TerminalConfiguration() {
    tcsetattr(fd, TCSANOW, &oldset);
    std::cout << "Terminal config restored\n";
}

resin::InputPort::InputPort(const std::string& port) {
    fildesc = open (port.c_str(), O_RDWR | O_NOCTTY);
    if (fildesc == -1) throw PortUnavailable();
}
resin::InputPort::~InputPort() {
    std::cout << "Closing input port at session end\n";
    close(fildesc);
}

resin::OpenDirectory::OpenDirectory(const std::string& dirname) {
    dir = opendir(dirname.c_str());
}
resin::OpenDirectory::~OpenDirectory() {
    closedir(dir);
    std::cout << "Scanned Directory closed\n";
}
