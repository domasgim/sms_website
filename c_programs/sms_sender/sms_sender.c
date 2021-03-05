#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <ctype.h>
#include <iconv.h>
#include <errno.h>

#include "src/zhelpers.h"
#include "src/cJSON.h"
#include "src/pdu.h"
#include "sms_sender.h"

void send_gsm_msg(unsigned char msg[], int *serial_port) {
    //printf("Sending %s\n", msg);
    char read_buf [256];
    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno)); 
        return;
    }
    //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
    process_signal(read_buf, num_bytes);
    return;
}

struct termios setup_tty(int serial_port) {
    struct termios tty;

    if(tcgetattr(serial_port, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
        exit(-1);
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 1;    // Wait for up to 1s (10 deciseconds), returning as soon as 20 bits are received.
    tty.c_cc[VMIN] = 20;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        fprintf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        exit(1);
    }

    return tty;
}

int containsNonASCII(char message[]) {
    for (int i = 0; i < strlen(message); i++) {
        if ((unsigned char)message[i] > 127) {
            return 1;
        }
    }
    return 0;
}

int process_pdu(char recipient[], char message[], int with_udh, char udh_data[], int alphabet, int is_last_message, void *push) {

    // Converting UTF-8 string to UTF-16BE
    char Output[140];
    memset(Output, 0, sizeof(Output));
    size_t insize,out_size;
    memset(Output,0,sizeof(Output));
    int nconv=0;
    char *Inptr;
    char *outptr;  

    iconv_t cd = iconv_open("UTF-16BE","utf-8");
    insize=strlen(message);
    out_size=3*insize;
    Inptr = message;
    outptr=(char *)Output;
    nconv=iconv(cd,&Inptr,&insize,&outptr,&out_size);
    int loop_length = strlen(message) * 2;
    int msg_length = 0;

    if (nconv!=0) {
        fprintf(stderr,"error = %s\n", strerror(errno));
        exit(-1);
        printf("Unable to perform conversion ");
        return 0;
    }
    for (size_t i = 0; i < loop_length; i++) {
        //printf("%3d:%3hhx\n", i, Output[i]);
        msg_length++;

        if (Output[i + 1] == 0 && Output[i + 2] == 0) {
                break;
        }
    }


    int flash = 0;
    char* mode = "new";
    int report = 0;
    char pdu[1024] = {};
    int validity = 167; // 1 day
    int system_msg = 0;
    int replace_msg = 0;
    int to_type = 1;

    if (alphabet == 0) {
        make_pdu(recipient, message, strlen(message), alphabet, flash, report, with_udh, udh_data, mode, pdu, validity, replace_msg, system_msg, to_type);

    } else {
        make_pdu(recipient, Output, msg_length, alphabet, flash, report, with_udh, udh_data, mode, pdu, validity, replace_msg, system_msg, to_type);
        if (with_udh) {
            int i = 41;
            for (i = 41; i < strlen(pdu) - 1 ; i++) {
                pdu[i] = pdu[i+1];
            }
            pdu[strlen(pdu) - 1] = '\x00';
        }
    }
    //printf("%s\n", pdu);
    send_sms(pdu, is_last_message, push);
}

int process_sms_data(char recipient[], char message[], void *push) {
    int contains_non_ascii = containsNonASCII(message);
    int ref_no = rand() % 100;
    srand(time(0));


    if (contains_non_ascii == 0) {
        char split_message[154];
        memset(split_message,0,sizeof(split_message));
        int divisions = strlen(message) / 153;
        divisions++; 
        if (divisions > 1) {
            for (int i = 0; i < divisions; i++) {
            int iterations = 0;
            for (int j = 0; j < strlen(message); j++) {
                if (iterations == 153) {
                    split_message[j] = '\0';
                    break;
                }
                iterations++;
                split_message[j] = message[153 * i + j];
            }

            char udh_data[17];
            //sprintf(udh_data, "05 00 03 %02hhX %02hhX %02hhX", ref_no, divisions, i + 1);

            if(i == divisions - 1) {
                    process_pdu(recipient, split_message, 1, udh_data, 0, 1, push);
            } else {
                    process_pdu(recipient, split_message, 1, udh_data, 0, 0, push);
            }
            memset(split_message,0,sizeof(split_message));
            }
        } else {
            process_pdu(recipient, message, 0, "", 0, 1, push);
        }

    } else {
        char split_message[63];
        memset(split_message,0,sizeof(split_message));
        int divisions = strlen(message) / 63;
        divisions++; 

        if (divisions > 1) {
            for (int i = 0; i < divisions; i++) {
                int iterations = 0;
                for (int j = 0; j < strlen(message); j++) {
                    if (iterations == 63) {
                        break;
                    }
                    iterations++;
                    split_message[j] = message[63 * i + j];
                }

                char udh_data[17];
                sprintf(udh_data, "05 00 03 %02hhX %02hhX %02hhX", ref_no, divisions, i + 1);
                if(i == divisions - 1) {
                    process_pdu(recipient, split_message, 1, udh_data, 2, 1, push);

                } else {
                    process_pdu(recipient, split_message, 1, udh_data, 2, 0, push);
                }
                memset(split_message,0,sizeof(split_message));
            }
        } else {
            process_pdu(recipient, message, 0, "", 2, 1, push);
        }
    }
    return 0;
}

void process_pdu_return(char msg[], int size, void *push) {
    for (int i = 0; i < size; i++) {
        if (msg[i] == '+' && msg[i + 1] == 'C' && msg[i + 2] == 'M' && 
            msg[i + 3] == 'S' && msg[i + 5] == 'E' && msg[i + 6] == 'R' 
            && msg[i + 7] == 'R' && msg[i + 8] == 'O' && msg[i + 6] == 'R') {
                printf("Failed to send a message\n");
                printf("GSM return: %s", msg);
                s_send(push, "-10");
                return;
        }
    }
    printf("Message sent successfuly\n");
    s_send(push, "10");
    return;
}

void send_sms(char pdu[], int is_last_message, void *push) {
    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);
    char * command_cmfg_s ;

