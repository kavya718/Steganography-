#ifndef DECODE_H
#define DECODE_H

#include "types.h" // user defined Status, OperationType, etc.

/* 
 * Structure to store all information required for
 * decoding secret data from the stego image
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)

// Only define MAX_FILE_SUFFIX if it's not already defined
#ifndef MAX_FILE_SUFFIX
#define MAX_FILE_SUFFIX 8
#endif

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    uint image_capacity;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /* Output Secret File info */
    char *output_fname;
    FILE *fptr_output;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;
    char secret_data[MAX_SECRET_BUF_SIZE];

    /* Extracted Magic String */
    char magic_string[50];  // stores the decoded magic string for verification

} DecodeInfo;


/* -------- Function Prototypes -------- */

/* Read and validate Decode args */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from LSBs of image buffer */
char decode_byte_from_lsb(char *image_buffer);

/* Decode data from image */
Status decode_data_from_image(char *data, int size, FILE *fptr_stego);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

#endif






