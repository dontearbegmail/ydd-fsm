#ifndef CSERVERSOCKETFSM_H
#define CSERVERSOCKETFSM_H

#include "csocketfsm.h"
#include "cclientsocketfsm.h"
#include <vector>
#include <string>

namespace ydd
{
    class CServerSocketFsm : public CSocketFsm
    {
	public:
	    enum States : StateType
	    {
		q_none = CSocketFsm::q_none,
		q_shutdown = CSocketFsm::q_shutdown,
		q_getSockFd,
		q_bind,
		q_makeNonBlocking,
		q_setListening,
		q_waitIncomings,
		q_processIncomings
	    };
	    static const size_t NUM_STATES = 8;

	    CServerSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
	private:
	    static CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks getStatesCallbacksT();
	    static const CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks statesCallbacks_;
	    std::vector<CClientSocketFsm> clients_;

	    void processSignal(CSocketFsm::Signals signal);

	    void q_Bind();
	    void q_SetListening();
	    void q_WaitIncomings();
	    void q_ProcessIncomings();
    };
}

#endif /* CSERVERSOCKETFSM_H  */
