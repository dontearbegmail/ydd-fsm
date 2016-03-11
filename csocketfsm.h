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
	    typedef short StateType;
	    static const StateType q_none = 0, q_shutdown = 1;
	    static const size_t NUM_STATES = 0;

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

	    typedef std::vector<StateType> StateLine;
	    typedef std::vector<CSocketFsm::StateLine> StateTable;

	    void processSignal(CSocketFsm::Signals signal);

	    CSocket socket_;
	protected:
	    CSocketFsm::StateTable* table_;
	    bool needDeleteTable_;
	    StateType state_;
	    void q_GetSockFd();
	    void q_MakeNonBlocking();
	    void q_Shutdown();
	    void q_ConnectPending();
	    void q_Connect();
	    void q_ConnectCheck();

	public:
	    CSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
	    ~CSocketFsm();
	    StateType getState();
    };
}

#endif /* CSOCKETFSM_H */
