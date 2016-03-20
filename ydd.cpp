#include "general.h"
#include "csocketfsm.h"
#include "cserversocketfsm.h"
#include <string>
#include <iostream>
#include <vector>
#include <sys/epoll.h>

using namespace ydd;
using namespace std;
using ydd::CSocketFsm;
using ydd::CServerSocketFsm;

namespace ydd
{
    template <class T> struct method_ptr
    {
	typedef int (T::*Function)();
	typedef std::vector<Function> FunctionsTable;

	static Function getFunction(FunctionsTable& t, CSocketFsm::StateType state)
	{
	    Function f = NULL;

	    f = t.at(state);

	    return f;
	}

    };


    class A {
	public:
	    static const int s = 1;
	    method_ptr<A>::FunctionsTable ft;

	    template<class T> void asd(typename method_ptr<T>::FunctionsTable& f)
	    {
	    }
	    virtual int getS()
	    {
		asd<A>(ft);
		return A::s;
	    }
	    A()
	    {
		cout << "A->getS() " << this->getS() << endl;
	    }

    };




    class B : public A 
    {
	public:
	    static const int s = 3;
	virtual int getS()
	{
	    return B::s;
	}
	B()
	{
	    cout << "B->getS() " << this->getS() << endl;
	}
    };


}

int main(int argc, char *argv[])
{
    int efd;
    openlog("ydd-client", LOG_PID, LOG_USER);

    CSocketFsm::StateLine e(CSocketFsm::NUM_SIGNALS, CServerSocketFsm::q_none);
    CSocketFsm::StateTable t(CServerSocketFsm::NUM_STATES, e);

    t[CServerSocketFsm::q_none][CSocketFsm::sig_empty] = CServerSocketFsm::q_getSockFd;

    t[CServerSocketFsm::q_getSockFd][CSocketFsm::sig_noerr] = CServerSocketFsm::q_bind;
    t[CServerSocketFsm::q_getSockFd][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    t[CServerSocketFsm::q_bind][CSocketFsm::sig_noerr] = CServerSocketFsm::q_makeNonBlocking;
    t[CServerSocketFsm::q_bind][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    t[CServerSocketFsm::q_makeNonBlocking][CSocketFsm::sig_noerr] = CServerSocketFsm::q_setListening;
    t[CServerSocketFsm::q_makeNonBlocking][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    t[CServerSocketFsm::q_setListening][CSocketFsm::sig_noerr] = CServerSocketFsm::q_waitIncomings;
    t[CServerSocketFsm::q_setListening][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    t[CServerSocketFsm::q_waitIncomings][CSocketFsm::sig_epollin] = CServerSocketFsm::q_processIncomings;
    t[CServerSocketFsm::q_waitIncomings][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    t[CServerSocketFsm::q_processIncomings][CSocketFsm::sig_noerr] = CServerSocketFsm::q_waitIncomings;
    t[CServerSocketFsm::q_processIncomings][CSocketFsm::sig_err] = CServerSocketFsm::q_shutdown;

    struct sockaddr ai_addr;
    if(CSocket::getAddrinfo("192.168.1.84", "11437", ai_addr) != 0)
	return -1;

    efd = epoll_create1(0);
    if(efd == -1)
    {
	msyslog(LOG_ERR, "Failed epoll_create1");
	return -1;
    }

    CServerSocketFsm sfsm(&ai_addr, true, -1, efd, true, &t, true);
    std::string s, p;
    CSocket::getIpString(*sfsm.socket_.ai_addr_, s);
    cout << s << endl;
    sfsm.socket_.getHostPortStrings(s, p);
    cout << s << ":" << p << endl;

    sfsm.processSignal(CSocketFsm::sig_empty);
    int maxevents = 10;
    struct epoll_event events[maxevents];
    while(1)
    {
	int n = epoll_pwait(efd, events, maxevents, -1, NULL);
	int i;
	for(i = 0; i < n; i++)
	{
	    CSocketFsm* fsm;
	    if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
		msyslog(LOG_ERR, "epoll error on socket %d", events[i].data.fd);
		close(events[i].data.fd);
		continue;
	    }
	}
    }

    closelog();
    return 0;
}
