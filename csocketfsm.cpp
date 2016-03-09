#include "csocketfsm.h"
#include <stdexcept>

namespace ydd 
{

    CSocketFsm::CSocketFsm(char const* host, char const * port, bool isListening, 
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable) 
	: socket_(host, port, isListening, epollfd, useEpollet)
    {
	if(table == NULL)
	    throw std::invalid_argument("table is NULL");
	if(table->size() != CSocketFsm::NUM_STATES)
	    throw std::invalid_argument("table.size() != CSocketFsm::NUM_STATES");
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

    void CSocketFsm::q_Resolve()
    {
	if(this->socket_.getAddrinfo() != 0)
	    this->processSignal(CSocketFsm::sig_err);
	else 
	    this->processSignal(CSocketFsm::sig_noerr);
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
	    this->processSignal(CSocketFsm::sig_err);
	else
	{
	    if(soError == 0)
		this->processSignal(CSocketFsm::sig_noerr);
	    else
		this->processSignal(CSocketFsm::sig_err);
	}
    }
}
