#include "general.h"
#include "csocketfsm.h"
#include <string>
#include <iostream>
#include <vector>

using namespace ydd;
using ydd::CSocketFsm;

void f(CSocketFsm** p)
{
    if(p == NULL)
	throw std::exception();
    /*CSocketFsm::StateLine q_Resolve(CSocketFsm::NUM_SIGNALS, CSocketFsm::q_none);
    q_Resolve[CSocketFsm::sig_noerr] = CSocketFsm::q_getSockFd;
    q_Resolve[CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    CSocketFsm::StateLine q_GetSockFd(CSocketFsm::NUM_SIGNALS, CSocketFsm::q_none);
    q_GetSockFd[CSocketFsm::sig_noerr] = CSocketFsm::q_makeNonBlocking;
    q_GetSockFd[CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    CSocketFsm::StateLine q_MakeNonBlocking(CSocketFsm::NUM_SIGNALS, CSocketFsm::q_none);
    q_MakeNonBlocking[CSocketFsm::sig_noerr] = CSocketFsm::q_epoll;
    q_MakeNonBlocking[CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    CSocketFsm::StateTable t = {q_Resolve, q_GetSockFd, q_MakeNonBlocking};

    *p = new CSocketFsm("lenta.ru", "80", false, -1, &t, true);

    t[CSocketFsm::q_resolve][CSocketFsm::sig_noerr] = CSocketFsm::q_connect;*/
    //int x = 0;
    //int y = x + 1;
}

int main(int argc, char *argv[])
{
    openlog("ydd-client", LOG_PID, LOG_USER);

    CSocketFsm::StateLine e(CSocketFsm::NUM_SIGNALS, CSocketFsm::q_none);
    CSocketFsm::StateTable t(CSocketFsm::NUM_STATES, e);

    t[CSocketFsm::q_getSockFd][CSocketFsm::sig_noerr] = CSocketFsm::q_makeNonBlocking;
    t[CSocketFsm::q_getSockFd][CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    t[CSocketFsm::q_makeNonBlocking][CSocketFsm::sig_noerr] = CSocketFsm::q_connect;
    t[CSocketFsm::q_makeNonBlocking][CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    t[CSocketFsm::q_connect][CSocketFsm::sig_einprogress] = CSocketFsm::q_connectPending;
    t[CSocketFsm::q_connect][CSocketFsm::sig_noerr] = CSocketFsm::q_sslWrite;
    t[CSocketFsm::q_connect][CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    t[CSocketFsm::q_connectPending][CSocketFsm::sig_epollout] = CSocketFsm::q_connectCheck;
    t[CSocketFsm::q_connectPending][CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    t[CSocketFsm::q_connectCheck][CSocketFsm::sig_noerr] = CSocketFsm::q_sslWrite;
    t[CSocketFsm::q_connectCheck][CSocketFsm::sig_err] = CSocketFsm::q_shutdown;

    //CSocketFsm sfsm("yandex.ru", "80", false, -1, true, &t, false);
    //CSocketFsm* pfsm;
    //f(&pfsm);
    closelog();


    return 0;
}
