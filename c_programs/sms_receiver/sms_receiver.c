#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <time.h>

#include "src/SMS.h"
#include "src/utf.h"
#include "src/cJSON.h"
#include "src/zhelpers.h"
#include "src/pdu.h"
#include "src/list.h"
#include <x86_64-linux-gnu/curl/curl.h>
#include "sms_receiver.h"

#define DSC_to_msg(DSC) (DSC == 0 ? "Bit7" : (DSC == 1 ? "Bit8" : "UCS2"))

struct SMS_Struct sms_list[32];
int sms_list_size = 0;

uint8_t concateMsgRefNo_list[32]; 
int concateMsgRefNo_list_size = 0;
int concateMsgRefNo_list_size_completed = 0;

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};

size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;                             /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */

    /* expand buffer using a temporary pointer to avoid memory leaks */
    char * temp = realloc(p->payload, p->size + realsize + 1);

    /* check allocation */
    if (temp == NULL) {
      /* this isn't good */
      fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
      /* free buffer */
      free(p->payload);
      /* return */
      return 1;
    }

    /* assign payload */
    p->payload = temp;

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode;                   /* curl result code */

    /* init payload */
    fetch->payload = (char *) calloc(1, sizeof(fetch->payload));

    /* check payload */
    if (fetch->payload == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);

    /* set default user agent */
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 15);

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    /* fetch the url */
    rcode = curl_easy_perform(ch);

    /* return */
    return rcode;
}

void send_post_request(char json_message[]) {
    CURL *ch;                                               /* curl handle */
    CURLcode rcode;

    struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */
    struct curl_slist *headers = NULL;                      /* http headers to send with request */

    /* url to test site */
    char *url = "http://localhost:8000/api";

    /* init curl handle */
    if ((ch = curl_easy_init()) == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
        /* return error */
        return 1;
    }

    /* set content type */
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

     /* set curl options */
    curl_easy_setopt(ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(ch, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ch, CURLOPT_POSTFIELDS, json_message);

    /* fetch page and capture return code */
    rcode = curl_fetch_url(ch, url, cf);

    /* cleanup curl handle */
    curl_easy_cleanup(ch);

    /* free headers */
    curl_slist_free_all(headers);

    /* check return code */
    if (rcode != CURLE_OK || cf->size < 1) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
            url, curl_easy_strerror(rcode));
        /* return error */
        return 1;
    }

    /* check payload */
    if (cf->payload != NULL) {
        /* print result */
        //printf("CURL Returned: \n%s\n", cf->payload);
        /* free payload */
        free(cf->payload);
    } else {
        /* error */
        fprintf(stderr, "ERROR: Failed to populate payload");
        /* free payload */
        free(cf->payload);
        /* return */
        return 1;
    }

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

    /** Significant part. You can cusomise the response behaviour with these two fields **/
    tty.c_cc[VTIME] = 20;    // Wait for up to 2s (20 deciseconds), returning as soon as 128 bits are received.
    tty.c_cc[VMIN] = 128;

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

