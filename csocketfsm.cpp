#include "csocketfsm.h"

namespace ydd 
{

    CSocketFsm::CSocketFsm(char const* host, char const * port, bool isListening, 
	    int epollfd, StateTable* table, bool copyTable) 
	: socket_(host, port, isListening, epollfd)
    {
	if(table == NULL)
	    throw std::exception();
	this->state_ = CSocketFsm::q_none;
	if(copyTable)
	{
	    this->needDeleteTable_ = true;
	    this->table_ = new CSocketFsm::StateTable(*table);
	}
	else 
	{
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
}
