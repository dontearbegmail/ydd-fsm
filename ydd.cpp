#include "general.h"
#include "csocketfsm.h"
#include "cserversocketfsm.h"
#include <string>
#include <iostream>
#include <vector>

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

	    template<class T> void asd(typename method_ptr<T>::FunctionsTable f)
	    {
	    }
	    virtual int getS()
	    {
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
    openlog("ydd-client", LOG_PID, LOG_USER);

    CSocketFsm::StateLine e(CSocketFsm::NUM_SIGNALS, CServerSocketFsm::q_none);
    CSocketFsm::StateTable t(CServerSocketFsm::NUM_STATES, e);

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
    if(CSocket::getAddrinfo("192.168.1.84", "80", ai_addr) != 0)
	return -1;

    CServerSocketFsm sfsm(&ai_addr, true, -1, -1, true, &t, true);
    
    std::string s;
    //CSocket::getIpString(*sfsm.socket_.ai_addr_, s);

    cout << s << endl;

    closelog();

    method_ptr<A>::FunctionsTable ft = {&A::getS};
    method_ptr<A>::Function f = method_ptr<A>::getFunction(ft, 0);
    A a;
    cout << (a.*f)() << endl;


    return 0;
}
