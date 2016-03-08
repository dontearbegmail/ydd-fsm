#include "csocket.h"
#include "general.h"
#include <arpa/inet.h>

using std::string;

namespace ydd 
{
    CSocket::CSocket(const char* host, const char* port, bool isListening)
    {
	this->sockfd_ = -1;
	this->ai_ = NULL;
	this->host_ = string(host);
	this->port_ = string(port);
	this->isListening_ = isListening;
    }

    CSocket::~CSocket()
    {
	if(this->ai_ != NULL)
	    freeaddrinfo(this->ai_);
    }

    int CSocket::getAddrinfo()
    {
	struct addrinfo hints = {}, *result;
	int retval = -1;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if(this->isListening_)
	    hints.ai_flags = AI_PASSIVE;

	int s = getaddrinfo(this->host_.c_str(), this->port_.c_str(), &hints, &result);
	if (s != 0) {
	    msyslog(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(s));
	}
	else {
	    this->ai_ = result;
	    retval = 0;
	}

	return retval;
    }

    int CSocket::getIpString(std::string& str)
    {
	char buf[INET_ADDRSTRLEN];
	sockaddr_in *sai = (sockaddr_in*) this->ai_->ai_addr;
	inet_ntop(AF_INET, &(sai->sin_addr), buf, INET_ADDRSTRLEN);
	str = buf;
	return 0;
    }

    int CSocket::getSockFd()
    {
	int e;
	this->sockfd_ = socket(this->ai_->ai_family, this->ai_->ai_socktype, this->ai_->ai_protocol);
	if(this->sockfd_ == -1) {
	    e = errno;
	    log_errno(e);
	    return -1;
	}
	return 0;
    }
}
