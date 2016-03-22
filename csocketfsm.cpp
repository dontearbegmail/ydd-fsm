#include "csocketfsm.h"
#include <stdexcept>

namespace ydd 
{
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
	this->state_ = CSocketFsm::q_initial;
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
	this->selfSignal_ = CSocketFsm::sig_empty;
	this->setSelfSignal_ = false;
    }

    CSocketFsm::~CSocketFsm()
    {
	if(this->needDeleteTable_)
	    delete this->table_;
    }

    int CSocketFsm::sockfd()
    {
	return this->socket_.sockfd();
    }

    CSocketFsm::StateType CSocketFsm::getState()
    {
	return this->state_;
    }

    bool CSocketFsm::getNewState(CSocketFsm::Signals signal, CSocketFsm::StateType& newState)
    {
	try
	{
	    newState = this->table_->at(this->state_).at(signal);
	}
	catch(std::out_of_range& oor)
	{
	    msyslog(LOG_ERR, "Got out of range exception while trying to access "
		    "this->table_[curr state = %d][signal = %d]", this->state_, signal);
	    return false;
	}
	return true;
    }

    void CSocketFsm::setSelfSignal(CSocketFsm::Signals signal)
    {
	this->setSelfSignal_ = true;
	this->selfSignal_ = signal;
    }

    void CSocketFsm::q_GetSockFd()
    {
	if(this->socket_.getSockFd() != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
	else
	    this->setSelfSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::q_MakeNonBlocking()
    {
	if(this->socket_.makeNonBlocking() != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
	else
	    this->setSelfSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::doShutdown()
    {
	this->socket_.shutdown();
    }

    void CSocketFsm::q_Shutdown()
    {
	this->doShutdown();
    }

    void CSocketFsm::q_Error()
    {
	msyslog(LOG_ERR, "CSocketFsm ended in q_error state");
	this->doShutdown();
    }

    void CSocketFsm::q_ConnectPending()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollout) != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
    }

    void CSocketFsm::q_Connect()
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

    void CSocketFsm::q_ConnectCheck()
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

    void CSocketFsm::q_ReadEpollinPending()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollin) != 0)
	    this->setSelfSignal(sig_err);
    }

    void CSocketFsm::q_Read()
    {
	CSocket::ReadStates state = this->socket_.read(this->readData);
	if(state == CSocket::rsConnectionClosed)
	    this->setSelfSignal(sig_connection_closed);
	else if(state == CSocket::rsGotEagain)
	    this->setSelfSignal(sig_eagain);
	else
	    this->setSelfSignal(sig_err);
    }


}
