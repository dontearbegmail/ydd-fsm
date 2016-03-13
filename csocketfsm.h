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
			callback = table->at(state);
		    }
		    catch(std::out_of_range& oor)
		    {
			callback = NULL;
			msyslog(LOG_ERR, "The state #%d is out of range of this->statesCallbacks_ (size = %d)", 
				state, table->size());
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


	protected:
	    CSocket socket_;
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
	    void processSignal(CSocketFsm::Signals signal);
	    //template<class TFSM> void processSignalT(CSocketFsm::TFSMHelper<TFSM>::StatesCallbacks& );
    };
}

#endif /* CSOCKETFSM_H */
