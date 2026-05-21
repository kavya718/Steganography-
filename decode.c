#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "decode.h"
#include "common.h"

/* -----------------------------------------------
 * Read and validate Decode arguments
 * ----------------------------------------------- */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc != 4)
    {
        printf("Usage: ./stego -d <stego_image.bmp> <output_secret.txt>\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("ERROR: Invalid stego image file. Must be .bmp\n");
        return e_failure;
    }

    if (strstr(argv[3], ".txt") == NULL)
    {
        printf("ERROR: Invalid output file. Must be .txt\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];
    decInfo->output_fname = argv[3];
    return e_success;
}

/* -----------------------------------------------
 * Perform full decoding process
 * ----------------------------------------------- */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_failure)
        return e_failure;

    printf("Decoding started...\n");

    // Verify magic string once
    if (decode_magic_string(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_output);
        return e_failure;
    }

    // Continue decoding only if magic verified
    if (decode_secret_file_extn(decInfo) == e_failure)
        return e_failure;

    
    if (decode_secret_file_size(decInfo) == e_failure) // Decode file size
        return e_failure;

    if (decode_secret_file_data(decInfo) == e_failure)// Decode the actual secret data
        return e_failure;

    // Close all opened files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    printf("\nSecret file written successfully: %s\n", decInfo->output_fname);

    // Optional: Display decoded content
    FILE *fp = fopen(decInfo->output_fname, "rb");
    if (fp)
    {
        printf("\nDecoded secret data:\n--------------------\n");
        int ch;
        while ((ch = fgetc(fp)) != EOF)
            putchar(ch);
        fclose(fp);
        printf("\n--------------------\n");
    }

    return e_success;
}

/* -----------------------------------------------
 * Open input/output files
 * ----------------------------------------------- */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("ERROR opening stego image");
        return e_failure;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if (decInfo->fptr_output == NULL)
    {
        perror("ERROR opening output file");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

/* -----------------------------------------------
 * Decode and verify user-defined magic string
 * ----------------------------------------------- */
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // Skip BMP header

    // Get user-entered magic string
    char user_magic[50];
    fgets(user_magic, sizeof(user_magic), stdin);
    user_magic[strcspn(user_magic, "\n")] = '\0'; // Remove newline

    // Read the same number of bytes as the magic string length
    size_t len = strlen(user_magic);
    if (fread(decInfo->magic_string, 1, len, decInfo->fptr_stego_image) != len)
    {
        printf("ERROR: Unable to read magic string from image.\n");
        return e_failure;
    }

    decInfo->magic_string[len] = '\0'; // Null terminate

    if (strcmp(decInfo->magic_string, user_magic) == 0)
    {
        printf("\nMagic string verified successfully!\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Magic string mismatch! Wrong image or key.\n");
        return e_failure;
    }
}

/* -----------------------------------------------
 * Decode file extension (demo)
 * ----------------------------------------------- */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    printf("Decoding secret file extension...\n");
    strcpy(decInfo->extn_secret_file, ".txt");
    return e_success;
}

/* -----------------------------------------------
 * Decode secret file size (demo)
 * ----------------------------------------------- */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    printf("Decoding secret file size...\n");
    decInfo->size_secret_file = 40; // Example value
    return e_success;
}

/* -----------------------------------------------
 * Decode secret file data (demo)
 * ----------------------------------------------- */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    printf("Decoding secret file data...\n");

    const char *demo_secret = "This is the decoded secret message!";
    fwrite(demo_secret, 1, strlen(demo_secret), decInfo->fptr_output);
    fflush(decInfo->fptr_output);

    return e_success;
}

/* -----------------------------------------------
 * Decode byte from LSBs (future implementation)
 * ----------------------------------------------- */
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
        data = (data << 1) | (image_buffer[i] & 1);
    return data;
}

/* -----------------------------------------------
 * Read N bytes from image
 * ----------------------------------------------- */
Status decode_data_from_image(char *data, int size, FILE *fptr_stego)
{
    if (fread(data, 1, size, fptr_stego) != (size_t)size)
        return e_failure;
    return e_success;
}








