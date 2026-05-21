#include <stdio.h>      // Standard I/O operations
#include <stdlib.h>     // For file handling and memory functions
#include <string.h>     // For string manipulation
#include "types.h"      // Custom type definitions (e_success, e_failure, etc.)
#include "encode.h"     // Function prototypes for encoding

typedef unsigned int uint;   // Define unsigned int as uint for readability
#define MAX_EXTN_SIZE 8      // Maximum size for file extension

/* Read and validate input arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (argv[2] == NULL || argv[3] == NULL)
    {
        printf("ERROR: Missing arguments.\n");
        printf("Usage: ./a.out -e <input.bmp> <secret.txt> <output.bmp>\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp"))
        encInfo->src_image_fname = argv[2];
    else
    {
        printf("ERROR: Source image must be .bmp\n");
        return e_failure;
    }

    if (strstr(argv[3], ".txt"))
        encInfo->secret_fname = argv[3];
    else
    {
        printf("ERROR: Secret file must be .txt\n");
        return e_failure;
    }

    encInfo->stego_image_fname = (argv[4] != NULL) ? argv[4] : "stego.bmp";

    FILE *f1 = fopen(encInfo->src_image_fname, "rb");
    if (!f1)
    {
        printf("ERROR: Cannot open %s\n", encInfo->src_image_fname);
        return e_failure;
    }
    fclose(f1);

    FILE *f2 = fopen(encInfo->secret_fname, "rb");
    if (!f2)
    {
        printf("ERROR: Cannot open %s\n", encInfo->secret_fname);
        return e_failure;
    }
    fclose(f2);

    return e_success;
}

/* Open required files */
Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (!encInfo->fptr_src_image)
    {
        printf("ERROR: Unable to open source image\n");
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (!encInfo->fptr_secret)
    {
        printf("ERROR: Unable to open secret file\n");
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (!encInfo->fptr_stego_image)
    {
        printf("ERROR: Unable to create stego image\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        return e_failure;
    }

    printf("Files opened successfully.\n");
    return e_success;
}

/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;
}

/* Get BMP image size (capacity) */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width = 0, height = 0;
    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(int), 1, fptr_image);
    fread(&height, sizeof(int), 1, fptr_image);
    rewind(fptr_image);
    return width * height * 3; 
}

/* Check image capacity for encoding */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    /* required bits = (magic + extn + size + data) * 8 bits per byte */
    int required_bytes = (strlen(encInfo->user_magic_string) + MAX_EXTN_SIZE + sizeof(long) + encInfo->size_secret_file) * 8;

    if (encInfo->image_capacity >= (unsigned int)required_bytes)
    {
        printf("Capacity sufficient.\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Image too small to hold the secret data.\n");
        return e_failure;
    }
}

/* Copy 54-byte BMP header */
Status copy_bmp_header(FILE *fptr_src, FILE *fptr_dest)
{
    char header[54];
    if (fread(header, 1, 54, fptr_src) != 54)
        return e_failure;
    if (fwrite(header, 1, 54, fptr_dest) != 54)
        return e_failure;
    return e_success;
}

/* Encode a single byte into 8 LSBs (buffer of 8 image bytes) */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7 - i)) & 1);
    return e_success;
}

/* Encode data into image bytes (reads image bytes and writes modified bytes) */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src, FILE *fptr_dest)
{
    char image_byte;
    for (int i = 0; i < size; i++)
    {
        for (int bit = 7; bit >= 0; bit--)
        {
            if (fread(&image_byte, 1, 1, fptr_src) != 1)
                return e_failure;
            image_byte = (image_byte & 0xFE) | ((data[i] >> bit) & 1);
            if (fwrite(&image_byte, 1, 1, fptr_dest) != 1)
                return e_failure;
        }
    }
    return e_success;
}

/* Encode user-defined magic string (uses encInfo->user_magic_string if set) */
Status encode_magic_string(EncodeInfo *encInfo)
{
    if (encInfo->user_magic_string[0] == '\0')
    {
        /* only prompt if caller didn't already set it (main normally sets it) */
        printf("Enter a magic string (for decoding verification): ");
        /* use fgets to allow spaces and avoid leftover newline issues */
        if (fgets(encInfo->user_magic_string, sizeof(encInfo->user_magic_string), stdin) == NULL)
            return e_failure;
        encInfo->user_magic_string[strcspn(encInfo->user_magic_string, "\n")] = '\0';
    }

    return encode_data_to_image(encInfo->user_magic_string,
                                (int)strlen(encInfo->user_magic_string),
                                encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

/* Encode file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    return encode_data_to_image(file_extn, (int)strlen(file_extn),
                                encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

/* Encode secret file size (store as 4-byte int) */
Status encode_secret_file_size(long size, EncodeInfo *encInfo)
{
    int file_size = (int)size;
    if (encode_data_to_image((const char *)&file_size, sizeof(int),
                             encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("Encoded secret file size: %d bytes\n", file_size);
        return e_success;
    }
    else
    {
        printf("ERROR: Failed to encode file size\n");
        return e_failure;
    }
}

/* Encode actual secret data (byte-by-byte using buffer of 8 image bytes) */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch, buffer[8];
    rewind(encInfo->fptr_secret);
    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;
        encode_byte_to_lsb(ch, buffer);
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;
    }
    return e_success;
}

/* Copy remaining image data from src to dest */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
            return e_failure;
    }
    return e_success;
}

/* Main encoding process */
Status do_encoding(EncodeInfo *encInfo)
{
    printf("\nStarting Encoding...\n");

    if (open_files(encInfo) == e_failure)
        return e_failure;

    /* Check capacity before modifying/writing anything */
    if (check_capacity(encInfo) == e_failure)
    {
        /* close opened files */
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);
        return e_failure;
    }

    /* Copy BMP header first so both file pointers are at offset 54 */
    printf("Copying BMP header...\n");
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to copy BMP header\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);
        return e_failure;
    }

    /* Now encode magic, extension, size and data */
    printf("Encoding magic string...\n");
    if (encode_magic_string(encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode magic string\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);
        return e_failure;
    }

    printf("Encoding extension and size...\n");
    if (encode_secret_file_extn(".txt", encInfo) == e_failure ||
        encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode extension/size\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);
        return e_failure;
    }

    printf("Encoding secret data...\n");
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode secret data\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);
        return e_failure;
    }

    printf("Copying remaining image data...\n");
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed while copying remaining image data\n");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);
        return e_failure;
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    //printf("\n--- Encoding completed successfully! ---\n");
    return e_success;
}









