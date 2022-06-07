#include "ntp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>

static NTP_PACK net_pack;
static TIMESTAMP_FORMAt_64 originate_timestamp;

// NTP组包
static void ntp_client_pack_encode()
{
    struct timeval tv;

    memset(&net_pack, 0, NTP_PAYLOAD_LEN);
    
    net_pack.head.VN = VERSION_3;
    net_pack.head.Mode = MODE_CLIENT;

    gettimeofday(&tv, NULL);
    net_pack.transmit_timestamp.intpart = htonl(tv.tv_sec + JAN_1970);
    net_pack.transmit_timestamp.fracpart = htonl(FRACPART_ENCODE(tv.tv_usec));

    /*
    for (size_t i = 0; i < NTP_PAYLOAD_LEN; i++)
    {
        printf("%02X ", ((uint8_t *)&net_pack)[i]);
    }
    printf("\r\n");
    */

    // 存下发送的时间  用来验证返回数据的正确性
    originate_timestamp = net_pack.transmit_timestamp;
}

// 域名解析
static in_addr_t inet_host(const char *host)
{
    in_addr_t saddr;
    struct hostent *hostent;

    if ((saddr = inet_addr(host)) == INADDR_NONE) 
    {
        if ((hostent = gethostbyname(host)) == NULL)
            return INADDR_NONE;

        memmove(&saddr, hostent->h_addr, hostent->h_length);
    }

    return saddr;
}

struct timeval ntp_request(const char * host)
{
    int sockfd, ret;
    struct timeval ret_tv = {0};
    struct timeval tv = {0};
    struct sockaddr_in server_addr = {0};
    double server_Recv_Time, server_Send_Time, device_Recv_Time, device_Send_Time;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) 
    {
        printf("fail to creat ntp_socket.\r\n");
        return ret_tv;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(NTP_PORT);
    server_addr.sin_addr.s_addr = inet_host(host);

    ntp_client_pack_encode();

    ret = sendto(sockfd, &net_pack, NTP_PAYLOAD_LEN, MSG_DONTWAIT, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (ret != NTP_PAYLOAD_LEN)
    {
        printf("ntp_socket send err\r\n");
        close(sockfd);
        return ret_tv;
    }
        
    tv.tv_sec = 3;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ret = recvfrom(sockfd, &net_pack, NTP_PAYLOAD_LEN, 0, NULL, NULL);
    if (ret > 0)
    {
        if (memcmp(&originate_timestamp, &(net_pack.originate_timestamp), sizeof(TIMESTAMP_FORMAt_64)))
        {
            printf("ntp format err\r\n");
            close(sockfd);
            return ret_tv;
        }
        else
        {
            gettimeofday(&tv, NULL);
            device_Send_Time = TIMESTAMP_FORMAt_64_TO_DOUBLE(net_pack.originate_timestamp);
            //printf("device_Send_Time = %lf\r\n", device_Send_Time);
            server_Recv_Time = TIMESTAMP_FORMAt_64_TO_DOUBLE(net_pack.receive_timestamp);
            //printf("server_Recv_Time = %lf\r\n", server_Recv_Time);
            server_Send_Time = TIMESTAMP_FORMAt_64_TO_DOUBLE(net_pack.transmit_timestamp);
            //printf("server_Send_Time = %lf\r\n", server_Send_Time);
            device_Recv_Time = tv.tv_sec + tv.tv_usec / 1000000.0;
            //printf("device_Recv_Time = %lf\r\n", device_Recv_Time);

            device_Recv_Time = (server_Recv_Time + server_Send_Time + device_Recv_Time - device_Send_Time) / 2;
            //printf("timestamp1 = %lf\r\n", device_Recv_Time);
            ret_tv.tv_sec = (int)device_Recv_Time;
            ret_tv.tv_usec = (device_Recv_Time - ret_tv.tv_sec) * 1000000;
            //printf("timestamp2 = %d.%d\r\n", ret_tv.tv_sec, ret_tv.tv_usec);

            close(sockfd);
            return ret_tv;
        } 
    }
    else
    {
        printf("ntp request timeout\r\n");
        close(sockfd);
        return ret_tv;
    }
}
