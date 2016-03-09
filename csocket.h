#ifndef CSOCKET_H
#define CSOCKET_H

#include "general.h" 
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ydd {

    class CSocket 
    {
	public:
	    enum EpollMode {emNone, emEpollin, emEpollout, emEpollinEpollout};
	private: 
	    int sockfd_;
	    int epollfd_;
	    CSocket::EpollMode epollMode_;
	    bool useEpollet_;
	    std::vector<std::string> dataChunks_;
	    std::string host_;
	    std::string port_;
	    bool isListening_;
	    addrinfo* ai_;

	    int close();
	public:
	    CSocket(const char* host, const char* port, bool isListening, int epollfd, bool useEpollet);
	    ~CSocket();
	    int getAddrinfo();
	    int getIpString(std::string& str);
	    int getSockFd();
	    int makeNonBlocking();
	    void shutdown();
	    int setEpollMode(CSocket::EpollMode mode);
	    int connect(bool& gotEInProgress);
	    int getSoError(int& soError);

    };
}

#endif /* CSOCKET_H */
