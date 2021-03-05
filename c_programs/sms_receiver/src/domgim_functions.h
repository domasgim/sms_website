#ifndef DOMGIM_FUNCTIONS_H
# define DOMGIM_FUNCTIONS_H

/*
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

#include "SMS.h"
#include "utf.h"
#include "cJSON.h"
#include "zhelpers.h"
*/

#define DSC_to_msg(DSC) (DSC == 0 ? "Bit7" : (DSC == 1 ? "Bit8" : "UCS2"))

struct termios setup_tty(int serial_port);

char *create_monitor_with_helpers(char number[], char message[]);

void process_all_pdus(void *socket);

void process_CMGL(char msg[], int size, void *socket);

int process_signal(char msg[], int size);

void send_gsm_msg(unsigned char msg[], int *serial_port, void *socket);

void prepare_program();

#endif
