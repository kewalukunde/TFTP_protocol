/* =====================================================================================
 *
 *       Filename:  tftp.h
 *        Created:  Friday 31 March 2017 12:12:33  IST
 *         Author:  KEWAL UKUNDE , kewalukunde@gmail.com
 *
 * =====================================================================================
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
 
#define IP_ADDRESS  "127.0.0.1"
#define SERVER_PORT 7000 
#pragma pack(1)

#define DATA_SIZE 450

// enum for error  
typedef enum error
{
    FILE_NOT_FOUND = 1,
    FILE_EXIST,
}error_pack;

/* enum for opcode */
typedef enum opcode
{
    READ_REQ = 1,
    WRITE_REQ,
    DATA_PACKET,
    ACK,
    ERROR,
}opcode;

/* comman structure to send data */
typedef struct packet
{
    short int opcode;
    char filename[15];
    short int b_num;
    char data[450];
    short int e_code;
    char msg[20];
}packet;

char *get_ip_address(void);
