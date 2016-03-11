#include "csocket.h"
#include "general.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdexcept>
#include <cstring>

using std::string;

namespace ydd 
{
    CSocket::CSocket(struct sockaddr* ai_addr, bool copyAiAddr, int sockfd, int epollfd, bool useEpollet)
    {
	if(ai_addr == NULL)
	    throw std::invalid_argument("ai_addr is NULL");
	this->sockfd_ = sockfd;
	this->epollfd_ = epollfd;
	this->epollMode_ = CSocket::emNone;
	this->useEpollet_ = useEpollet;
	if(copyAiAddr)
	{
	    std::memcpy(this->ai_addr_, ai_addr, sizeof(struct sockaddr));
	    this->needDeleteAiAddr_ = true;
	}
	else
	{
	    this->ai_addr_ = ai_addr;
	    this->needDeleteAiAddr_ = false;
	}
    }

    CSocket::~CSocket()
    {
	this->shutdown();
    }

    addrinfo* CSocket::getAddrinfo(const char* host, const char* port)
    {
	if(host == NULL)
	    throw std::invalid_argument("host is NULL");
	if(port == NULL)
	    throw std::invalid_argument("port is NULL");
	struct addrinfo hints = {}, *result = NULL;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int s = getaddrinfo(host, port, &hints, &result);
	if (s != 0) {
	    msyslog(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(s));
	    result = NULL;
	}

	return result;
    }

    int CSocket::getHostPortStrings(std::string& host, std::string& port)
    {
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	socklen_t ai_len = sizeof(this->ai_addr_);
	int e = getnameinfo(this->ai_addr_, ai_len, hbuf, NI_MAXHOST, sbuf, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	if(e != 0)
	    return -1;
	host.assign(hbuf);
	port.assign(sbuf);
	return 0;
    }

    int CSocket::getSockFd()
    {
	if(this->sockfd_ != -1)
	    throw std::logic_error("Trying to getSockFd, but this->sockfd_ != -1");
	int e;
	this->sockfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(this->sockfd_ == -1) {
	    e = errno;
	    log_errno(e);
	    return -1;
	}
	return 0;
    }

    int CSocket::makeNonBlocking()
    {
	int flags, s;

	flags = fcntl(this->sockfd_, F_GETFL, 0);
	if (flags == -1) {
	    msyslog(LOG_ERR, "make_socket_non_blocking: couldn't get flags for socket");
	    return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(this->sockfd_, F_SETFL, flags);
	if (s == -1) {
	    msyslog(LOG_ERR, "make_socket_non_blocking: failed to set O_NONBLOCK");
	    return -1;
	}

	return 0;
    }

    int CSocket::close()
    {
	int retval = -1;

	if(this->sockfd_ != -1)
	    retval = ::close(this->sockfd_);

	if(retval == -1) {
	    int e = errno;
	    log_errno(e);
	}
	return retval;    
    }

    void CSocket::shutdown()
    {
	if(this->needDeleteAiAddr_)
	    delete this->ai_addr_;
	if(this->sockfd_ != -1)
	    this->close();
    }

    int CSocket::setEpollMode(CSocket::EpollMode mode)
    {
	if(mode == this->epollMode_)
	    return 0;

	int e;
	bool gotErr = false;
	struct epoll_event event = {0};
	event.data.fd = this->sockfd_;

	if(mode == CSocket::emNone)
	{
	    e = epoll_ctl(this->epollfd_, EPOLL_CTL_DEL, this->sockfd_, &event);
	    if(e == -1)
	    {
		e = errno;
		gotErr = true;
		log_errno(e);
	    }
	}
	else 
	{
	    switch(mode)
	    {
		case CSocket::emEpollin:
		    event.events = EPOLLIN;
		    break;

		case CSocket::emEpollout:
		    event.events = EPOLLOUT;
		    break;

		case CSocket::emEpollinEpollout:
		    event.events = EPOLLIN | EPOLLOUT;
		    break;

		default:
		    break;
	    }
	    if(this->useEpollet_)
		event.events |= EPOLLET;
	    int op = this->epollMode_ == CSocket::emNone ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
	    e = epoll_ctl(this->epollfd_, op, this->sockfd_, &event);
	    if(e == -1)
	    {
		e = errno;
		gotErr = true;
		log_errno(e);
	    }
	}

	if(gotErr)
	    this->epollMode_ = CSocket::emNone;
	else
	    this->epollMode_ = mode;

	return gotErr ? -1 : 0;
    }

    int CSocket::connect(bool& gotEInProgress)
    {
	bool gotError = false;
	int e;
	gotEInProgress = false;
	e = ::connect(this->sockfd_, this->ai_addr_, sizeof(this->ai_addr_));
	if(e == -1) 
	{
	    e = errno;
	    if(e == EINPROGRESS)
	    {
		gotEInProgress = true;
	    }
	    else 
	    {
		gotError = true;
		log_errno(e);
	    }
	}

	return gotError ? -1 : 0;
    }

    int CSocket::getSoError(int& soError)
    {
	bool gotErr = false;
	int optval, e;
	size_t optlen = sizeof(optval);
	e = getsockopt(this->sockfd_, SOL_SOCKET, SO_ERROR, &optval, &optlen);
	if(e == -1)
	{
	    e = errno;
	    log_errno(e);
	    gotErr = true;
	}
	else
	{
	    soError = optval;
	}
	
	return gotErr ? -1 : 0;
    }

    int CSocket::accept(struct sockaddr& in_addr)
    {
	socklen_t in_len;
	int infd;
	int e;

	in_len = sizeof(in_addr);
	infd = ::accept(this->sockfd_, &in_addr, &in_len);
	if (infd == -1) {
	    e = errno;
	    if ((e == EAGAIN) || (e == EWOULDBLOCK)) {
		/* We have processed all incoming connections. */
		return 0;
	    }
	    else {
		log_errno(e);
		return -1;
	    }
	}
	return infd;
    }
}
