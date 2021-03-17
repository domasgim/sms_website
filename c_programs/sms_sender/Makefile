CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

all: sms_sender

sms_sender_debug:	sms_sender_debug.o src/pdu.o
	$(CC) $(CFLAGS) src/cJSON.c src/sms_sender_debug.o src/pdu.o -lzmq -o sms_sender

sms_sender:	src/sms_sender.o src/pdu.o
	$(CC) $(CFLAGS) src/cJSON.c src/sms_sender.o src/pdu.o -lzmq -o sms_sender

sms_sender.o: src/sms_sender.c
	$(CC) $(CFLAGS) -c src/sms_sender.c -o src/sms_sender.o

sms_sender_debug.o: src/sms_sender.c
	$(CC) $(CFLAGS) -c -DDEBUG src/sms_sender.c -o src/sms_sender_debug.o

pdu.o: src/pdu.c
	$(CC) $(CFLAGS) -c src/pdu.c -o src/pdu.o

clean:
	rm sms_sender
	rm src/*.o
