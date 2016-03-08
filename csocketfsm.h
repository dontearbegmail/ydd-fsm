#ifndef CSOCKETFSM_H
#define CSOCKETFSM_H

#include "general.h"
#include "csocket.h"
#include <vector>

namespace ydd 
{
    class CSocketFsm 
    {
	public:
	    enum States 
	    {
		q_none,
		q_resolve,
		q_getSockFd,
		q_makeNonBlocking,
		q_epoll,
		q_connect,
		q_read,
		q_readEpollinPending,
		q_write,
		q_writeEpolloutPending,
		q_shutdown
	    };

	    enum Signals
	    {
		sig_noerr,
		sig_err,
		sig_epollin,
		sig_epollout,
		sig_empty
	    };
	    const static int NUM_SIGNALS = 5;

	    typedef std::vector<CSocketFsm::States> StateLine;
	    typedef std::vector<StateLine> StateTable;

	    void processSignal(CSocketFsm::Signals signal);

	private:
	    CSocket socket_;
	    CSocketFsm::StateTable* table_;
	    bool needDeleteTable_;
	    CSocketFsm::States state_;
	    void q_Resolve();
	    void q_GetSockFd();
	    void q_MakeNonBlocking();
	    void q_Shutdown();

	public:
	    CSocketFsm(char const* host, char const* port, bool isListening, int epollfd, 
		    StateTable* table, bool copyTable);
	    ~CSocketFsm();
    };
}

#endif /* CSOCKETFSM_H */
