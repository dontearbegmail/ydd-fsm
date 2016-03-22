#ifndef CSERVERSOCKETFSM_H
#define CSERVERSOCKETFSM_H

#include "csocketfsm.h"
#include "cserveracceptedfsm.h"
#include <map>

namespace ydd
{
    class CServerSocketFsm : public CSocketFsm
    {
	public:
	    enum States : StateType
	    {
		q_initial = CSocketFsm::q_initial,
		q_shutdown = CSocketFsm::q_shutdown,
		q_getSockFd,
		q_bind,
		q_makeNonBlocking,
		q_setListening,
		q_waitIncomings,
		q_processIncomings,
		q_error
	    };
	    static const size_t NUM_STATES = 9;

	    CServerSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
	    ~CServerSocketFsm();
	    void processSignal(CSocketFsm::Signals signal);
	    CServerAcceptedFsm* getClientBySockfd(int sockfd);
	    void shutdownClient(int sockfd, CSocketFsm::Signals finalSignal = CSocketFsm::sig_shutdown);
	protected:
	    static CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks statesCallbacks_;
	    static CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks getStatesCallbacksT();

	    typedef std::map<int, CServerAcceptedFsm*> ClientsMap;
	    CServerSocketFsm::ClientsMap clients_;

	    static CSocketFsm::StateTable clientStateTable_;
	    static CSocketFsm::StateTable getClientsStateTable();

	    void q_Bind();
	    void q_SetListening();
	    void q_WaitIncomings();
	    void q_ProcessIncomings();
    };
}

#endif /* CSERVERSOCKETFSM_H  */
