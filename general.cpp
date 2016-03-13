#include "general.h"
#include <cstring>

namespace ydd {
    void _log_errno(const char *file, const char *func, int e)
    {
	char buf[ydd::ERR_BUF_LEN] = "";
	char* str = buf;
#ifdef _GNU_SOURCE
	str = strerror_r(e, buf, ydd::ERR_BUF_LEN);
#endif
	syslog(LOG_ERR, "[%s %s] %s", file, func, str);
    }
}

