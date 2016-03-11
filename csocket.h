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
	    struct sockaddr* ai_addr_;
	    bool needDeleteAiAddr_;

	    int close();
	public:
	    CSocket(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, int epollfd, bool useEpollet);
	    ~CSocket();
	    static addrinfo* getAddrinfo(const char* host, const char* port);
	    int getHostPortStrings(std::string& host, std::string& port);
	    int getSockFd();
	    int makeNonBlocking();
	    void shutdown();
	    int setEpollMode(CSocket::EpollMode mode);
	    int connect(bool& gotEInProgress);
	    int getSoError(int& soError);
	    int accept(struct sockaddr& in_addr);
    };
}

#endif /* CSOCKET_H */
