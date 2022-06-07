#include <stdio.h>
#include "ntp.h"
#include <sys/time.h>

int main()
{
    struct timeval tv;

    tv = ntp_request("ntp.aliyun.com");
    printf("ntp timestamp = %ld.%ld\r\n", tv.tv_sec, tv.tv_usec);

    ntp_server_task();

    return 0;
}
