#ifndef CSOCKETFSM_H
#define CSOCKETFSM_H

#include "general.h"
#include "csocket.h"
#include <string>

namespace ydd 
{
    class CSocketFsm 
    {
	public:
	    enum States 
	    {
		q_none,
		q_resolve,
		q_getsockfd,
		q_make_non_blocking,
		q_epoll,
		q_connect,
		q_read,
		q_read_epollin_pending,
		q_write,
		q_write_epollout_pending,
		q_shutdown
	    };

	    enum Signals
	    {
		sig_none,
		sig_noerr,
		sig_err,
		sig_epollin,
		sig_epollout
	    };
	    const static int NUM_SIGNALS = 5;

	    void processSignal(ydd::CSocketFsm::Signals signal);

	private:
	    CSocket socket_;
	    ydd::CSocketFsm::States state_;
	    void q_Resolve();
	    void q_GetSockFd();

	public:
	    CSocketFsm(char const* host, char const* port, bool isListening);
    };
}

#endif /* CSOCKETFSM_H */
