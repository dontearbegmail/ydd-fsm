#ifndef GENERAL_H
#define GENERAL_H

#include <errno.h>
#include <syslog.h>
#include <cstddef>


namespace ydd {
    /* at least 128 bytes for SSL!!! */
    static const std::size_t ERR_BUF_LEN = 256;
    void _log_errno(const char *file, const char *func, int e);
}

#define msyslog(prior, fmt, args...) syslog(prior, "[%s %s] " fmt, __FILE__, __func__, ##args) 
#define log_errno(e) ydd::_log_errno(__FILE__, __func__, e);

#endif /* GENERAL_H */
