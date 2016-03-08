#include "general.h"
#include "csocketfsm.h"
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    using namespace ydd;
    using ydd::CSocketFsm;
    openlog("ydd-client", LOG_PID, LOG_USER);

    CSocketFsm::States table[3][CSocketFsm::NUM_SIGNALS] = {{CSocketFsm::q_none}};

    table[CSocketFsm::q_getsockfd][CSocketFsm::sig_noerr] = CSocketFsm::q_make_non_blocking;
    table[CSocketFsm::q_getsockfd][CSocketFsm::sig_err] = CSocketFsm::q_shutdown;


    CSocket s("yandex.ru", "11437", false);

    std::string ip;
    s.getIpString(ip);
    std::cout << ip;

    closelog();

    return 0;
}
