#include "cserveracceptedfsm.h"
#include <stdexcept>

namespace ydd
{
    CServerAcceptedFsm::CServerAcceptedFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable)
	: CSocketFsm(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet, table, copyTable)
    {
	if(table->size() != CServerAcceptedFsm::NUM_STATES)
	    throw std::invalid_argument("table.size() != CServerAcceptedFsm::NUM_STATES");
    }

    void CServerAcceptedFsm::q_ConnectPending()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollout) != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
    }

    void CServerAcceptedFsm::q_Connect()
    {
	bool gotEInProgress = false;
	if(this->socket_.connect(gotEInProgress) == -1)
	{
	    this->setSelfSignal(CSocketFsm::sig_err);
	}
	else
	{
	    if(gotEInProgress)
		this->setSelfSignal(CSocketFsm::sig_einprogress);
	    else
		this->setSelfSignal(CSocketFsm::sig_noerr);
	}
    }

    void CServerAcceptedFsm::q_ConnectCheck()
    {
	int soError;
	if(this->socket_.getSoError(soError) == -1)
	{
	    this->setSelfSignal(CSocketFsm::sig_err);
	}
	else
	{
	    if(soError == 0)
		this->setSelfSignal(CSocketFsm::sig_noerr);
	    else
		this->setSelfSignal(CSocketFsm::sig_err);
	}
    }

}
