#ifndef CBASECLIENTFSM_H
#define CBASECLIENTFSM_H

#include "csocketfsm.h"

namespace ydd
{
    class CBaseClientFsm : public CSocketFsm
    {
	public:
	    static const size_t NUM_STATES = 0;

	    void q_ConnectPending();
	    void q_Connect();
	    void q_ConnectCheck();
    };
}

#endif /* CBASECLIENTFSM_H  */
