#ifndef SMS_SENDER_H
#define SMS_SENDER_H

#include "../include/cJSON.h"

/**
 * @brief Signal handler. Used to stop the program and safely exit.
 * 
 */
void handler();

/**
 * @brief Read the return value of a serial port and determine if SMS has been sent or not.
 * 
 * @param msg Return value from a serial port
 * @param size Size of msg string
 * @param push ZMQ push socket, used to send a message to a web browser
 * @return int 0 for success, -1 for error
 */
int process_pdu_return(char msg[], int size);

/**
 * @brief Check if a string contains non ASCII characters
 * 
 * @param message Input string
 * @return int 0 - true; 1 - false;
 */
int contains_non_ascii(char message[]);

/**
 * @brief Send out an AT command to serial port
 * 
 * @param msg Command
 * @param serial_port Serial port file descriptor
 */
int send_gsm_msg(char msg[], int serial_port);

/**
 * @brief Send out a PDU code through a serial port and process it
 * 
 * @param msg PDU code
 * @param serial_port Serial port file descriptor
 * @return int 0 for success, -1 for error while sending SMS, -2 for error with file descriptor
 */
int send_gsm_msg_last(char msg[], int serial_port);

/**
 * @brief Process signal strength information returned by the modem if there is any information regarding the signal strength.
 * 
 * @param msg Return message from the modem.
 * @param size Size of the returned message.
 * @return int Returns 0 or -1 depending if there is a signal or not.
 */
int process_signal(char msg[], int size);

/**
 * @brief Get signal strength
 * 
 * @param serial_port Opened serial port descriptor of the connected modem (usually /dev/ttyUSB[0-5])
 * @return int 0 for success, otherwise -1
 */
int get_signal(int serial_port);

/**
 * @brief Set the up tty object
 * 
 * @param serial_port Serial port file descriptor
 * @return int 0 for success, otherwise -1
 */
int setup_tty(int serial_port);

/**
 * @brief Create a PDU message
 * 
 * @param recipient Recipient's phone number (must not contains '+' sign)
 * @param message SMS message string
 * @param with_udh UDH flag (used in sending long multipart messages)
 * @param udh_data UDH data string
 * @param alphabet PDU encoding (0 - 7bits; 1 - 8bits; 2 - 16bits)
 * @return char* PDU string
 */
char * process_pdu(char recipient[], char message[], int with_udh, char udh_data[], int alphabet);

/**
 * @brief Send out PDU message through a serial port
 * 
 * @param serial_port Serial port file descriptor
 * @param is_last_message Used to check serial port response (used to get a response on a web server)
 * @param push ZMQ push socket
 * @param recipient Recipient's phone number
 * @param message SMS message string
 * @param with_udh UDH flag (used in sending long multipart messages)
 * @param udh_data UDH data string
 * @param alphabet PDU encoding (0 - 7bits; 1 - 8bits; 2 - 16bits)
 * @return int 
 */
int send_sms(int serial_port, int is_last_message, void *push, char recipient[], char message[], int with_udh, char udh_data[], int alphabet);

/**
 * @brief Process SMS request. Determines PDU generation parameters 
 * (single/multipart message and 7/16bit encoding)
 * 
 * @param serial_port Serial port file descriptor
 * @param recipient Recipient's phone number
 * @param message SMS message string
 * @param push ZMQ push socket
 * @return int 
 */
int process_sms_data(int serial_port, char recipient[], char message[], void *push);

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

#endif