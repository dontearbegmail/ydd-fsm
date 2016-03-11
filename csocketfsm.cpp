#include "csocketfsm.h"
#include <stdexcept>

namespace ydd 
{
    /*const CSocketFsm::StateType CSocketFsm::q_none = 0;
    const CSocketFsm::StateType CSocketFsm::q_shutdown = 1;*/

    CSocketFsm::CSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable) 
	: socket_(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet)
    {
	if(table == NULL)
	    throw std::invalid_argument("table is NULL");
	CSocketFsm::StateTable::const_iterator i;
	bool isSizeCorrect = true;
	for(i = table->begin(); (i != table->end()) && (isSizeCorrect); i++)
	    isSizeCorrect = i->size() == CSocketFsm::NUM_SIGNALS;
	if(!isSizeCorrect)
	    throw std::invalid_argument("One of table::StateLine size != CSocketFsm::NUM_SIGNALS");
	this->state_ = CSocketFsm::q_none;
	if(copyTable)
	{
	    this->needDeleteTable_ = true;
	    this->table_ = new CSocketFsm::StateTable(*table);
	}
	else 
	{
	    this->needDeleteTable_ = false;
	    this->table_ = table;
	}
    }

    CSocketFsm::~CSocketFsm()
    {
	if(this->needDeleteTable_)
	    delete this->table_;
    }

    void CSocketFsm::processSignal(CSocketFsm::Signals signal)
    {
    }

    void CSocketFsm::q_GetSockFd()
    {
	if(this->socket_.getSockFd() != 0)
	    this->processSignal(CSocketFsm::sig_err);
	else
	    this->processSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::q_MakeNonBlocking()
    {
	if(this->socket_.makeNonBlocking() != 0)
	    this->processSignal(CSocketFsm::sig_err);
	else
	    this->processSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::q_Shutdown()
    {
	this->socket_.shutdown();
    }

    void CSocketFsm::q_ConnectPending()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollout) != 0)
	    this->processSignal(CSocketFsm::sig_err);
    }

    void CSocketFsm::q_Connect()
    {
	bool gotEInProgress = false;
	if(this->socket_.connect(gotEInProgress) == -1)
	{
	    this->processSignal(CSocketFsm::sig_err);
	}
	else
	{
	    if(gotEInProgress)
		this->processSignal(CSocketFsm::sig_einprogress);
	    else
		this->processSignal(CSocketFsm::sig_noerr);
	}
    }

    void CSocketFsm::q_ConnectCheck()
    {
	int soError;
	if(this->socket_.getSoError(soError) == -1)
	{
	    this->processSignal(CSocketFsm::sig_err);
	}
	else
	{
	    if(soError == 0)
		this->processSignal(CSocketFsm::sig_noerr);
	    else
		this->processSignal(CSocketFsm::sig_err);
	}
    }
}
