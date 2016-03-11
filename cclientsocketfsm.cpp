#include "cclientsocketfsm.h"
#include <stdexcept>

namespace ydd
{
    CClientSocketFsm::CClientSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable)
	: CSocketFsm(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet, table, copyTable)
    {
	if(table->size() != CClientSocketFsm::NUM_STATES)
	    throw std::invalid_argument("table.size() != CClientSocketFsm::NUM_STATES");
    }
}
