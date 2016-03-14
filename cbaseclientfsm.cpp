#include "cbaseclientfsm.h"
#include <stdexcept>

namespace ydd
{
    void CBaseClientFsm::q_ConnectPending()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollout) != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
    }

    void CBaseClientFsm::q_Connect()
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

    void CBaseClientFsm::q_ConnectCheck()
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
