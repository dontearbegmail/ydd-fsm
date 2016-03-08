#include "csocketfsm.h"

namespace ydd 
{

    CSocketFsm::CSocketFsm(char const* host, char const * port, bool isListening) : socket_(host, port, isListening)
    {
	this->state_ = CSocketFsm::q_none;
    }

    void CSocketFsm::processSignal(CSocketFsm::Signals signal)
    {
    }

    void CSocketFsm::q_Resolve()
    {
	if(socket_.getAddrinfo() != 0)
	    this->processSignal(CSocketFsm::sig_err);
	else 
	    this->processSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::q_GetSockFd()
    {
	if(socket_.getSockFd() != 0)
	    this->processSignal(CSocketFsm::sig_err);
	else
	    this->processSignal(CSocketFsm::sig_noerr);
    }
}