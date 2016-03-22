#include "general.h"
#include "csocketfsm.h"
#include "cserversocketfsm.h"
#include <string>
#include <iostream>
#include <vector>
#include <sys/epoll.h>

using namespace ydd;
using namespace std;

CSocketFsm::StateTable* getServerStateTable();

int main(int argc, char *argv[])
{
    int efd;
    const int maxevents = 10;
    struct epoll_event events[maxevents];

    openlog("ydd-client", LOG_PID, LOG_USER);

    efd = epoll_create1(0);
    if(efd == -1)
    {
	msyslog(LOG_ERR, "Failed epoll_create1");
	return -1;
    }
    
    struct sockaddr ai_addr;
    if(CSocket::getAddrinfo("192.168.1.84", "11437", ai_addr) != 0)
	return -1;
    CSocketFsm::StateTable* t = getServerStateTable();
    CServerSocketFsm sfsm(&ai_addr, true, -1, efd, true, t, true);
    delete t;
    sfsm.processSignal(CSocketFsm::sig_empty);

    enum appStates {doEpoll, listeningSockErr};
    appStates appState = doEpoll;

    while(appState == doEpoll)
    {
	int n = epoll_pwait(efd, events, maxevents, -1, NULL);
	int i;
	for(i = 0; i < n; i++)
	{
	    if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
		msyslog(LOG_ERR, "epoll error on socket %d", events[i].data.fd);
		if(events[i].data.fd == sfsm.sockfd())
		{
		    sfsm.processSignal(CSocketFsm::sig_err);
		}
		else
		{
		    CServerAcceptedFsm* client = sfsm.getClientBySockfd(events[i].data.fd);
		    if(client != NULL)
			client->processSignal(CSocketFsm::sig_err);
		}
		continue;
	    }
	}
	if(sfsm.getState() != CServerSocketFsm::q_waitIncomings)
	    appState = listeningSockErr;
    }

    closelog();
    return 0;
}

CSocketFsm::StateTable* getServerStateTable()
{
    CSocketFsm::StateLine e(CSocketFsm::NUM_SIGNALS, CServerSocketFsm::q_none);
    CSocketFsm::StateTable* t = new CSocketFsm::StateTable(CServerSocketFsm::NUM_STATES, e);

    (*t)[CServerSocketFsm::q_none][CSocketFsm::sig_empty] = CServerSocketFsm::q_getSockFd;

    (*t)[CServerSocketFsm::q_getSockFd][CSocketFsm::sig_noerr] = CServerSocketFsm::q_bind;
    (*t)[CServerSocketFsm::q_getSockFd][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    (*t)[CServerSocketFsm::q_bind][CSocketFsm::sig_noerr] = CServerSocketFsm::q_makeNonBlocking;
    (*t)[CServerSocketFsm::q_bind][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    (*t)[CServerSocketFsm::q_makeNonBlocking][CSocketFsm::sig_noerr] = CServerSocketFsm::q_setListening;
    (*t)[CServerSocketFsm::q_makeNonBlocking][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    (*t)[CServerSocketFsm::q_setListening][CSocketFsm::sig_noerr] = CServerSocketFsm::q_waitIncomings;
    (*t)[CServerSocketFsm::q_setListening][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    (*t)[CServerSocketFsm::q_waitIncomings][CSocketFsm::sig_epollin] = CServerSocketFsm::q_processIncomings;
    (*t)[CServerSocketFsm::q_waitIncomings][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    (*t)[CServerSocketFsm::q_processIncomings][CSocketFsm::sig_noerr] = CServerSocketFsm::q_waitIncomings;
    (*t)[CServerSocketFsm::q_processIncomings][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;
    return t;
}
