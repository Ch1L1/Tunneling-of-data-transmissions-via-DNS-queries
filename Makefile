CFLAGS = -std=gnu11 -Wall -Wextra -pedantic -fdiagnostics-color=always -g
CC = gcc
INC=-I.
# Wildcards do not work on eva?
# COMMON_SOURCES =  $(wildcard common/*.c) $(wildcard common/*.h)
# SENDER_SOURCES =  $(wildcard sender/*.c) $(wildcard sender/*.h)
# RECEIVER_SOURCES =  $(wildcard receiver/*.c) $(wildcard receiver/*.h)
COMMON_SOURCES =  common/base64.h common/base64.c
SENDER_SOURCES =  sender/dns_sender_events.c sender/dns_sender.c sender/dns_sender.h sender/dns_sender_events.h
RECEIVER_SOURCES =  receiver/dns_receiver_events.h receiver/dns_receiver.h receiver/dns_receiver_events.c receiver/dns_receiver.c


default: all

all: sender receiver

sender: $(COMMON_SOURCES) $(SENDER_SOURCES)
	$(CC) $(CFLAGS) $(INC) $(COMMON_SOURCES) $(SENDER_SOURCES) -o dns_sender 

receiver: $(COMMON_SOURCES) $(RECEIVER_SOURCES)
	$(CC) $(CFLAGS) $(INC) $(COMMON_SOURCES) $(RECEIVER_SOURCES) -o dns_receiver 

clean:
	rm dns_receiver dns_sender

.PHONY: default all clean sender receiver