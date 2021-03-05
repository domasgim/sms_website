#ifndef SMS_SENDER_H
#define SMS_SENDER_H

/**
 * @brief Send out an AT command to serial port
 * 
 * @param msg Command
 * @param serial_port Serial port file descriptor
 */
void send_gsm_msg(unsigned char msg[], int *serial_port);

/**
 * @brief Set the up tty object
 * 
 * @param serial_port Serial port file descriptor
 * @return struct termios 
 */
struct termios setup_tty(int serial_port);

/**
 * @brief Check if a string contains non ASCII characters
 * 
 * @param message Input string
 * @return int 0 - true; 1 - false;
 */
int containsNonASCII(char message[]);

/**
 * @brief Create a PDU message
 * 
 * @param recipient Recipient's phone number (must not contains '+' sign)
 * @param message SMS message string
 * @param with_udh UDH flag (used in sending long multipart messages)
 * @param udh_data UDH data string
 * @param alphabet PDU encoding (0 - 7bits; 1 - 8bits; 2 - 16bits)
 * @param is_last_message Used to check serial port response (used to get a response on a web server)
 * @param push ZMQ push socket
 * @return int 
 */
int process_pdu(char recipient[], char message[], int with_udh, char udh_data[], int alphabet, int is_last_message, void *push);

/**
 * @brief Process SMS request. Used to determine PDU generation parameters.
 * If the message contains unicode characters, the encoding will be 16 bits, otherwise 7 bits.
 * If the message contains more than 153 or 63 characters depending if the message will be 7 or 16
 * bits, the program will split the message strings to even parts and generate multiple PDUs 
 * containing appropriate UDH data.
 * 
 * @param recipient Recipient's phone number
 * @param message SMS message string
 * @param push ZMQ push socket
 * @return int 
 */
int process_sms_data(char recipient[], char message[], void *push);

/**
 * @brief Read the return value of a serial port and determine if SMS has been sent or not.
 * 
 * @param msg Return value from a serial port
 * @param size Size of msg string
 * @param push ZMQ push socket, used to send a message to a web browser
 */
void process_pdu_return(char msg[], int size, void *push);

/**
 * @brief Send out PDU message through a serial port
 * 
 * @param pdu PDU string
 * @param is_last_message Used to check serial port response (used to get a response on a web server)
 * @param push ZMQ push socket
 */
void send_sms(char pdu[], int is_last_message, void *push);

/**
 * @brief Process a return value from a serial port. Determines the signal strength.
 * 
 * @param msg Return value from a serial port
 * @param size Size of msg string
 * @return int 
 */
int process_signal(char msg[], int size);

/**
 * @brief Check if the modem has signal using AT commands
 * 
 */
void check_signal();

#endif //SMS_SENDER_H