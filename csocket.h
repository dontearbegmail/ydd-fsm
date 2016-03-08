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
	private: 
	    int sockfd_;
	    std::vector<std::string> dataChunks_;
	    std::string host_;
	    std::string port_;
	    bool isListening_;
	    addrinfo* ai_;

	    int close();
	public:
	    CSocket(const char* host, const char* port, bool isListening);
	    ~CSocket();
	    int getAddrinfo();
	    int getIpString(std::string& str);
	    int getSockFd();
	    int makeNonBlocking();
	    void shutdown();
    };
}

#endif /* CSOCKET_H */
