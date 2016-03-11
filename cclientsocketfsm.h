#ifndef CCLIENTSOCKETFSM_H
#define CCLIENTSOCKETFSM_H

#include "csocketfsm.h"
#include <vector>
#include <string>

namespace ydd
{
    class CClientSocketFsm : public CSocketFsm
    {
	private:
	    std::vector<std::string> data_;
	public:
	    enum States : StateType
	    {
		q_none = 0,
		q_getSockFd,
		q_makeNonBlocking,
		q_connect,
		q_connectPending,
		q_connectCheck,
		q_read,
		q_readEpollinPending,
		q_write,
		q_writeEpolloutPending,
		q_sslWrite,
		q_sslWriteWantRead,
		q_sslWriteWantWrite,
		q_sslRead,
		q_sslReadWantWrite,
		q_sslReadWantRead,
		q_shutdown
	    };
	    static const size_t NUM_STATES = 17;

	    CClientSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
    };
}

#endif /* CCLIENTSOCKETFSM_H  */
