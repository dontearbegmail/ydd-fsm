#include "cserveracceptedfsm.h"
#include <stdexcept>

namespace ydd
{

    CSocketFsm::TFSMHelper<CServerAcceptedFsm>::StatesCallbacks 
	CServerAcceptedFsm::statesCallbacks_ = CServerAcceptedFsm::getStatesCallbacksT();

    CServerAcceptedFsm::CServerAcceptedFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable)
	: CSocketFsm(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet, table, copyTable)
    {
	if(table->size() != CServerAcceptedFsm::NUM_STATES)
	    throw std::invalid_argument("table.size() != CServerAcceptedFsm::NUM_STATES");
    }

    CSocketFsm::TFSMHelper<CServerAcceptedFsm>::StatesCallbacks CServerAcceptedFsm::getStatesCallbacksT()
    {
	CSocketFsm::TFSMHelper<CServerAcceptedFsm>::StatesCallbacks table(
		CServerAcceptedFsm::NUM_STATES, NULL);
	table[q_shutdown] = &CServerAcceptedFsm::q_Shutdown;
	table[q_makeNonBlocking] = &CServerAcceptedFsm::q_MakeNonBlocking;
	table[q_readEpollinPending] = &CServerAcceptedFsm::q_ReadEpollinPending;
	table[q_read] = &CServerAcceptedFsm::q_Read;

	return table;
    }

    void CServerAcceptedFsm::processSignal(CSocketFsm::Signals signal)
    {
	this->processSignalT<CServerAcceptedFsm>(this, CServerAcceptedFsm::statesCallbacks_, signal);
    }
}
