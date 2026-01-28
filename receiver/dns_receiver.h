#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h>
#include<ctype.h>
#include<err.h>

#define PORT 53
#define BUFFER (1024)

struct Receiver_args{
    char base_host[50];
    char dst_filepath[50];
}args;

/**
 * @brief Parsovanie vstupnych argumentov
 * @param argc
 * @param argv
 */
int parsArguments(int argc, char *argv[], struct Receiver_args *args);

/**
 * @brief Debug print for DNS header
 * @param header
 *
 */
//void header_debug(struct dns_header *header);