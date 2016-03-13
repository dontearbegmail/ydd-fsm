#include "cserversocketfsm.h"
#include <stdexcept>

namespace ydd
{
    const CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks 
	CServerSocketFsm::statesCallbacks_ = CServerSocketFsm::getStatesCallbacksT();

    CServerSocketFsm::CServerSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable)
	: CSocketFsm(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet, table, copyTable)
    {
	if(table->size() != CServerSocketFsm::NUM_STATES)
	    throw std::invalid_argument("table.size() != CServerSocketFsm::NUM_STATES");
    }

    void CServerSocketFsm::q_Bind()
    {
	if(this->socket_.bind() != 0)
	    this->processSignal(sig_err);
	else
	    this->processSignal(sig_noerr);
    }

    void CServerSocketFsm::q_SetListening()
    {
	if(this->socket_.setListening() != 0)
	    this->processSignal(sig_err);
	else
	    this->processSignal(sig_noerr);
    }

    void CServerSocketFsm::q_WaitIncomings()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollin) != 0)
	    this->processSignal(sig_err);
    }

    void CServerSocketFsm::q_ProcessIncomings()
    {
	msyslog(LOG_DEBUG, "Starting to process incoming connections");
	int infd;
	struct sockaddr in_addr;
	while(1)
	{
	    infd = this->socket_.accept(in_addr);
	    if(infd == 0) {
		msyslog(LOG_DEBUG, "Processed all incoming connections");
		break;
	    }
	    else if(infd == -1) {
		msyslog(LOG_ERR, "Failed to accept an incoming connection. "
			"Will stop processing incoming connections, some of them may be lost. "
			"See log above for details");
		break;
	    }
	    else {
		CClientSocketFsm newclient(&in_addr, true, infd, this->socket_.getEpollFd(), 
			this->socket_.getUseEpollet(), NULL, false);
		newclient.processSignal(sig_empty);
	    }
	}
	/*
    while(1) {
	int infd = accept_and_epoll(sockfd, efd, EPOLLET);
	if(infd == 0) {
	    msyslog(LOG_DEBUG, "Processed all incoming connections");
	    break;
	}
	else if(infd == -1) {
	    msyslog(LOG_ERR, "Failed to accept or epoll-queue an incoming connection. "
		    "Will stop processing incoming connections, some of them may be lost. "
		    "See log above for details");
	    break;
	}
	else {
	    msyslog(LOG_DEBUG, "Successfully accepted an incoming connection on socket %d " 
		    "and added it to epoll queue. See log above for details", infd);
	    int r = sfd_sd_add(sfd_sd, infd, NULL, 0, NULL);
	    if(r != 0) { */
		/* The saddest possible case: we're adding a new incoming connection socket file descriptor to our
		 * SFD-DCL storage, but it's value is already present there. How could it happen? The only explanation
		 * that comes into mind: we added this infd earlier, but the connection was closed, and we didn't
		 * notice that. So we have to clean DCL for that infd */
		/*if(r == 1) {
		    msyslog(LOG_WARNING, "sfd_sd returned 1 when adding a new incoming connection "
			    "with socketfd = %d. Will empty the existing DCL", infd);
		    sfd_sd_empty_sd(sfd_sd, infd);
		}
		else if(r == -1) {
		    msyslog(LOG_WARNING, "SFD-DCL storage limit reached while trying to add an incoming connection "
			    "with socketfd = %d. Will close the connection. TODO: add SFD-DCL storage cleaner", infd);
		    close(infd);*/ /* epoll removes infd automatically from its' watchlist */
		/*}
		else if(r == -2) {*/
		    /* Iput data error?? (i.e. sfd_sd == NULL) This should never happen, but still... */
		    /*msyslog(LOG_WARNING, "Lucky me! sfd_sd_add returned -2 which means input data error. "
			    "Don't know what to do, so I'll continue hoping for the best and I'll pray for you :))");
		}
	    }
	    else {
		msyslog(LOG_DEBUG, "Successfully added the socketfd %d for the incoming connection "
			"to SFD-DCL storage.", infd);
	    }
	}
    }*/
    }
    CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks CServerSocketFsm::getStatesCallbacksT()
    {
	CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks table(
		CServerSocketFsm::NUM_STATES, NULL);
	table[q_shutdown] = &CServerSocketFsm::q_Shutdown;
	table[q_getSockFd] = &CServerSocketFsm::q_GetSockFd;
	table[q_bind] = &CServerSocketFsm::q_Bind;
	table[q_makeNonBlocking] = &CServerSocketFsm::q_MakeNonBlocking;
	table[q_setListening] = &CServerSocketFsm::q_SetListening;
	table[q_waitIncomings] = &CServerSocketFsm::q_WaitIncomings;
	table[q_processIncomings] = &CServerSocketFsm::q_ProcessIncomings;

	return table;
    }

    /*CSocketFsm::StatesCallbacks CServerSocketFsm::getStatesCallbacksT()
    {
	CSocketFsm::StatesCallbacks t(CServerSocketFsm::NUM_STATES, NULL);
	t[q_makeNonBlocking] = &CServerSocketFsm::q_MakeNonBlocking;
	//t[q_bind] = &CServerSocketFsm::q_Bind;
	return t;
    }*/
}
