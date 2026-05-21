/*
NAME: kavya p
Description:This project implements a simple image steganography system that hides a secret text message inside a BMP image using the Least Significant Bit (LSB) technique.
The main goal is to securely conceal data within an image file so that it is not visible to the human eye.
The project has two main operations:
Encoding: Hides the secret text (.txt file) inside a .bmp image by modifying the least significant bits of the image pixels and creates a new stego image.
Decoding: Extracts the hidden text message from the stego image and saves it into an output text file.
It includes modules for encoding, decoding, file validation, and bit-level manipulation. This ensures secure, lossless, and reliable data hiding within images.

DATE: 27/10/2025

Sample I/O: Input:Encoding:-./stego -e beautiful.bmp secret.txt stego.bmp

sample output:-
Selected Operation: Encoding
Validation successful.
Read and validate successful
Starting Encoding...

---Opening files---
Files opened successfully.

---Checking image capacity---
Capacity sufficient.
Copying BMP header...
Encoding magic string...
Encoding extension and size...
Encoded secret file size: 35 bytes
Encoding secret data...
Copying remaining image data...

---Encoding completed successfully!---

sample input:decoding:-./stego -d stego.bmp secret.txt
sample output:-
Selected Operation: Decoding
Decoding started...
Decoding magic string...
Magic string found:
Decoding secret file extension...
Decoding secret file size...
Decoding secret file data...
Secret file written successfully: secret.txt

Decoded secret data:
--------------------
This is the decoded secret message!
--------------------

---Decoding completed successfully!---

*/

#include <stdio.h> // Standard I/O functions
#include <string.h> // For string handling functions
#include "encode.h" // Header file for encoding-related functions
#include "decode.h" // Header file for decoding-related functions
#include "types.h" // Custom data types and enums
#include "common.h" // Common utility functions
 
int main(int argc, char *argv[])
{
    // If the user runs ./a.out or provides no arguments
    if (argc <= 1)
    {
        printf("Invalid Command!\n");
        printf("Usage:\n");
        printf("  Encode: ./a.out -e <input.bmp> <secret.txt> <output.bmp>\n");
        printf("  Decode: ./a.out -d <stego.bmp> <output_secret.txt>\n");
        return 1;
    }

    // -------------------- ENCODING MODE --------------------
    if (strcmp(argv[1], "-e") == 0)
    {
        printf("Selected Operation: Encoding\n");

        // Check argument count (must be exactly 5)
        if (argc != 5)
        {
            printf("ERROR: Invalid number of arguments for encoding.\n");
            printf("Usage: ./a.out -e <input.bmp> <secret.txt> <output.bmp>\n");
            return 1; // Exit if argument count is invalid
        }

        //  Extra validation: check for wrong commas or missing dots
        if (strchr(argv[2], ',') || strchr(argv[3], ',') || strchr(argv[4], ','))
        {
            printf("ERROR: Invalid file name! Remove any commas and use correct extensions (.bmp / .txt).\n");
            return 1;
        }

        // Validate file extensions
        if (strstr(argv[2], ".bmp") == NULL) // Check input image type
        {
            printf("ERROR: Input image must be a .bmp file.\n");
            return 1;
        }
        if (strstr(argv[3], ".txt") == NULL) // Check secret file type
        {
            printf("ERROR: Secret file must be a .txt file.\n");
            return 1;
        }
        if (strstr(argv[4], ".bmp") == NULL) // Check output image type
        {
            printf("ERROR: Output image must be a .bmp file.\n");
            return 1;
        }

        // Initialize structure with filenames
        EncodeInfo encInfo; 
        encInfo.src_image_fname = argv[2]; // Source BMP file
        encInfo.secret_fname = argv[3]; // Secret text file
        encInfo.stego_image_fname = argv[4]; // Output encoded image

        // Ask user for custom magic string
        printf("Enter a magic string (used for verification during decoding): ");
        scanf("%99s", encInfo.user_magic_string); // Store user input safely

        // Validate arguments and perform encoding
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Validation successful.\n");

            // Start encoding process
            if (do_encoding(&encInfo) == e_success)
                printf("\n--- Encoding completed successfully! ---\n");
            else
                printf("ERROR: Encoding failed.\n");
        }
        else
        {
            printf("ERROR: Failed to validate encoding arguments.\n");
        }
    }

    // -------------------- DECODING MODE --------------------
    else if (strcmp(argv[1], "-d") == 0) // Check if mode is decoding
    {
        printf("Selected Operation: Decoding\n");

        // Check argument count (must be exactly 4)
        if (argc != 4)
        {
            printf("ERROR: Invalid number of arguments for decoding.\n");
            printf("Usage: ./a.out -d <stego.bmp> <output_secret.txt>\n");
            return 1;
        }

        //  Extra validation: check for wrong commas or missing dots
        if (strchr(argv[2], ',') || strchr(argv[3], ','))
        {
            printf("ERROR: Invalid file name! Remove any commas and use correct extensions (.bmp / .txt).\n");
            return 1;
        }

        // Validate file extensions
        if (strstr(argv[2], ".bmp") == NULL) // Check stego image type
        {
            printf("ERROR: Stego image must be a .bmp file.\n");
            return 1;
        }
        if (strstr(argv[3], ".txt") == NULL) // Check output secret file type
        {
            printf("ERROR: Output secret file must be a .txt file.\n");
            return 1;
        }

        // Initialize structure with filenames
        DecodeInfo decInfo;
        decInfo.stego_image_fname = argv[2]; // Input stego image
        decInfo.output_fname = argv[3];  // Output text file

        // Ask user for the magic string (to verify)
        char user_magic[50];
        printf("Enter the magic string used during encoding: ");
        scanf("%49s", user_magic);

        // Validate arguments and perform decoding
        if (read_and_validate_decode_args(argc, argv, &decInfo) == e_success)
        {
            printf("Validation successful.\n");

            // Start decoding process
            if (do_decoding(&decInfo) == e_success)
            {
                printf("\n--- Decoding completed successfully! ---\n");
            }
            else
            {
                printf("ERROR: Decoding failed.\n");
            }
        }
        else
        {
            printf("ERROR: Failed to validate decoding arguments.\n");
        }
    }

    // -------------------- INVALID OPTION --------------------
    else
    {
        printf("Invalid option: %s\n", argv[1]);
        printf("Usage:\n");
        printf("  Encode: ./a.out -e <input.bmp> <secret.txt> <output.bmp>\n");
        printf("  Decode: ./a.out -d <stego.bmp> <output_secret.txt>\n");
    }

    return 0;
}

