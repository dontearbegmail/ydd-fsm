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
	    bool needDeleteAiAddr_;

	    int close();
	public:
	    struct sockaddr* ai_addr_;
	    CSocket(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, int epollfd, bool useEpollet);
	    ~CSocket();
	    static int getAddrinfo(const char* host, const char* port, struct sockaddr& ai_addr);
	    static int getIpString(struct sockaddr& ai_addr, std::string& s);
	    int getHostPortStrings(std::string& host, std::string& port);
	    int getSockFd();
	    int makeNonBlocking();
	    void shutdown();
	    int setEpollMode(CSocket::EpollMode mode);
	    int connect(bool& gotEInProgress);
	    int getSoError(int& soError);
	    int bind();
	    int setListening();
	    int accept(struct sockaddr& in_addr);
	    int getEpollFd();
	    bool getUseEpollet();
    };
}

#endif /* CSOCKET_H */
