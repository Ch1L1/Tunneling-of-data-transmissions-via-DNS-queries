/********************************************/
/* NAME:   dns_sender.c                     */
/*                                          */
/* AUTHOR: Tomas Homola(xhomol28)           */
/*                                          */
/* DESCR:  Sending coded data from          */
/*         file/STDIN in DNS format         */  
/*                                          */  
/********************************************/  

#include "dns_sender.h"
#include "../common/base64.h"
#include "dns_sender_events.h"

int main(int argc, char *argv[], struct Sender_Args *args)
{
    parsingArguments(argc, argv, args);

    int sock, msg_size, i;
    struct sockaddr_in server, from;
    int index = 1;
    socklen_t len;
    char buffer[512];
    char f_buffer[512];
    char *coded_buffer_ptr;
    char *coded_dst_filepath_ptr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        err(1, "socket() failed\n");
    printf("* Socket created\n");

    server.sin_addr.s_addr = inet_addr(args->upstream_dns_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // set DNS structure to standard queries
    memset(buffer, 0, sizeof(buffer));
    dns_header_t header;
    memset(&header, 0, sizeof(dns_header_t));

    
    header.qr = htons(0x0100);
    header.qdcount = htons(1);

    // point to the query portion
    dns_question_t dns_question;
    dns_question.type = htons(1);
    dns_question.qclass = htons(1);

    len = sizeof(server);
    size_t *output_length;

    char dot[] = ".";
    int chunkId = 1;

    // send DST_FILEPATH as first packet //
    dns_sender__on_transfer_init((struct sockaddr *)&server);

    coded_dst_filepath_ptr = base64_encode((unsigned char *)args->dst_filepath, strlen(args->dst_filepath), (size_t *)&output_length);

    dns_sender__on_chunk_encoded(args->dst_filepath, chunkId, coded_dst_filepath_ptr);   

    char *pointer;

    dns_question.name = calloc(output_length + 2, sizeof(char));
        if (dns_question.name == NULL)
        {
            err(1, "Calloc failed\n");
        }

        pointer = dns_question.name;

        memcpy(dns_question.name, coded_dst_filepath_ptr, output_length);
        strcat(dns_question.name, dot);

        pointer += strlen(dns_question.name);

        strcat(pointer, args->base_host);

        dns_question.name = ChangeToDnsFormat(dns_question.name);

        /* Copy all fields into a single, concatenated packet */
        size_t packetlen = sizeof(header) + strlen(dns_question.name) + 1 + sizeof(dns_question.type) + sizeof(dns_question.qclass);
        uint8_t *first_packet = calloc(packetlen, sizeof(uint8_t));
        uint8_t *first_p = (uint8_t *)first_packet;

        /* copy DNS header  */
        header.id = htons(index++);
        memcpy(first_p, &header, sizeof(header));
        first_p += sizeof(header);

        /* copy dns query qname */
        memcpy(first_p, dns_question.name, strlen(dns_question.name) + 1);
        first_p += strlen(dns_question.name) + 1;

        /* copy dns query type and class */
        memcpy(first_p, &dns_question.type, sizeof(dns_question.type));
        first_p += sizeof(dns_question.type);
        memcpy(first_p, &dns_question.qclass, sizeof(dns_question.qclass));

        /* send packet to server */
        dns_sender__on_chunk_sent((struct sockaddr *)&server, args->dst_filepath, chunkId, sizeof(first_packet)); 

        i = sendto(sock, first_packet, packetlen, 0, (struct sockaddr *)&server, len);                                                  //-------------------
        if (i == -1)
        {
            err(1, "sendto() failed");
        }
        chunkId++;
        len = sizeof(from);

        if (getsockname(sock, (struct sockaddr *)&from, &len) == -1)
            err(1, "getsockname() failed");

        printf("* Data sent from port %d (%d) to %s, port %d (%d)\n", ntohs(from.sin_port), from.sin_port, inet_ntoa(server.sin_addr), ntohs(server.sin_port), server.sin_port);

        //read the answer from the server
        if ((i = recvfrom(sock, buffer, 512, 0, (struct sockaddr *)&from, &len)) == -1)
         {
             err(1, "recfrom() failed");
         }
         else if (i > 0)
         {
             printf("* UDP packet received from %s, port %d (%d)\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port), from.sin_port);
         }

    // ---------------------------------------------------------------------- //
    int fileSize = 0;
    // read data from a user and send them to the server
    while ((msg_size = fread(f_buffer, sizeof(char), 45, args->fp)) > 0)
    {
        fileSize += msg_size;
        coded_buffer_ptr = base64_encode((unsigned char *)f_buffer, msg_size, (size_t *)&output_length);
        dns_sender__on_chunk_encoded(args->dst_filepath, chunkId, coded_buffer_ptr);   //---------------------------------------------
        char *ptr;

        dns_question.name = calloc(output_length + 2, sizeof(char));
        if (dns_question.name == NULL)
        {
            err(1, "Calloc failed\n");
        }

        ptr = dns_question.name;

        memcpy(dns_question.name, coded_buffer_ptr, output_length);
        strcat(dns_question.name, dot);

        ptr += strlen(dns_question.name);

        strcat(ptr, args->base_host);

        dns_question.name = ChangeToDnsFormat(dns_question.name);

        /* Copy all fields into a single, concatenated packet */
        size_t packetlen = sizeof(header) + strlen(dns_question.name) + 1 + sizeof(dns_question.type) + sizeof(dns_question.qclass);
        uint8_t *packet = calloc(packetlen, sizeof(uint8_t));
        uint8_t *p = (uint8_t *)packet;

        /* copy DNS header  */
        header.id = htons(index++);
        memcpy(p, &header, sizeof(header));
        p += sizeof(header);

        /* copy dns query qname */
        memcpy(p, dns_question.name, strlen(dns_question.name) + 1);
        p += strlen(dns_question.name) + 1;

        /* copy dns query type and class */
        memcpy(p, &dns_question.type, sizeof(dns_question.type));
        p += sizeof(dns_question.type);
        memcpy(p, &dns_question.qclass, sizeof(dns_question.qclass));


        /* send packet to server */
        dns_sender__on_chunk_sent((struct sockaddr *)&server, args->dst_filepath, chunkId, sizeof(packet)); //--------------------
        i = sendto(sock, packet, packetlen, 0, (struct sockaddr *)&server, len);
        if (i == -1)
        {
            err(1, "sendto() failed");
        }
        chunkId++;
        len = sizeof(from);

        if (getsockname(sock, (struct sockaddr *)&from, &len) == -1)
            err(1, "getsockname() failed");

        printf("* Data sent from port %d (%d) to %s, port %d (%d)\n", ntohs(from.sin_port), from.sin_port, inet_ntoa(server.sin_addr), ntohs(server.sin_port), server.sin_port);

        //read the answer from the server
        if ((i = recvfrom(sock, buffer, 512, 0, (struct sockaddr *)&from, &len)) == -1)
         {
             err(1, "recfrom() failed");
         }
         else if (i > 0)
         {
             printf("* UDP packet received from %s, port %d (%d)\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port), from.sin_port);
         }
        free(coded_buffer_ptr);
    }
    // read data until end-of-file (CTRL-D)
    if (msg_size == -1)
        err(1, "reading failed");

    // ---------------------------------------------------------------------- //

// send ALL_PACKETS_SEND as last packet //
    char *last_packet = "* ALL_PACKETS_SEND *";
    last_packet = base64_encode((unsigned char *)last_packet, strlen(last_packet), (size_t *)&output_length);
    dns_sender__on_chunk_encoded(args->dst_filepath, chunkId, last_packet);   //---------------------------------------------
    
    char *l_pointer;

    dns_question.name = calloc(output_length + 2, sizeof(char));
        if (dns_question.name == NULL)
        {
            err(1, "Calloc failed\n");
        }

        l_pointer = dns_question.name;

        memcpy(dns_question.name, last_packet, output_length);
        strcat(dns_question.name, dot);

        l_pointer += strlen(dns_question.name);

        strcat(l_pointer, args->base_host);

        dns_question.name = ChangeToDnsFormat(dns_question.name);

        /* Copy all fields into a single, concatenated packet */
        size_t l_packetlen = sizeof(header) + strlen(dns_question.name) + 1 + sizeof(dns_question.type) + sizeof(dns_question.qclass);
        uint8_t *l_packet = calloc(l_packetlen, sizeof(uint8_t));
        uint8_t *l_p = (uint8_t *)l_packet;

        /* copy DNS header  */
        header.id = htons(0xFFFF);
        memcpy(l_p, &header, sizeof(header));
        l_p += sizeof(header);

        /* copy dns query qname */
        memcpy(l_p, dns_question.name, strlen(dns_question.name) + 1);
        l_p += strlen(dns_question.name) + 1;

        /* copy dns query type and class */
        memcpy(l_p, &dns_question.type, sizeof(dns_question.type));
        l_p += sizeof(dns_question.type);
        memcpy(l_p, &dns_question.qclass, sizeof(dns_question.qclass));
        
        /* send packet to server */
        
        i = sendto(sock, l_packet, l_packetlen, 0, (struct sockaddr *)&server, len);
        dns_sender__on_chunk_sent((struct sockaddr *)&server, args->dst_filepath, chunkId, sizeof(l_packet)); //-----------------
        if (i == -1)
        {
            err(1, "sendto() failed");
        }
        chunkId++;
        // ---------------------------------------------------------------------- //
    dns_sender__on_transfer_completed(args->dst_filepath, fileSize);                //------------------------
    close(sock);
    printf("* Closing the client socket...\n");
    return 0;
}

void parsingArguments(int argc, char *argv[], struct Sender_Args *args)
{
    FILE *localDNS;
    ssize_t read;
    size_t len = 0;
    char *token;
    char *line = NULL;

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[1], "-u")) == 0)
        { 
            // s parametrom "-u UPSTREAM_DNS_IP"
            strcpy(args->upstream_dns_ip, argv[2]);
            strcpy(args->base_host, argv[3]);
            strcpy(args->dst_filepath, argv[4]);
            if (argc == 6)
            {
                args->fp = fopen(argv[5], "r");
            }
            else
            {
                args->fp = stdin;
            }
            break;
        }
        else
        { 
            // bez "-u UPSTREAM_DNS_IP"
            if (argc == 4)
            { 
                // s parametrom "SRC_FILEPATH"
                args->fp = fopen(argv[3], "r");
            }
            else
            { 
                // bez parametru "SRC_FILEPATH"
                args->fp = stdin;
            }
            strcpy(args->base_host, argv[1]);
            strcpy(args->dst_filepath, argv[2]);
            // nacitat lokalne DNS zo systemu
            localDNS = fopen("/etc/resolv.conf", "r");
            while ((read = getline(&line, &len, localDNS)) != -1)
            {
                if (strncmp(line, "nameserver", strlen("nameserver")) == 0)
                {
                    token = strtok(line, " \n");
                    token = strtok(NULL, " \n");
                    strcpy(args->upstream_dns_ip, token);
                    break;
                }
            }
            fclose(localDNS);
            if (line)
            {
                free(line);
            }
            break;
        }
    }
}

char *ChangeToDnsFormat(char *name)
{
    // TODO assert pre_qname != NULL
    int size_of_name = strlen(name);
    char *qname = (char *)calloc(size_of_name + 2, sizeof(char));
    name[size_of_name] = 0;
    char *dot = qname;
    char *last_dot = name - 1;
    char *traveller = name;

    while (*traveller != '\0')
    {
        while (*traveller != '.' && *traveller != 0)
            traveller++;
        char length = (traveller - last_dot) - 1;
        *dot++ = length;

        for (char *p = last_dot + 1; p < traveller; p++)
            *dot++ = *p;
        last_dot = traveller++;
    }
    *dot = 0;

    return qname;
}
/* 
bool isValidIpAddress(char *ip_address){
    int result = inet_pton(AF_INET, ip_address, &(sa.sin_addr));
    return result != 0;
} */

int file_size(FILE *file_ptr){
    fseek(file_ptr, 0, SEEK_END);
    int size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);
    return size;
}
