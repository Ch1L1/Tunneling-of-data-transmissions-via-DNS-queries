#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h>
#include<err.h>

#define PORT 53

typedef struct Sender_Args{
    char upstream_dns_ip[50];
    char base_host[50];
    char dst_filepath[50];
    FILE *fp;
}args_t;

typedef struct dns_header{
    uint16_t id;
    uint16_t qr;        // query/response flag
    uint16_t qdcount;           // number of question entries
    uint16_t ancount;           // number of answer entries
    uint16_t nscount;           // number of authority entries
    uint16_t arcount;           // number of resource entries
}dns_header_t;

typedef struct dns_question{
    char *name;                 // pointer to the domain name in memory
    uint16_t type;              // QTYPE
    uint16_t qclass;            // QCLASS
}dns_question_t;

/**
 * @brief Parsovanie vstupnych argumentov
 * @param argc
 * @param argv
 */
void parsingArguments(int argc, char *argv[], struct Sender_Args *args);

/**
 * @brief Uprava qname do validneho DNS formatu
 * 
 * @param name 
 * @return char* 
 */
char *ChangeToDnsFormat(char *name);

/**
 * @brief Size of file
 * 
 * @param file_ptr 
 * @return int 
 */
int file_size(FILE *file_ptr);
