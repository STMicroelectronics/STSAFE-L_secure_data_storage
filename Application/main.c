/**
 ******************************************************************************
 * \file    		main.c
 * \author  		https://github.com/Grom-
 ******************************************************************************
 *           			COPYRIGHT 2022 STMicroelectronics
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "Drivers/rng/rng.h"
#include "Drivers/uart/uart.h"
#include "stselib.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Defines -------------------------------------------------------------------*/
#define PRINT_RESET "\x1B[0m"
#define PRINT_CLEAR_SCREEN "\x1B[1;1H\x1B[2J"
#define PRINT_RED "\x1B[31m"   /* Red */
#define PRINT_GREEN "\x1B[32m" /* Green */

#define READ_BUFFER_SIZE 16
#define RANDOM_SIZE 16
#define ZONE_INDEX 32U

/* STDIO redirect */
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar()
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE {
    uart_putc(ch);
    return ch;
}
GETCHAR_PROTOTYPE {
    return uart_getc();
}

void apps_terminal_init(uint32_t baudrate) {
    (void)baudrate;
    /* - Initialize UART for example output log (baud 115200)  */
    uart_init(115200);
    /* Disable I/O buffering for STDOUT stream*/
    setvbuf(stdout, NULL, _IONBF, 0);
    /* - Clear terminal */
    printf(PRINT_RESET PRINT_CLEAR_SCREEN);
}

void apps_print_hex_buffer(uint8_t *buffer, uint16_t buffer_size) {
    uint16_t i;
    for (i = 0; i < buffer_size; i++) {
        if (i % 16 == 0) {
            printf(" \n\r ");
        }
        printf(" 0x%02X", buffer[i]);
    }
}

void apps_randomize_buffer(uint8_t *pBuffer, uint16_t buffer_length) {
    for (uint16_t i = 0; i < buffer_length; i++) {
        *(pBuffer + i) = (rng_generate_random_number() & 0xFF);
    }
}

int main(void) {
    stse_ReturnCode_t stse_ret = STSE_API_INVALID_PARAMETER;
    stse_Handler_t stse_handler;
    uint8_t readBuffer[READ_BUFFER_SIZE];
    uint8_t random[RANDOM_SIZE];
    uint32_t counter_value;

    /* - Initialize Terminal */
    apps_terminal_init(115200);

    /* - Print Example instruction on terminal */
    printf(PRINT_CLEAR_SCREEN);
    printf("----------------------------------------------------------------------------------------------------------------");
    printf("\n\r-                            STSAFE-L010 secure data storage counter access example                            -");
    printf("\n\r----------------------------------------------------------------------------------------------------------------");
    printf("\n\r-                                                                                                              -");
    printf("\n\r- description :                                                                                                -");
    printf("\n\r- This examples illustrates how to makes use of the STSAFE-L data storage APIs by performing following         -");
    printf("\n\r- accesses/commands to the target STSAFE device                                                                -");
    printf("\n\r-          o Query STSAFE-L total partition count                                                              -");
    printf("\n\r-          o Query STSAFE-L partitions information                                                             -");
    printf("\n\r-          o Read STSAFE-L zone ZONE_INDEX counter                                                             -");
    printf("\n\r-          o decrement STSAFE-L zone ZONE_INDEX counter                                                        -");
    printf("\n\r-          o Update STSAFE-L counter zone ZONE_INDEX                                                           -");
    printf("\n\r-                                                                                                              -");
    printf("\n\r- Note : zone IDs used in this example are aligned with STSAFE-L010 personalization                            -");
    printf("\n\r-        Accesses parameters must be adapted for other device personalization                                  -");
    printf("\n\r-                                                                                                              -");
    printf("\n\r----------------------------------------------------------------------------------------------------------------");

    /* ## Initialize STSAFE-L010 device handler */
    stse_ret = stse_set_default_handler_value(&stse_handler);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\r ## stse_set_default_handler_value ERROR : 0x%04X\n\r", stse_ret);
        while (1)
            ;
    }

    stse_handler.device_type = STSAFE_L010;
    stse_handler.io.Devaddr = 0x0C;
    stse_handler.io.busID = 1; // Needed to use expansion board I2C

    printf("\n\r - Initialize target STSAFE-L010");
    stse_ret = stse_init(&stse_handler);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\r ## stse_init ERROR : 0x%04X\n\r", stse_ret);
        while (1)
            ; // infinite loop
    }

    /* ## Read zone ZONE_INDEX (counter zone) */
    stse_ret = stse_data_storage_read_counter_zone(
        &stse_handler,      /* SE handler		*/
        ZONE_INDEX,         /* Zone index		*/
        0x0000,             /* Read Offset		*/
        readBuffer,         /* Read buffer		*/
        sizeof(readBuffer), /* Read length		*/
        04,                 /* Read chunk size	*/
        &counter_value,     /* Counter Value	*/
        STSE_NO_PROT);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### stse_data_storage_read_data_zone : ERROR 0x%04X", stse_ret);
        while (1)
            ; // infinite loop
    } else {
        printf("\n\n\r - stse_data_storage_read_data_zone (zone : %d - length : %d - counter : %lu)", ZONE_INDEX, sizeof(readBuffer) / sizeof(readBuffer[0]), counter_value);
        apps_print_hex_buffer(readBuffer, sizeof(readBuffer));
    }

    /*## Generate random number */
    apps_randomize_buffer(random, sizeof(random));

    /* ## Decrement zone ZONE_INDEX counter and store Randomized Associated data */
    stse_ret = stse_data_storage_decrement_counter_zone(
        &stse_handler,  /* SE handler 			*/
        ZONE_INDEX,     /* Zone index 			*/
        1,              /* Decrement amount		*/
        0x0000,         /* Update Offset 		*/
        random,         /* Update input buffer 	*/
        sizeof(random), /* Update Length 		*/
        &counter_value, /* Counter value		*/
        STSE_NO_PROT);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### stse_data_storage_decrement_counter_zone : ERROR 0x%04X", stse_ret);
        while (1)
            ; // infinite loop
    } else {
        printf("\n\n\r - stse_data_storage_decrement_counter_zone (zone = %d - length = %d - New counter : %lu)", ZONE_INDEX, sizeof(random) / sizeof(random[0]), counter_value);
        apps_print_hex_buffer(random, sizeof(random));
    }

    /* ## Read Zone ZONE_INDEX (counter zone) */
    stse_ret = stse_data_storage_read_counter_zone(
        &stse_handler,      /* SE handler		*/
        ZONE_INDEX,         /* Zone index		*/
        0x0000,             /* Read Offset		*/
        readBuffer,         /* Read buffer		*/
        sizeof(readBuffer), /* Read length		*/
        04,                 /* Read chunk size	*/
        &counter_value,     /* Counter Value	*/
        STSE_NO_PROT);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### stse_data_storage_read_data_zone : ERROR 0x%04X", stse_ret);
    } else {
        printf(PRINT_GREEN "\n\n\r - stse_data_storage_read_data_zone (zone : %d - length : %d - counter : %lu)", ZONE_INDEX, sizeof(readBuffer) / sizeof(readBuffer[0]), counter_value);
        apps_print_hex_buffer(readBuffer, sizeof(readBuffer));
    }

    printf(PRINT_RESET "\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

    while (1)
        ; // infinite loop

    return 0;
}
