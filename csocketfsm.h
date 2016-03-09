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
		q_connect,
		q_connectPending,
		q_connectCheck,
		q_read,
		q_readEpollinPending,
		q_write,
		q_writeEpolloutPending,
		q_sslWrite,
		q_sslWriteWantRead,
		q_sslWriteWantWrite,
		q_sslRead,
		q_sslReadWantWrite,
		q_sslReadWantRead,
		q_shutdown
	    };
	    const static size_t NUM_STATES = 18;

	    enum Signals
	    {
		sig_noerr,
		sig_err,
		sig_einprogress,
		sig_epollin,
		sig_epollout,
		sig_empty
	    };
	    const static size_t NUM_SIGNALS = 6;

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
	    void q_ConnectPending();
	    void q_Connect();

	public:
	    CSocketFsm(char const* host, char const* port, bool isListening, int epollfd, 
		    bool useEpollet, StateTable* table, bool copyTable);
	    ~CSocketFsm();
    };
}

#endif /* CSOCKETFSM_H */
