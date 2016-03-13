#ifndef CSOCKETFSM_H
#define CSOCKETFSM_H

#include "general.h"
#include "csocket.h"
#include <vector>
#include <stdexcept>

namespace ydd 
{

    class CSocketFsm 
    {
	public:
	    typedef short StateType;
	    static const StateType q_none = 0, q_shutdown = 1;
	    static const size_t NUM_STATES = 0;
	    
	    template<class TFSM> struct TFSMHelper
	    {
		typedef void (TFSM::*StateCallback)();
		typedef std::vector<StateCallback> StatesCallbacks;

		static StateCallback getCallback(StatesCallbacks& table, CSocketFsm::StateType state)
		{
		    StateCallback callback = NULL;
		    try 
		    {
			callback = table.at(state);
		    }
		    catch(std::out_of_range& oor)
		    {
			callback = NULL;
			msyslog(LOG_ERR, "The state #%d is out of range of this->statesCallbacks_ (size = %d)", 
				state, table.size());
		    }
		    return callback;
		}
	    };

	    enum Signals
	    {
		sig_noerr,
		sig_err,
		sig_einprogress,
		sig_epollin,
		sig_epollout,
		sig_empty
	    };
	    const static size_t NUM_SIGNALS = 6;

	    typedef std::vector<StateType> StateLine;
	    typedef std::vector<CSocketFsm::StateLine> StateTable;

	    CSocket socket_;
	protected:
	    CSocketFsm::StateTable* table_;
	    bool needDeleteTable_;

	    StateType state_;

	    CSocketFsm::Signals selfSignal_;
	    bool setSelfSignal_;
	    void setSelfSignal(CSocketFsm::Signals signal);
	    
	    //typedef void (CSocketFsm::*StateCallback)();
	    //typedef std::vector<StateCallback> StatesCallbacks;
	    //CSocketFsm::StatesCallbacks* statesCallbacks_;
	    //CSocketFsm::StateCallback getCallback(CSocketFsm::StateType state);
	    bool getNewState(CSocketFsm::Signals signal, CSocketFsm::StateType& newState);

	    void q_GetSockFd();
	    void q_MakeNonBlocking();
	    void q_Shutdown();

	public:
	    CSocketFsm(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, 
		    int epollfd, bool useEpollet, StateTable* table, bool copyTable);
	    ~CSocketFsm();
	    StateType getState();
	    //void processSignal(CSocketFsm::Signals signal);
	    template<class TFSM> void processSignalT(
		    TFSM* tfsm,
		    typename CSocketFsm::TFSMHelper<TFSM>::StatesCallbacks& statesCallbacks,
		    CSocketFsm::Signals signal)
	    {
		CSocketFsm::StateType newState;
		typename CSocketFsm::TFSMHelper<TFSM>::StateCallback scb = NULL;
		bool newOk; 
		CSocketFsm::Signals sig = signal;

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
		    scb = CSocketFsm::TFSMHelper<TFSM>::getCallback(statesCallbacks, newState);
		    if(scb == NULL)
			msyslog(LOG_WARNING, "Got NULL as the callback function for the new state %d", newState);
		    else
			(tfsm->*scb)(); // can change this->setSelfSignal_ to true
		    if(this->setSelfSignal_)
			sig = this->selfSignal_;
		} while(this->setSelfSignal_);
	    }
    };
}

#endif /* CSOCKETFSM_H */
