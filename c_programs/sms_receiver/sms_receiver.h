#ifndef SMS_RECEIVER_H
#define SMS_RECEIVER_H

/**
 * @brief Send a post request to a web server
 * 
 * @param json_message 
 */
void send_post_request(char json_message[]);

/**
 * @brief Set the up tty object
 * 
 * @param serial_port Serial port file descriptor
 * @return struct termios 
 */
struct termios setup_tty(int serial_port);

/**
 * @brief Create a JSON string containing data for SMS message
 * 
 * @param sender_number Sender's phone number
 * @param recipient_number Recipient's phone number
 * @param message Message string
 * @return char* JSON string
 */
char *create_monitor_with_helpers(char sender_number[], char recipient_number[], char message[]);

/**
 * @brief Process the information of all PDU messages inside of an array.
 * 
 * @param socket Defined ZeroMQ socket. Only used if the program detects that modem returns SMS message in PDU format.
 */
void process_all_pdus(void *socket);

/**
 * @brief Process the return string of a modem if the return string contains information about SMS messages in PDU format.
 * A bit of a mess but it works at least.
 * 
 * @param msg Return message from the modem.
 * @param size Size of the returned message.
 * @param socket Defined ZeroMQ socket. Only used if the program detects that modem returns SMS message in PDU format.
 */
void process_CMGL(char msg[], int size, void *socket);

/**
 * @brief Process signal strength information returned by the modem if there is any information regarding the signal strength.
 * 
 * @param msg Return message from the modem.
 * @param size Size of the returned message.
 * @return int Returns 0 or 1 depending if there is a signal or not.
 */
int process_signal(char msg[], int size);

/**
 * @brief Send an AT command to the modem.
 * 
 * @param msg AT command. Must be terminated with \\r\.
 * @param serial_port Opened serial port descriptor of the connected modem (usually /dev/ttyUSB[0-5])
 * @param socket Defined ZeroMQ socket. Only used if the program detects that modem returns SMS message in PDU format.
 */
void send_gsm_msg(unsigned char msg[], int *serial_port, void *socket);

/**
 * @brief Prepare the modem for processing the messages in PDU format and deletes all pre-existing messages inside the modem.
 * 
 */
void prepare_program();

#endif