    int length = (strlen(pdu) - 2) / 2;
    char TPDU_length[3];

    sprintf(TPDU_length, "%d", length);

    printf("\nPDU: %s\n", pdu);

    unsigned char msg11[] = "AT+CMGF=0\r";
    unsigned char msg11_alt[] = "AT+CMGF=0\r";
    char msg12[32];
    sprintf(msg12, "AT+CMGS=%d\r", length);

    unsigned char msg13[1024];

    unsigned char cmd_end[] = "\r";
    unsigned char msg_end[] = "\x1A\r";
    unsigned char msg_end_alt[] = "\x1A\r";

    strncat(msg13, pdu, 512);
    strncat(msg13, msg_end, 5);
    send_gsm_msg(msg11_alt, serial_port);
    send_gsm_msg(msg12, serial_port);

    strncat(pdu, msg_end_alt, 4);
    
    // SEND PDU (UNSIGNED VERSION DOESNT WORK)
    char read_buf [256];
    write(serial_port, pdu, strlen(pdu));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno));
        return;
    }
    //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);

    if(is_last_message) {
        process_pdu_return(read_buf, num_bytes, push);
    }
    close(serial_port);
    return 0; // success
}

int process_signal(char msg[], int size) {
    int contains_signal = 0;
    char signal_strength[4];
    int signal_value;

    for (int i = 0; i < size; i++) {
        if (msg[i] == '+' && msg[i + 1] == 'C' && msg[i + 2] == 'S' && 
            msg[i + 3] == 'Q' && msg[i + 4] == ':') {

            sprintf(signal_strength, "%c%c", msg[i + 6], msg[i + 7]);
            signal_value = atoi(signal_strength);
            printf("Signal strength: %d\n", signal_value);

            if (signal_value < 3) {
                printf("Error: no signal\n");
                exit(1);

                return 1;
            } else {
                return 0;
            }
        }
    }

    return 0;
}

void check_signal() {
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);
    unsigned char msg1[] = "AT+CSQ\r";

    printf("Checking signal strength...\n");
    send_gsm_msg(msg1, serial_port);
}

char * get_time() {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    return buffer;
}

int main(int argc, char *argv[]) {

    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    const cJSON *recipient = NULL;
    const cJSON *message = NULL;

    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_PULL);
    int rc = zmq_bind (subscriber, "tcp://127.0.0.1:5555");

    void *push = zmq_socket (context, ZMQ_PUSH);
    rc = zmq_bind (push, "tcp://127.0.0.1:5540");
    int timeout = 5000; /* 5 seconds */
    zmq_setsockopt(push, ZMQ_SNDTIMEO, &timeout, sizeof(int));

    check_signal();
    printf("Program ready, waiting for messages...\n"); 

    char msg1[] = "Option-1";
    char msg2[] = "Option-2";
    char msg3[] = "Option-3";
    char msg4[] = "Long live Lithuania";
    while (1) 
    {


        char *contents = s_recv(subscriber);
        fflush (stdout);

        timer = time(NULL);
        tm_info = localtime(&timer);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

        cJSON *message_json = cJSON_Parse(contents);

        if (message_json == NULL)
        {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }

            cJSON_Delete(message_json);
        }

        recipient = cJSON_GetObjectItemCaseSensitive(message_json, "recipient");
        message = cJSON_GetObjectItemCaseSensitive(message_json, "content");

        char *recipient_modified = recipient->valuestring;
        recipient_modified++;

        printf("\n========================================================================\n");
        printf("[%s]\n", buffer);
        printf("Recipient: %s\n", recipient->valuestring);
        printf("Message: %s\n", message->valuestring);

        free(contents);

        //send_sms(recipient_modified, message->valuestring);
        process_sms_data(recipient_modified, message->valuestring, push);
        //printf("========================================================================\n\n");
    } 
    return 0; // success
}
