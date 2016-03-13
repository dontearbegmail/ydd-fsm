#include "csocketfsm.h"
#include <stdexcept>

namespace ydd 
{
    /*const CSocketFsm::StateType CSocketFsm::q_none = 0;
    const CSocketFsm::StateType CSocketFsm::q_shutdown = 1;*/

    CSocketFsm::CSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd,
	    int epollfd, bool useEpollet, StateTable* table, bool copyTable) 
	: socket_(ai_addr, copyAiAddr, sockfd, epollfd, useEpollet)
    {
	if(table == NULL)
	    throw std::invalid_argument("table is NULL");
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
	this->selfSignal_ = CSocketFsm::sig_empty;
	this->setSelfSignal_ = false;
	//this->statesCallbacks_ = NULL;
    }

    CSocketFsm::~CSocketFsm()
    {
	if(this->needDeleteTable_)
	    delete this->table_;
    }

    /*CSocketFsm::StateCallback CSocketFsm::getCallback(CSocketFsm::StateType state)
    {
	CSocketFsm::StateCallback sig_callback = NULL;
	if(this->statesCallbacks_ == NULL)
	    throw std::logic_error("statesCallbacks_ table is NULL");
	try 
	{
	    sig_callback = this->statesCallbacks_->at(state);
	}
	catch(std::out_of_range& oor)
	{
	    sig_callback = NULL;
	    msyslog(LOG_ERR, "The state #%d is out of range of this->statesCallbacks_ (size = %d)", 
		    state, this->statesCallbacks_->size());
	}
	return sig_callback;
    }*/
	    
    bool CSocketFsm::getNewState(CSocketFsm::Signals signal, CSocketFsm::StateType& newState)
    {
	try
	{
	    newState = this->table_->at(this->state_).at(signal);
	}
	catch(std::out_of_range& oor)
	{
	    msyslog(LOG_ERR, "Got out of range exception while trying to access "
		    "this->table_[curr state = %d][signal = %d]", this->state_, signal);
	    return false;
	}
	return true;
    }

    void CSocketFsm::processSignal(CSocketFsm::Signals signal)
    {
	CSocketFsm::StateType newState;
	//CSocketFsm::StateCallback scb = NULL;
	bool newOk; 
	CSocketFsm::Signals sig = signal;

	// do input var "signal" processing here
	do
	{
	    this->setSelfSignal_ = false;
	    newOk = this->getNewState(sig, newState);
	    if(!newOk)
	    {
		msyslog(LOG_ERR, "Failed to get the new state. See log above. The state remains unchanged.");
		return;
	    }
	    this->state_ = newState;
	    /*scb = this->getCallback(newState);
	    if(scb == NULL)
		msyslog(LOG_WARNING, "Got NULL as the callback function for the new state %d", newState);
	    else
		;
		//(*scb)(); // can change this->setSelfSignal_ to true*/
	    if(this->setSelfSignal_)
		sig = this->selfSignal_;
	} while(this->setSelfSignal_);
    }

    void CSocketFsm::setSelfSignal(CSocketFsm::Signals signal)
    {
	this->setSelfSignal_ = true;
	this->selfSignal_ = signal;
    }

    void CSocketFsm::q_GetSockFd()
    {
	if(this->socket_.getSockFd() != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
	else
	    this->setSelfSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::q_MakeNonBlocking()
    {
	if(this->socket_.makeNonBlocking() != 0)
	    this->setSelfSignal(CSocketFsm::sig_err);
	else
	    this->setSelfSignal(CSocketFsm::sig_noerr);
    }

    void CSocketFsm::q_Shutdown()
    {
	this->socket_.shutdown();
    }

}
