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
    int n, i;
    while(appState == doEpoll)
    {
	n = epoll_pwait(efd, events, maxevents, -1, NULL);
	for(i = 0; i < n; i++)
	{
	    if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) 
	    {
		msyslog(LOG_ERR, "epoll error on socket %d", events[i].data.fd);
		if(events[i].data.fd == sfsm.sockfd())
		{
		    sfsm.processSignal(CSocketFsm::sig_err);
		    break;
		}
		else
		{
		    sfsm.shutdownClient(events[i].data.fd, CSocketFsm::sig_err);
		}
		continue;
	    }
	    else if(events[i].data.fd == sfsm.sockfd())
	    {
		sfsm.processSignal(CSocketFsm::sig_epollin);
	    }
	    else if(events[i].events & EPOLLIN)
	    {
		CServerAcceptedFsm* client = sfsm.getClientBySockfd(events[i].data.fd);
		if(client == NULL)
		{
		    msyslog(LOG_WARNING, "Received EPOLLIN for sockfd = %d, but it's not in "
			    "sfsm clients", events[i].data.fd);
		    continue;
		}
		client->processSignal(CSocketFsm::sig_epollin);
		CSocketFsm::StateType clientState = client->getState();
		if(clientState == CServerAcceptedFsm::q_error || 
			clientState == CServerAcceptedFsm::q_connectionClosed)
		{
		    /* Temporary solution */
		    std::vector<std::string> clientData(client->getReadData());
		    std::vector<std::string>::iterator it;
		    cout << endl << "The whole data read from socket " << 
			events[i].data.fd << ":" << endl;
		    for(it = clientData.begin(); it != clientData.end(); it++)
			cout << *it;
		    /* END Temporary solution */
		    sfsm.shutdownClient(events[i].data.fd);
		}
		else
		{
		    cout << endl << "Have read the following data chunk from "
			"socket " << events[i].data.fd << ": " << 
			client->getReadData().back();
		}
	    }
	}
	if(sfsm.getState() != CServerSocketFsm::q_waitIncomings)
	    appState = listeningSockErr;
    }

    if(appState == listeningSockErr)
	msyslog(LOG_ERR, "Got an error on listening socket. Will shutdown");

    closelog();
    return 0;
}

CSocketFsm::StateTable* getServerStateTable()
{
    CSocketFsm::StateLine e(CSocketFsm::NUM_SIGNALS, CServerSocketFsm::q_shutdown);
    CSocketFsm::StateTable* t = new CSocketFsm::StateTable(CServerSocketFsm::NUM_STATES, e);

    for(CSocketFsm::StateType i = CServerSocketFsm::q_initial; i < CServerSocketFsm::q_error; i++)
	(*t)[i][CSocketFsm::sig_err] = CServerSocketFsm::q_error;

    (*t)[CServerSocketFsm::q_initial][CSocketFsm::sig_empty] = CServerSocketFsm::q_getSockFd;

    (*t)[CServerSocketFsm::q_getSockFd][CSocketFsm::sig_noerr] = CServerSocketFsm::q_bind;

    (*t)[CServerSocketFsm::q_bind][CSocketFsm::sig_noerr] = CServerSocketFsm::q_makeNonBlocking;

    (*t)[CServerSocketFsm::q_makeNonBlocking][CSocketFsm::sig_noerr] = CServerSocketFsm::q_setListening;

    (*t)[CServerSocketFsm::q_setListening][CSocketFsm::sig_noerr] = CServerSocketFsm::q_waitIncomings;

    (*t)[CServerSocketFsm::q_waitIncomings][CSocketFsm::sig_epollin] = CServerSocketFsm::q_processIncomings;

    (*t)[CServerSocketFsm::q_processIncomings][CSocketFsm::sig_noerr] = CServerSocketFsm::q_waitIncomings;
    return t;
}
