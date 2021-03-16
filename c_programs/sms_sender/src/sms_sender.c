#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>     /* Used to stop the program */
#include <unistd.h>
#include <termios.h>    /* Serial port options */
#include <time.h>       /* Printing out timestamps */
#include <fcntl.h>      /* File controls: O_RDWR */
#include <iconv.h>      /* UTF16 conversions */
#include <zmq.h>
#include <stdatomic.h>

#include "../include/cJSON.h"
#include "../include/sms_sender.h"

#define CONFIG_PATH "sms_sender_config.json"

/* debug macro for printing additional info; compile it with -DDEBUG flag */
#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

atomic_int signal_received = 0;

void handler() {
    signal_received = 1;
    printf("\nStopping process...\n");
}

int s_send (void *socket, char *string) {
    int size = zmq_send (socket, string, strlen (string), 0);
    return size;
}

char * s_recv (void *socket) {
    char buffer [2048];
    int size = zmq_recv (socket, buffer, 2048 - 1, 0);
    if (size == -1)
        return NULL;
    buffer[size < 2048 ? size : 2048 - 1] = '\0';
    return strndup (buffer, sizeof(buffer) - 1);
}

int process_pdu_return(char msg[], int size) {
    for (int i = 0; i < size; i++) {
        if (msg[i] == '+' && msg[i + 1] == 'C' && msg[i + 2] == 'M' && 
            msg[i + 3] == 'S' && msg[i + 5] == 'E' && msg[i + 6] == 'R' 
            && msg[i + 7] == 'R' && msg[i + 8] == 'O' && msg[i + 6] == 'R') {
                printf("GSM return: %s", msg);
                return -1;
        }
    }
    return 0;
}

int contains_non_ascii(char message[]) {
    for (int i = 0; i < strlen(message); i++) {
        if ((unsigned char)message[i] > 127) {
            return 1;
        }
    }
    return 0;
}

void send_gsm_msg(char msg[], int serial_port) {
    DEBUG_PRINT(("Sending %s\n", msg));
    //printf("Sending %s\n", msg);
    char read_buf [256];
    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        fprintf(stderr, "(%s:%d) error reading serial port - %s\n", __FILE__, __LINE__, strerror(errno)); 
        return;
    }
    DEBUG_PRINT(("Read %i bytes. Received message: %s", num_bytes, read_buf));
    //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
    return;
}

int send_gsm_msg_last(char msg[], int serial_port) {
    DEBUG_PRINT(("Sending %s\n", msg));
    //printf("Sending %s\n", msg);
    char read_buf [256];
    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        fprintf(stderr, "(%s:%d) error reading serial port - %s\n", __FILE__, __LINE__, strerror(errno)); 
        return -2;
    }
    DEBUG_PRINT(("Read %i bytes. Received message: %s", num_bytes, read_buf));
    //printf("Read %i bytes. Received message: %s", num_bytes, read_buf);
    return process_pdu_return(read_buf, num_bytes);;
}