char *create_monitor_with_helpers(char sender_number[], char recipient_number[], char message[])
{
    char *string = NULL;
    cJSON *resolutions = NULL;
    size_t index = 0;

    cJSON *monitor = cJSON_CreateObject();

    if (cJSON_AddStringToObject(monitor, "sender_number", sender_number) == NULL) {
        goto end;
    }

    if (cJSON_AddStringToObject(monitor, "recipient_number", recipient_number) == NULL) {
        goto end;
    }

    if (cJSON_AddStringToObject(monitor, "message", message) == NULL) {
        goto end;
    }

    string = cJSON_Print(monitor);

    if (string == NULL)
    {
        printf("Failed to print monitor.\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}

void process_all_pdus(void *socket) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    char message[131072]; /* We give it a larger size so that we dont overflow it. The string could still overflow but only in a case of basically non-stop spam of long messages. */

    /** We start by iterating over all refrence numbers of the SMS message. In each iteration we search for the actual messages containing that refrence number 
     * and if a match is found, we strcat() the found message until we find the last part of the multipart SMS message. **/
    for (int i = 0; i < concateMsgRefNo_list_size; i++) {
        memset(message, 0, sizeof message);
        for(int j = 0; j < sms_list_size; j++) {

            /** Only copying the contents because we know there are more parts of the message of this refrence number **/
            if (concateMsgRefNo_list[i] == sms_list[j].concateMsgRefNo && sms_list[j].concateCurntPart < sms_list[j].concateTotalParts) {
                strcat(message, sms_list[j].UD);

                /** If the current message is the last part of the multipart SMS message, we copy it one last time to one big message, print out some info 
                 * of the SMS contents, create a JSON string of the number and message contents and send it over ZMQ **/
            } else if (concateMsgRefNo_list[i] == sms_list[j].concateMsgRefNo && sms_list[j].concateCurntPart == sms_list[j].concateTotalParts) {
                timer = time(NULL);
                tm_info = localtime(&timer);
                strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

                strcat(message, sms_list[j].UD);
                printf("\n========================================================================\n");
                printf("[%s]\n", buffer);

                printf("SMSC: %s\n", sms_list[j].SCA);
                printf("Sender number: %s\n", sms_list[j].OA);
                //printf("Timestamp: %s\n", sms_list[j].SCTS);
                //printf("Message: %s\n", message);
                //printf("Encoding： %s\n", DSC_to_msg(sms_list[j].DCS));

                /** Create a string containing JSON data of the message contents and the sender number and send it over ZeroMQ broker **/
                char *json_string = create_monitor_with_helpers(sms_list[i].OA, "+37065259965", message);
                send_post_request(json_string);

                //s_sendmore(socket, "SMS");
                //s_send(socket, json_string);

                //printf("JSON string sent...\n");
                //printf("JSON: %s\n", json_string);
                memset(message, 0, sizeof message);
                sleep(1);
            }
        }
    }

    /** This part is for searching non-multipart messages in the whole array. This wouldn't happen often though. **/
    for (int k = 0; k < sms_list_size; k++) {
        if (sms_list[k].concateTotalParts == 0) {
            timer = time(NULL);
            tm_info = localtime(&timer);
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

            printf("\n========================================================================\n");
            printf("[%s]\n", buffer);

            printf("SMSC: %s\n", sms_list[k].SCA);
            printf("Sender number: %s\n", sms_list[k].OA);
            //printf("Timestamp: %s\n", sms_list[k].SCTS);
            //printf("Message: %s\n", sms_list[k].UD);
            //printf("Encoding： %s\n", DSC_to_msg(sms_list[k].DCS));

            char *json_string = create_monitor_with_helpers(sms_list[k].OA, "+37065259965", sms_list[k].UD);
            send_post_request(json_string);

            //s_sendmore(socket, "SMS");
            //s_send(socket, json_string);
            sleep(1); 
        }
    }
}

void process_CMGL(char msg[], int size, void *socket) {

    /** This is a bit complicated only because the problem of receiving SMS messages from the modem in such fashion is 
     * complicated to begin with. First off, everything we receive is in a string format from the modem. We issue 
     * the command AT+CMGL=0 to receive all of unread messages and get one huge string. SMS messages can only contain
     * about 170 characters using simple symbols (no unicode stuff) and up to 70 if we include unicode characters so 
     * if a SMS message contains 700 characters it can be split into about 10 separate pieces and we need to process it.
     * A new problem is that the modem doesn't receive those 10 separate SMS message in one atomic action. It receives them one
     * by one that can take up some time. Another problem is that we send out the request to read all unread messages every 
     * few seconds so the modem can be receiving a huge multipart message and the program will read only a the first part
     * or the second part of the returned string. To add up to the whole mess is if two or more people (say person 1 and 2) send out multipart
     * SMS messages simulteniously, the modem can receive the messages like that: 1 1 2 1 2 1 2 so we need to diferentiate that too. 
     * Fortunately, we can extract useful data about the SMS message in PDU format like the unique-ish number of the 
     * multipart message, total parts of a specific multipart message and the current part of the multipart message. We 
     * will be using this data to solve all of these problems. **/

    /** I use these as flags to determine when we are reading a PDU message. **/
    int contains_header = 0;
    int contains_pdu = 0;
    int contains_refNo = 0;
    struct SMS_Struct sms;

    char pdu[2048];

    memset(pdu, 0, sizeof pdu);

    /** We go though each character in the return string and check if there is a line containing 
     * CMGL\: it tells us that a new line will contain a SMS message in PDU format. We need to keep in mind that we can receive
     * multiple messages in such format so we will be searching for multiple CMGL matches in the string **/
    for (int i = 0; i < size; i++) {
        if (msg[i] == 'C' && msg[i + 1] == 'M' && msg[i + 2] == 'G' && 
            msg[i + 3] == 'L' && msg[i + 4] == ':') {
                contains_header = 1;
                }
        
        if (contains_header == 1 && msg[i] == '\n') {
            contains_header = 0;
            contains_pdu = 1;
            i++;
        }

        /** If we are inside the PDU message, we copy each character to a string **/
        if (contains_pdu == 1) {
            strncat(pdu, &msg[i], 1);
        }

        /** If we reach the end of PDU message, we process it **/
        if (contains_pdu == 1 && msg[i] == '\n') {
            contains_pdu = 0;
            contains_refNo = 0;
            sms = PDUDecoding(pdu);

            sms_list[sms_list_size] = sms;
            sms_list_size++;

            /** All of this till the end is the actual solving part of multiple multi-part messages. We basically 
             * write a new array containing all of the refrence numbers of multipart messages which will be used in 
             * stiching up the message contents. **/
            for (int j = 0; j < concateMsgRefNo_list_size; j++) {
                if (concateMsgRefNo_list[j] == sms.concateMsgRefNo && sms.concateMsgRefNo != 0) {
                    contains_refNo = 1;
                }
            }

            if (contains_refNo == 0 && sms.concateTotalParts != 0) {
                concateMsgRefNo_list[concateMsgRefNo_list_size] = sms.concateMsgRefNo;
                concateMsgRefNo_list_size++;
            }

            if (sms.concateTotalParts == sms.concateCurntPart && sms.concateTotalParts != 0) {
                concateMsgRefNo_list_size_completed++;
            }

            memset(pdu, 0, sizeof pdu);
            i++;
        }
    }
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

void send_gsm_msg(unsigned char msg[], int *serial_port, void *socket) {
    char read_buf [65536];
    int signal_flag;

    write(serial_port, msg, strlen(msg));
    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));
    if (num_bytes < 0) {
        printf("Error reading: %s", strerror(errno));
        return;
    }
    //printf("Read %i bytes. Received message: \n%s\n", num_bytes, read_buf);
    process_signal(read_buf, num_bytes);
    process_CMGL(read_buf, num_bytes, socket);
    return;
}

void prepare_program() {
    int serial_port = open("/dev/ttyUSB2", O_RDWR);
    setup_tty(serial_port);

    unsigned char msg0[] = "AT+CSQ\r";
    unsigned char msg1[] = "AT+CNMI=2,1,0,0,0\r";
    unsigned char msg2[] = "AT+CMGF=0\r";
    unsigned char msg4[] = "AT+CMGD=1,4\r";

    printf("Checking signal strength...\n");
    send_gsm_msg(msg0, serial_port, NULL);

    printf("Initialising...\n");
    send_gsm_msg(msg1, serial_port, NULL);
    send_gsm_msg(msg2, serial_port, NULL);
    send_gsm_msg(msg4, serial_port, NULL);

    printf("Receiver ready, waiting for messages...\n");

    close(serial_port);
}

int main() {
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    zmq_bind (publisher, "tcp://*:5563");

    prepare_program();

    while (true) {
        int serial_port = open("/dev/ttyUSB2", O_RDWR);
        unsigned char msg3[] = "AT+CMGL=0\r";

        setup_tty(serial_port);
        send_gsm_msg(msg3, serial_port, publisher);
        close(serial_port);

        if (concateMsgRefNo_list_size == concateMsgRefNo_list_size_completed) {
            process_all_pdus(publisher);
            sms_list_size = 0;
            concateMsgRefNo_list_size = 0;
            concateMsgRefNo_list_size_completed = 0;

            memset(sms_list, 0, sizeof(sms_list));
            memset(concateMsgRefNo_list, 0, sizeof(concateMsgRefNo_list));
        }
        sleep(5);
    }
    return 0;
}
