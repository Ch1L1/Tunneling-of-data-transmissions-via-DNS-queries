/***************************************************************************************
*    Title: Base64 Encoding Program
*    Author:  M. Saqib
*    Date: Jun 22, 2021
*    Availability: https://www.mycplus.com/source-code/c-source-code/base64-encode-decode/
*
***************************************************************************************/
#ifndef _BASE64_H
#define _BASE64_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
                                
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table();

void base64_cleanup();

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);

#endif