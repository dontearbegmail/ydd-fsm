#include "general.h"

int main(int argc, char *argv[])
{

    openlog("ydd-client", LOG_PID, LOG_USER);
    msyslog(LOG_INFO, "Test");
    return 0;
}
