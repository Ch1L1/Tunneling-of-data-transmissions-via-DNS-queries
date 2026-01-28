# Tunneling-of-data-transmissions-via-DNS-queries
Implementation of a DNS tunneling tool in C, enabling data transfer and exfiltration via DNS queries.

### Author: Tomas Homola  
### Date: 14.11.2022  

## Description
The goal of this project, implemented in the C programming language, is to transmit data obtained from a file or from STDIN using DNS packets and encode them for transfer to a server. When data is sent to the implemented server application (`dns_receiver.c`), the received message is decoded back into a human-readable format.

## Limitations
- IPv4 only  
- IPv6 is not supported  

## Submitted Files
- `dns_sender.c`  
- `dns_header.h`  
- `dns_receiver.c`  
- `dns_receiver.h`  
- `base64.c`  
- `base64.h`  
- `Makefile`  

## Example Usage
```bash
./dns_sender -u 8.8.8.8 example.com ulozsem.txt test.txt
echo "Good day" | ./dns_sender -u 8.8.8.8 example.com ulozsem.txt
./dns_sender example.com ulozsem.txt test.txt

./dns_receiver example.com /etc/hatamata/
