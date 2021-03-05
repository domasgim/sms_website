CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

all: sms_sender

sms_sender:	sms_sender.o pdu.o
	$(CC) $(CFLAGS) src/cJSON.c sms_sender.o src/pdu.o -lzmq -o sms_sender

sms_sender.o: sms_sender.c
	$(CC) $(CFLAGS) -c sms_sender.c -o sms_sender.o

pdu.o:	src/pdu.c
	$(CC) $(CFLAGS) -c src/pdu.c -o src/pdu.o

clean:
	rm *.o
	rm src/*.o
	rm sms_sender
