/********************************************/
/* NAME:   dns_sender.c                     */
/*                                          */
/* AUTHOR: Tomas Homola (xhomol28)          */
/*                                          */
/* DESCR:  Receiving coded data from        */
/*         client in DNS format             */
/*                                          */
/********************************************/

#include "dns_receiver.h"
#include "../common/base64.h"
#include "dns_receiver_events.h"
#include "../sender/dns_sender.h"

int main(int argc, char *argv[], struct Receiver_args *args)
{
    parsArguments(argc, argv, args);
    int fd;
    struct sockaddr_in server;
    int msg_size, i, header_size;
    char buffer[512];
    struct sockaddr_in client;
    socklen_t length;
    size_t *output_length;
    dns_question_t dns_q;
    dns_header_t dns_h;
    char *pointer;
    char *decoded_message;
    int basehost_len = strlen(args->base_host);
    char *packet_base_hostname;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    printf("* Opening an UDP socket ...\n");
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        err(1, "socket() failed");
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        err(1, "setsockopt(SO_REUSEADDR) failed");

    printf("* Binding to the port %d (%d)\n", PORT, server.sin_port);
    if (bind(fd, (struct sockaddr *)&server, sizeof(server)) == -1)
        err(1, "bind() failed");
    length = sizeof(client);

    FILE *file;
    char buff[1024] = "";
    dns_header_t *header;
    struct stat st = {0};
    int file_size = 0;
    int chunkId = 1;

    dns_receiver__on_transfer_init((struct sockaddr *)&client);

    while ((msg_size = recvfrom(fd, buffer, 512, 0, (struct sockaddr *)&client, &length)) >= 0)
    {

        printf("* Request received from %s, port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        char *message = buffer + sizeof(dns_header_t) + 1;

        header = (dns_header_t *)buffer;
        // set flag to response
        header->qr = htons(0x8100);

        header->id;

        packet_base_hostname = message + strlen(message) - basehost_len;

        decoded_message = base64_decode((unsigned char *)message, packet_base_hostname - message - 1, (size_t *)&output_length);
        

        if (decoded_message == NULL)
            err(1, "nieco je mocne zle");

        //fwrite(decoded_message, 1, output_length, stdout);s

        if (header->id == htons(0x0001))
        {
            memset(buff, 0, sizeof(buff));
            strcpy(buff, args->dst_filepath);
            strcat(buff, "/");
            if(stat(buff, &st) == -1){
                 if(mkdir(buff, 0777) < 0)
                    err(1, "Unsupported: long path");
            }
            memcpy(buff + strlen(buff), decoded_message, output_length);

            dns_receiver__on_query_parsed(buff, message);               // <--------
            dns_receiver__on_chunk_received((struct sockaddr *)&client, buff, chunkId, length);

            file = fopen(buff, "w");
            if (file == NULL) {
                err(1, "fopen() failed");
            }
        }
        else if (header->id == htons(0xFFFF))
        {
            dns_receiver__on_transfer_completed(buff, file_size);
            fclose(file);
        }
        else
        {
            dns_receiver__on_query_parsed(buff, message);
            dns_receiver__on_chunk_received((struct sockaddr *)&client, buff, chunkId, length);
            file_size += strlen(decoded_message);
            fwrite(decoded_message, 1, output_length, file);
        }

        i = sendto(fd, buffer, msg_size, 0, (struct sockaddr *)&client, length);
        if (i == -1)
            err(1, "sendto() failed");
        else if (i != msg_size)
            err(1, "sendto(): Buffer written just parially");
        else
            printf("* Data \"%.*s\" sent from port %d to %s, port %d\n", i - 1, message, ntohs(server.sin_port), inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    }
    printf("* Closing the socket\n");
    close(fd);

    //dns_receiver__on_transfer_completed(char *filePath, int fileSize);

    return 0;
}

int parsArguments(int argc, char *argv[], struct Receiver_args *args)
{
    for (int i = 1; i < argc; i++)
    {
        strcpy(args->base_host, argv[1]);
        strcpy(args->dst_filepath, argv[2]);
    }
    return 0;
}

/* void header_debug(struct dns_header *header){
    fprintf(stderr, "DNS Header:\n");
    fprintf(stderr, "\t|-ID : %d\n", ntohs(header->id));
    fprintf(stderr, "\t|-QR : %d\n", header->qr);
    fprintf(stderr, "\t|-QDcount : %d\n", ntohs(header->qdcount));
}
 */
