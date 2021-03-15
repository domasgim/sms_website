CC=gcc
CFLAGS=-Wall -D NUMBER_OF_MODEMS=1

all: sms_receiver

sms_receiver:	sms_receiver.o SMS.o utf.o pdu.o list.o list_iterator.o list_node.o
	$(CC) $(CFLAGS) src/cJSON.c src/sms_receiver.o src/SMS.o src/utf.o src/pdu.o src/list.o src/list_iterator.o src/list_node.o -lcurl -o sms_receiver

sms_receiver.o: src/sms_receiver.c
	$(CC) $(CFLAGS) -c src/sms_receiver.c -o src/sms_receiver.o

SMS.o: src/SMS.c
	$(CC) $(CFLAGS) -c src/SMS.c -o src/SMS.o

utf.o: src/utf.c
	$(CC) $(CFLAGS) -c src/utf.c -o src/utf.o

pdu.o: src/pdu.c
	$(CC) $(CFLAGS) -c src/pdu.c -o src/pdu.o

list.o: src/list.c
	$(CC) $(CFLAGS) -c src/list.c -o src/list.o

list_iterator.o: src/list_iterator.c
	$(CC) $(CFLAGS) -c src/list_iterator.c -o src/list_iterator.o

list_node.o: src/list_node.c
	$(CC) $(CFLAGS) -c src/list_node.c -o src/list_node.o

clean:
	rm src/*.o
	rm sms_receiver
