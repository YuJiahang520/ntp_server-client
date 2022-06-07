/*                  NTPv3报文格式
0   2     5     8               16              24              32
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|LI | VN  |Mode |    Stratum    |     Poll      |   Precision   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Root Delay                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Root Dispersion                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Reference Identifier                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                   Reference Timestamp (64)                    |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                   Originate Timestamp (64)                    |
|                   客户端发送时间请求的时间                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                   Receive Timestamp (64)                      |
|                   服务器收到时间请求的时间                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                   Transmit Timestamp (64)                     |
|                   此命令发出的时间                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                  Authentication (optional) (64)               |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                    NTP时间戳格式
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Integer Part                          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Fraction Part                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
#ifndef __NTP_H__
#define __NTP_H__

#include <stdint.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define NTP_PORT                    123
#define NTP_SERVER_HOST             "0.0.0.0"

#define VERSION_3                   3
#define VERSION_4                   4

#define MODE_CLIENT                 3
#define MODE_SERVER                 4

//#define NTP_PAYLOAD_LEN           48
#define NTP_PAYLOAD_LEN             sizeof(NTP_PACK)

#define JAN_1970                    0x83AA7E80

#define NTP_CONV_FRAC32(x)          (uint64_t) ((x) * ((uint64_t)1<<32))    
#define NTP_REVE_FRAC32(x)          ((double) ((double) (x) / ((uint64_t)1<<32)))      

#define FRACPART_ENCODE(x)          ((uint32_t)NTP_CONV_FRAC32( (x) / 1000000.0 )) 
#define FRACPART_DECODE(x)          ((uint32_t)NTP_REVE_FRAC32( (x) * 1000000.0 )) 

#define TIMESTAMP_FORMAt_64_TO_DOUBLE(x) (htonl(x.intpart) - JAN_1970 +FRACPART_DECODE(htonl(x.fracpart)) / 1000000.0)  

typedef struct{
    uint8_t Mode : 3; 
    uint8_t VN : 3;
    uint8_t LI : 2;
    uint8_t Stratum;
    uint8_t Poll;
    uint8_t Precision;
}NTP_HEAD;

typedef struct{
    uint16_t intpart;
    uint16_t fracpart;
}TIMESTAMP_FORMAt_32;

typedef struct{
    uint32_t intpart;
    uint32_t fracpart;
}TIMESTAMP_FORMAt_64;

typedef struct{
    NTP_HEAD                head;
    TIMESTAMP_FORMAt_32     root_delay;
    TIMESTAMP_FORMAt_32     root_dispersion;
    uint32_t                reference_identifier;
    TIMESTAMP_FORMAt_64     Reference_Timestamp;
    TIMESTAMP_FORMAt_64     originate_timestamp;
    TIMESTAMP_FORMAt_64     receive_timestamp;
    TIMESTAMP_FORMAt_64     transmit_timestamp;
}NTP_PACK;

struct timeval ntp_request(const char * server);
void ntp_server_task(void);

#endif
