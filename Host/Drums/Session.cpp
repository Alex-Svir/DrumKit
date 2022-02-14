#include "Session.h"


void Session::set_terminal_configuration(int filedescr)
{
    tcgetattr(filedescr, &oldset);
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
    tcsetattr(filedescr, TCSANOW, &newset);
}

void Session::restore_terminal_configuration(int filedescr)
{
    tcsetattr(filedescr, TCSANOW, &oldset);
}

void Session::rec()
{
    std::cout << "Record started\n";

    Midi midi;
    char buffer[6];
    int read_count;
    std::ifstream ifs(prms->bp->get_port_name());
    //  dump
    do
    {
        read_count = ifs.readsome(buffer, 6);
    }
    while (read_count>0);

    while (status == CONNECTED)
    {
        read_count = ifs.readsome(buffer, 6);
        if (read_count > 0)
        {
            midi.add((uint8_t*)buffer);

            if (read_count != 6) printf(">>>>>>>ERROR READING! READ %d BYTES!\n", read_count);
            else printf("Instr.: %d, Vel.: %3d\n", (uint8_t)buffer[0], (uint8_t)buffer[1]);

        }
    }
    ifs.close();
    midi.process(prms);
}

void Session::start(struct params *prms)
{
    if (status != DISCONNECTED) return;
    status = CONNECTED;
    int fd = open (prms->bp->get_port_name().c_str(), O_RDWR | O_NOCTTY);
    if (fd == -1)
    {
        std::cout << "\nError opening port \"" << prms->bp->get_port_name() << "\". It may be unplugged or busy\n\n";
        status = DISCONNECTED;
        return;
    }

    this->prms = prms;
    if (prms->func != NULL && prms->arg != NULL) prms->func(prms->arg);
    else std::cout << "Something is null in params" << std::endl;

    set_terminal_configuration(fd);

    rec();

    restore_terminal_configuration(fd);
    close(fd);

    std::cout << "Session ended\n\n";
    status = DISCONNECTED;
}

void Session::stop()
{
    if (status == CONNECTED)
        status = DISCONNECTING;
}

bool Session::ready_for_start()
{
    return status == DISCONNECTED;
}

bool Session::ready_for_stop()
{
    return status == CONNECTED;
}
