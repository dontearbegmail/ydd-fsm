#include "cserversocketfsm.h"
#include <stdexcept>

namespace ydd
{
    CSocketFsm::TFSMHelper<CServerSocketFsm>::StatesCallbacks 
	CServerSocketFsm::statesCallbacks_ = CServerSocketFsm::getStatesCallbacksT();
    CSocketFsm::StateTable CServerSocketFsm::clientStateTable_ = CServerSocketFsm::getClientsStateTable();

    CServerSocketFsm::CServerSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable)
	: CSocketFsm(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet, table, copyTable)
    {
	if(table->size() != CServerSocketFsm::NUM_STATES)
	    throw std::invalid_argument("table.size() != CServerSocketFsm::NUM_STATES");
    }

    CServerSocketFsm::~CServerSocketFsm()
    {
	for(ClientsMap::iterator it = this->clients_.begin(); it != this->clients_.end(); it++)
	{
	    if(it->second != NULL)
	    {
		it->second->processSignal(CSocketFsm::sig_shutdown);
		delete it->second;
		it->second = NULL;
	    }
	}
    }

    void CServerSocketFsm::processSignal(CSocketFsm::Signals signal)
    {
	this->processSignalT<CServerSocketFsm>(this, CServerSocketFsm::statesCallbacks_, signal);
    }

    void CServerSocketFsm::q_Bind()
    {
	if(this->socket_.bind() != 0)
	    this->setSelfSignal(sig_err);
	else
	    this->setSelfSignal(sig_noerr);
    }

    void CServerSocketFsm::q_SetListening()
    {
	if(this->socket_.setListening() != 0)
	    this->setSelfSignal(sig_err);
	else
	    this->setSelfSignal(sig_noerr);
    }

    void CServerSocketFsm::q_WaitIncomings()
    {
	if(this->socket_.setEpollMode(CSocket::emEpollin) != 0)
	    this->setSelfSignal(sig_err);
    }

    void CServerSocketFsm::q_ProcessIncomings()
    {
	msyslog(LOG_DEBUG, "Starting to process incoming connections");
	int infd;
	struct sockaddr in_addr;
	bool gotMore = true;
	bool noErrors = true;
	while(gotMore && noErrors)
	{
	    infd = this->socket_.accept(in_addr);
	    if(infd == 0) {
		msyslog(LOG_DEBUG, "Processed all incoming connections");
		gotMore = false;
	    }
	    else if(infd == -1) {
		msyslog(LOG_ERR, "Got an error while processing an incoming connection. "
			"Will stop processing incoming connections, some of them may be lost. "
			"See log above for details");
		noErrors = false;
	    }
	    else {
		CServerAcceptedFsm* newclient = new CServerAcceptedFsm(&in_addr, true, infd, 
			this->socket_.getEpollFd(), this->socket_.getUseEpollet(), 
			&CServerSocketFsm::clientStateTable_, false);
		std::pair<ClientsMap::iterator, bool> ret;
		ret = this->clients_.insert(std::pair<int, CServerAcceptedFsm*>(infd, newclient));
		if(ret.second == false)
		{
		    msyslog(LOG_DEBUG, "A newly accepted connection sockfd = %d is already present "
			    "in this->clients_. Will delete existing CServerAcceptedFsm data for it "
			    "and create new one", infd);
		    if(ret.first->second != NULL)
			delete ret.first->second;
		    ret.first->second = newclient;
		}
		newclient->processSignal(sig_empty);
		msyslog(LOG_DEBUG, "Successfully added incoming sockfd %d into this->clients_ "
			"storage", infd);
	    }
	}
	if(!noErrors)
	    this->setSelfSignal(sig_err);
	else
	    this->setSelfSignal(sig_noerr);
    }

    CServerAcceptedFsm* CServerSocketFsm::getClientBySockfd(int sockfd)
    {
	CServerAcceptedFsm* client = NULL;
	try
	{
	    client = clients_.at(sockfd);
	}
	catch(std::out_of_range& oor)
	{
	    msyslog(LOG_ERR, "Requested client socket %d which is not present in "
		    "CServerSocketFsm::clients_ storage", sockfd);
	}
	return client;
    }

    void CServerSocketFsm::shutdownClient(int sockfd, CSocketFsm::Signals finalSignal)
    {
	CServerSocketFsm::ClientsMap::iterator it = this->clients_.find(sockfd);
	if(it == this->clients_.end())
	{
	    msyslog(LOG_ERR, "Trying to shutdown client sockfd = %d which isn't "
		    "present in this->clients_", sockfd);
	    return;
	}
	if(it->second != NULL)
	{
	    it->second->processSignal(finalSignal);
	    delete it->second;
	    it->second = NULL;
	}
	this->clients_.erase(it);
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

    CSocketFsm::StateTable CServerSocketFsm::getClientsStateTable()
    {
	CSocketFsm::StateLine e(CSocketFsm::NUM_SIGNALS, CServerAcceptedFsm::q_shutdown);
	CSocketFsm::StateTable t(CServerAcceptedFsm::NUM_STATES, e);

	for(CSocketFsm::StateType i = CServerAcceptedFsm::q_initial; i < CServerAcceptedFsm::q_shutdown; i++)
	    t[i][sig_err] = CServerAcceptedFsm::q_error;

	t[CServerAcceptedFsm::q_initial][CSocketFsm::sig_empty] = CServerAcceptedFsm::q_makeNonBlocking;

	t[CServerAcceptedFsm::q_makeNonBlocking][CSocketFsm::sig_noerr] = CServerAcceptedFsm::q_readEpollinPending;

	t[CServerAcceptedFsm::q_readEpollinPending][CSocketFsm::sig_epollin] = CServerAcceptedFsm::q_read;

	t[CServerAcceptedFsm::q_read][CSocketFsm::sig_eagain] = CServerAcceptedFsm::q_readEpollinPending;
	t[CServerAcceptedFsm::q_read][CSocketFsm::sig_noerr] = CServerAcceptedFsm::q_checkClientData;
	t[CServerAcceptedFsm::q_read][CSocketFsm::sig_connection_closed] = CServerAcceptedFsm::q_connectionClosed;

	t[CServerAcceptedFsm::q_checkClientData][CSocketFsm::sig_noerr] = CServerAcceptedFsm::q_write;

	t[CServerAcceptedFsm::q_write][CSocketFsm::sig_eagain] = CServerAcceptedFsm::q_writeEpolloutPending;
	t[CServerAcceptedFsm::q_write][CSocketFsm::sig_noerr] = CServerAcceptedFsm::q_shutdown;

	t[CServerAcceptedFsm::q_writeEpolloutPending][CSocketFsm::sig_epollout] = CServerAcceptedFsm::q_write;

	return t;
    }
}
