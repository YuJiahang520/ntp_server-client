#include "ntp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

static NTP_PACK net_pack;
static TIMESTAMP_FORMAt_64 originate_timestamp;
static TIMESTAMP_FORMAt_64 receive_timestamp;
static uint8_t ntp_version;

// NTP组包
static void ntp_server_pack_encode()
{
    struct timeval tv;

    memset(&net_pack, 0, NTP_PAYLOAD_LEN);
    
    net_pack.head.VN = ntp_version;
    net_pack.head.Mode = MODE_SERVER;

    net_pack.originate_timestamp = originate_timestamp;
    net_pack.receive_timestamp = receive_timestamp;

    gettimeofday(&tv, NULL);
    net_pack.transmit_timestamp.intpart = htonl(tv.tv_sec + JAN_1970);
    net_pack.transmit_timestamp.fracpart = htonl(FRACPART_ENCODE(tv.tv_usec));

    net_pack.Reference_Timestamp.intpart = htonl(tv.tv_sec + JAN_1970 - 60);
    net_pack.Reference_Timestamp.fracpart = net_pack.transmit_timestamp.fracpart;
}

void ntp_server_task()
{
    int sockfd;
    int ret;
    uint8_t buf[100];
    struct sockaddr_in local_addr, peer_addr;
    socklen_t peerlen;
    struct timeval tv;
    struct tm tm_now;

    peerlen = (socklen_t)sizeof(struct sockaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) 
    {
        printf("fail to creat socket.\r\n");
        exit(EXIT_FAILURE);
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET; 
    local_addr.sin_port = htons(NTP_PORT);
    local_addr.sin_addr.s_addr = inet_addr(NTP_SERVER_HOST);

    ret = bind(sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr));
    if (ret == -1) 
    {
        close(sockfd);
        printf("Fail to bind information.\r\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // 阻塞接收消息 并存储对端地址
        ret = recvfrom(sockfd, buf, 100, 0, (struct sockaddr *)&peer_addr, &peerlen);
        gettimeofday(&tv, NULL);
        if (ret > 0)
        {
            if (ret != NTP_PAYLOAD_LEN)
                continue;
            
            memcpy(&net_pack, buf, NTP_PAYLOAD_LEN);

            if (net_pack.head.VN != 2 && net_pack.head.VN != 3 && net_pack.head.VN != 4)
                continue;
            
            if (net_pack.head.Mode != MODE_CLIENT)
                continue;
            
            receive_timestamp.intpart = htonl(tv.tv_sec + JAN_1970);
            receive_timestamp.fracpart = htonl(FRACPART_ENCODE(tv.tv_usec));

            ntp_version = net_pack.head.VN;
            originate_timestamp = net_pack.transmit_timestamp;

            ntp_server_pack_encode();
            sendto(sockfd, &net_pack, NTP_PAYLOAD_LEN, MSG_DONTWAIT, (struct sockaddr *)&peer_addr, peerlen);
           
            localtime_r(&(tv.tv_sec), &tm_now);
            sprintf(buf, "%d-%d-%d %d:%d:%d", tm_now.tm_year+1900, tm_now.tm_mon+1, \
            tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
            sprintf(buf, "%s from %s:%d\r\n", buf, inet_ntoa(peer_addr.sin_addr), htons(peer_addr.sin_port));

            write(STDOUT_FILENO, buf, strlen(buf));
        }
        else if (ret == 0)
        {
            close(sockfd);
            break;
        }
        else
        {
            if(errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)
                continue;
        }
    }
}
