#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;

    /* Secret File Info */
    long size_secret_file;
    uint secret_file_extn_size;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;


//------------------------------------------------------------------------------------//

/* Decoding function prototype */

/* Check operation type */
OperationType check_operation_type(char opt);

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/*Decode secret file extantion size*/
Status decode_secret_file_extn_size( DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

//------------------------------------------------------------------------------------//

/* Decode functions, which does the real decoding */

/* Decode a size from LSB of image data array */
Status decode_size_to_lsb(char *image_buffer,int *data);

/* Decode a byte from LSB of image data array */
Status decode_byte_to_lsb(char *image_buffer, unsigned char *data);

#endif

//------------------------------------------------------------------------------------//