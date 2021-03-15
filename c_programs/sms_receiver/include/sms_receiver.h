#ifndef SMS_RECEIVER_H
#define SMS_RECEIVER_H

#include "../include/cJSON.h"

/**
 * @brief Signal handler. Used to stop the program and safely exit.
 * 
 */
void handler();

/**
 * @brief Process the return string of a modem if the return string contains information about SMS messages in PDU format.
 * A bit of a mess but it works at least.
 * 
 * @param msg Return message from the modem.
 * @param size Size of the returned message.
 */
void process_CMGL(char msg[], int size);

/**
 * @brief Create a JSON string containing data for SMS message
 * 
 * @param sender_number Sender's phone number
 * @param recipient_number Recipient's phone number
 * @param message Message string
 * @return char* JSON string
 */
char *create_json_object(char sender_number[], char recipient_number[], char message[]);

/**
 * @brief Process the information of all PDU messages inside of an array.
 * 
 * @param curl_address URL address used to send out a request to the website
 * @param recipient_number GSM modem's number (the number of the device which received the number)
 */
void process_all_pdus(char curl_address[], char recipient_number[]);

/**
 * @brief Send an AT command to the modem.
 * 
 * @param msg AT command. Must be terminated with \\r\.
 * @param serial_port Opened serial port descriptor of the connected modem (usually /dev/ttyUSB[0-5])
 */
void send_gsm_msg(char msg[], int serial_port);

/**
 * @brief Send a post request to a web server
 * 
 * @param json_message JSON string to be sent over curl
 * @param url URL address for the request to be sent
 */
void send_post_request(char json_message[], char *url);

/**
 * @brief Prepare the modem for processing the messages in PDU format and deletes all pre-existing messages inside the modem.
 * 
 * @param serial_port Opened serial port descriptor of the connected modem (usually /dev/ttyUSB[0-5])
 */
void prepare_modem(int serial_port);

/**
 * @brief Process signal strength information returned by the modem if there is any information regarding the signal strength.
 * 
 * @param msg Return message from the modem.
 * @param size Size of the returned message.
 * @return int Returns 0 or -1 depending if there is a signal or not.
 */
int process_signal(char msg[], int size);

/**
 * @brief Check if the modem has signal
 * 
 * @param serial_port Opened serial port descriptor of the connected modem (usually /dev/ttyUSB[0-5])
 * @return int 0 for success, otherwise -1
 */
int check_signal(int serial_port);

/**
 * @brief Set the up tty object
 * 
 * @param serial_port Serial port file descriptor
 * @return int 0 for success, otherwise -1
 */
int setup_tty(int serial_port);

/**
 * @brief Read file contents
 * 
 * @param file_name File name
 * @return char* Contents of the whole file
 */
char * get_file_contents(char file_name[]);

/**
 * @brief Convert a json string to json object
 * 
 * @param string json string
 * @return cJSON* json object
 */
cJSON * string_to_json(char *string);

/**
 * @brief Get a specific value of json object
 * 
 * @param json_object
 * @param item_name 
 * @return cJSON* json object containing a single value
 */
cJSON * get_json_object_item(cJSON *json_object, char *item_name);

/**
 * @brief Send a message through ZMQ socket
 * 
 * @param socket ZMQ socket
 * @param string Message
 * @return int 
 */
int s_send (void *socket, char *string);

/**
 * @brief Receive 0MQ string from socket and convert into C string. 
 * Caller must free returned string. Returns NULL if the context is being terminated.
 * 
 * @param socket ZMQ socket
 * @return char* returned message
 */
char * s_recv (void *socket);

#endif