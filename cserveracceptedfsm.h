#ifndef CSERVERACCEPTEDFSM_H
#define CSERVERACCEPTEDFSM_H

#include "csocketfsm.h"

namespace ydd
{
    class CServerAcceptedFsm : public CSocketFsm
    {
	public:
	    enum States : StateType
	    {
		q_initial = 0,
		q_makeNonBlocking,
		q_readEpollinPending,
		q_read,
		q_checkClientData,
		q_write,
		q_writeEpolloutPending,
		q_error,
		q_connectionClosed,
		q_shutdown
	    };
	    static const size_t NUM_STATES = 10;

	    CServerAcceptedFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
	    void processSignal(CSocketFsm::Signals signal);
	protected:
	    static CSocketFsm::TFSMHelper<CServerAcceptedFsm>::StatesCallbacks statesCallbacks_;
	    static CSocketFsm::TFSMHelper<CServerAcceptedFsm>::StatesCallbacks getStatesCallbacksT();
	    void q_Error();
	    void q_ConnectionClosed();
    };
}

#endif /* CSERVERACCEPTEDFSM_H  */
