#include "general.h"
#include <cstring>

namespace ydd {
    void _log_errno(const char *file, const char *func, int e)
    {
	char buf[ydd::ERR_BUF_LEN];
	strerror_r(e, buf, ydd::ERR_BUF_LEN);
	syslog(LOG_ERR, "[%s %s] %s", file, func, buf);
    }
}