int process_signal(char msg[], int size) {
    char signal_strength[4];
    int signal_value = 0;

    for (int i = 0; i < size; i++) {
        if (msg[i] == '+' && msg[i + 1] == 'C' && msg[i + 2] == 'S' && 
            msg[i + 3] == 'Q' && msg[i + 4] == ':') {

            sprintf(signal_strength, "%c%c", msg[i + 6], msg[i + 7]);
            signal_value = atol(signal_strength);
            printf("Signal strength: %d\n", signal_value);

            if (signal_value < 1) {
                fprintf(stderr, "(%s:%d) error: no signal\n", __FILE__, __LINE__);
                return -1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

int get_signal(int serial_port) {
    char msg[] = "AT+CSQ\r";
    printf("Checking signal strength...\n");

    char read_buf [256];
    DEBUG_PRINT(("Sending %s\n", msg));
    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        fprintf(stderr, "(%s:%d) error reading serial port - %s\n", __FILE__, __LINE__, strerror(errno)); 
        return -1;
    }
    DEBUG_PRINT(("Read %i bytes. Received message: %s", num_bytes, read_buf));
    return process_signal(read_buf, num_bytes);
}

int setup_tty(int serial_port) {
    struct termios tty;

    if(tcgetattr(serial_port, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
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

    tty.c_cc[VTIME] = 1;    // Wait for up to 1s (10 deciseconds), returning as soon as 20 bits are received.
    tty.c_cc[VMIN] = 20;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    return 0;
}

char * process_pdu(char recipient[], char message[], int with_udh, char udh_data[], int alphabet) {

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
    iconv_close(cd);
    char *pdu_ptr = pdu;
    return pdu_ptr;
}

int send_sms(int serial_port, int is_last_message, void *push, char recipient[], char message[], int with_udh, char udh_data[], int alphabet) {
    
    char *pdu = process_pdu(recipient, message, with_udh, udh_data, alphabet);
    int length = (strlen(pdu) - 2) / 2;

    char cmd1[] = "AT+CMGF=0\r";
    char cmd2[11];
    char pdu_modified[strlen(pdu) + 3];
    char read_buf[256]; /* Weird behaviour when deleted even though compiler tells its unused */
    sprintf(cmd2, "AT+CMGS=%d\r", length);

    char *pdu_2 = process_pdu(recipient, message, with_udh, udh_data, alphabet);
    sprintf(pdu_modified, "%s\x1A\r", pdu_2);

    send_gsm_msg(cmd1, serial_port);
    send_gsm_msg(cmd2, serial_port);
    if(is_last_message) {
        if(send_gsm_msg_last(pdu_modified, serial_port) == 0) {
            printf("Message sent successfuly\n");
            s_send(push, "10");
            printf("ZMQ sent\n");
        } else {
            printf("Error sending message\n");
            s_send(push, "-10");
            printf("ZMQ sent\n");
        }
    } else {
        send_gsm_msg(pdu_modified, serial_port);
    }

    return 0;
}

int process_sms_data(int serial_port, char recipient[], char message[], void *push) {
    int ascii_flag = contains_non_ascii(message);
    int ref_no = rand() % 100;
    srand(time(0));
    int max_characters = 153;
    int alphabet = 0;
    int last_msg_flag = 1;
    if (ascii_flag != 0) {
        max_characters = 63;
        alphabet = 2;
    }

    char split_message[max_characters];
    memset(split_message, 0, sizeof(split_message));
    int divisions = strlen(message) / max_characters;
    divisions++; 
    if (divisions > 1) {
        for (int i = 0; i < divisions; i++) {
            int iterations = 0;
            for (int j = 0; j < strlen(message); j++) {
                if (iterations == max_characters) {
                    split_message[j] = '\0';
                    break;
                }
                iterations++;
                split_message[j] = message[max_characters * i + j];
            }
            char udh_data[18];
            sprintf(udh_data, "05 00 03 %02hhX %02hhX %02hhX", ref_no, divisions, i + 1);

            if(i == divisions - 1) {
                last_msg_flag = 1;
            } else {
                last_msg_flag = 0;
            }
            send_sms(serial_port, last_msg_flag, push, recipient, split_message, 1, udh_data, alphabet);
            memset(split_message,0,sizeof(split_message));
        }
    } else {
        send_sms(serial_port, 1, push, recipient, message, 0, "", alphabet);
    }
    return 0;
}

char * get_file_contents(char file_name[]) {
    char * buffer = 0;
    long length;
    FILE * f = fopen (file_name, "r");

    if (f == NULL) {
        fprintf(stderr, "(%s:%d) error reading file - %s\n", __FILE__, __LINE__, strerror(errno));
        exit(-1);
    }

    if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer) {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }

    return buffer;
}

cJSON * string_to_json(char *string) {
    cJSON *json_object = cJSON_Parse(string);
    if (json_object == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "(%s:%d) error parsing json before: %s\n", __FILE__, __LINE__, error_ptr);
            cJSON_Delete(json_object);
            free(string);
            exit(-1);
        }
        cJSON_Delete(json_object);
    }
    return json_object;
}

cJSON * get_json_object_item(cJSON *json_object, char *item_name) {
    cJSON *item;

    item = cJSON_GetObjectItemCaseSensitive(json_object, item_name);

    if (item == NULL) {
        fprintf(stderr, "(%s:%d) error getting json object [%s]\n", __FILE__, __LINE__, item_name);
        cJSON_Delete(item);
        cJSON_Delete(json_object);
        exit(1);
    }

    return item;
}

int main(int argc, char **argv) {
    //printf("%s\n", pdu);
    int timeout = 5000; /* 5 seconds */
    signal(SIGINT, handler);

    /* Time struct for printing out timestamp of information */
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    /* Reading json config file and parsing it into json object */
    char *config_string = get_file_contents(CONFIG_PATH);
    cJSON *config_json = string_to_json(config_string);
    free(config_string);

    /* Getting specified json objects */
    cJSON *push_address = get_json_object_item(config_json, "ZMQ_PUSH");
    cJSON *pull_address = get_json_object_item(config_json, "ZMQ_PULL");
    cJSON *serial_port_address = get_json_object_item(config_json, "SERIAL_PORT");

    /* Open the serial port and set it up */
    int serial_port = open(serial_port_address->valuestring, O_RDWR);
    if (serial_port == -1) {
        fprintf(stderr, "(%s:%d) error opening serial port - %s\n", __FILE__, __LINE__, strerror(errno));
        cJSON_Delete(config_json);
        exit(-1);
    }

    if (setup_tty(serial_port) == -1) {
        cJSON_Delete(config_json);
        exit(-1);
    }

    /* Create pull and push ZMQ sockets */
    void *context = zmq_ctx_new ();
    void *zmq_pull = zmq_socket (context, ZMQ_PULL);
    void *zmq_push = zmq_socket (context, ZMQ_PUSH);

    /* Binding push and pull sockets and checking for errors */
    int rc = zmq_bind (zmq_push, push_address->valuestring);
    if (rc == -1) {
        fprintf(stderr, "(%s:%d) error connecting to %s - %s\n",  __FILE__, __LINE__, 
            push_address->valuestring, strerror(errno));
        cJSON_Delete(config_json);
        goto end;
    }

    rc = zmq_bind (zmq_pull, pull_address->valuestring);
    if (rc == -1) {
        fprintf(stderr, "(%s:%d) error connecting to %s - %s\n",  __FILE__, __LINE__, 
            pull_address->valuestring, strerror(errno));
        cJSON_Delete(config_json);
        goto end;
    }

    /* Sockets are binded, we won't be needing these addresses anymore */
    cJSON_Delete(config_json);

    /* Setting push socket so it times out after a given time */
    zmq_setsockopt(zmq_push, ZMQ_SNDTIMEO, &timeout, sizeof(int));
    rc = get_signal(serial_port);
    if (rc == -1) {
        goto end;
    }

    printf("Ready, waiting for messages...\n");

    /* MAIN LOOP */
    while (!signal_received) {
        char *received_messsage_zmq = s_recv(zmq_pull);
        fflush(stdout);

        /* Setting timestamp */
        timer = time(NULL);
        tm_info = localtime(&timer);
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

        /* Parsing string to json object */
        cJSON *received_messsage_json = cJSON_Parse(received_messsage_zmq);
        if (received_messsage_json == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                rc = -1;
                fprintf(stderr, "(%s:%d) error parsing json before: %s\n", __FILE__, __LINE__, error_ptr);
                cJSON_Delete(received_messsage_json);
                goto end;
            }
        }

        /* Getting individual json object items */
        cJSON *recipient = cJSON_GetObjectItemCaseSensitive(received_messsage_json, "recipient");
        cJSON *message = cJSON_GetObjectItemCaseSensitive(received_messsage_json, "content");

        if(signal_received) {
            goto end;
        }

        if(recipient == NULL || message == NULL) {
            fprintf(stderr, "(%s:%d) error getting json object\n", __FILE__, __LINE__);
            cJSON_Delete(received_messsage_json);
            rc = -1;
            goto end;
        }
        char *recipient_modified = recipient->valuestring;
        recipient_modified++;

        printf("\n========================================================================\n");
        printf("[%s]\n", buffer);
        printf("Recipient: %s\n", recipient->valuestring);
        printf("Message: %s\n", message->valuestring);

        process_sms_data(serial_port, recipient_modified, message->valuestring, zmq_push);

        cJSON_Delete(received_messsage_json);
        free(received_messsage_zmq);
    }

    /* Finishing the program */
    end:
        close(serial_port);
        zmq_close(zmq_push);
        zmq_close(zmq_pull);
        zmq_ctx_destroy(context);
        printf("Safely finished\n");
        return rc;
}