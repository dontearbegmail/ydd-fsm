#ifndef CSERVERACCEPTEDFSM_H
#define CSERVERACCEPTEDFSM_H

#include "csocketfsm.h"
#include <vector>
#include <string>

namespace ydd
{
    class CServerAcceptedFsm : public CSocketFsm
    {
	private:
	    std::vector<std::string> data_;
	public:
	    enum States : StateType
	    {
		q_none = 0,
		q_getSockFd,
		q_read,
		q_readEpollinPending,
		q_checkClientData,
		q_write,
		q_writeEpolloutPending,
		q_shutdown
	    };
	    static const size_t NUM_STATES = 8;

	    CServerAcceptedFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
    };
}

#endif /* CSERVERACCEPTEDFSM_H  */
