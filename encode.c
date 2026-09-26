#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include <string.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
        encInfo->fptr_src_image = fopen(encInfo->src_image_fname,"r");
        if(encInfo->fptr_src_image == NULL)
        {
            printf("Source File not open\n");
            return e_failure;
        }
        encInfo->fptr_secret = fopen(encInfo->secret_fname,"r");
        if(encInfo->fptr_secret == NULL)
        {
            printf("Secret File not open\n");
            return e_failure;
        }
        encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname,"w");
        if(encInfo->fptr_stego_image == NULL)
        {
            printf("output File not open\n");
            return e_failure;
        }
        printf("All files opened succesfully\n");
        return e_success;


}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    
    char *dot = strrchr(argv[2],'.');
    if(strcmp(dot,".bmp") != 0)
    {
        printf("Error\n");
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];
    if(argv[4] != NULL)
    {
        char *dot = strrchr(argv[4],'.');
         if(strcmp(dot,".bmp") != 0)
        {
            printf("Error\n");
            return e_failure;
        } 
        encInfo-> stego_image_fname = argv[4];
          
    }
    else
    {
        encInfo-> stego_image_fname = "output.bmp";
    }
    open_files (encInfo);
    printf("All validations completed succesfully\n");
    return e_success;

}

Status do_encoding(EncodeInfo *encInfo)
{
    if(check_capacity(encInfo) == e_failure)
    {
        printf("Insufficient capacity\n");
        return e_failure;
    }
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image )== e_failure)
    {
        printf("Unable to copy bmp header\n");
        return e_failure;
    }
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("Unable to encode\n");
        return e_failure;
    }
    if(encode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("Unable to encode_secret_file_extn_size\n");
        return e_failure;
    }
     if(encode_secret_file_extn(encInfo) == e_failure)
    {
        printf("Unable to encode_secret_file_extn_size\n");
        return e_failure;
    }
    if(encode_secret_file_size(encInfo) == e_failure)
    {
        printf("Unable to encode_secret_file_size\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Unable to encode_secret_file_data\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("Unable to copy_remaining_img_data\n");
        return e_failure;
    }
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if(encInfo->image_capacity < (14 + encInfo->size_secret_file)*8)
    {
        printf("Unsufficient image size\n");
        return e_failure;
    }
   return  e_success;
}

/* Get image size */
//uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind (fptr_src_image);
    char buffer[54];
    fread(buffer,54,1,fptr_src_image);
    fwrite(buffer,54,1,fptr_dest_image);
    printf("Copied header succesfully\n");
    return e_success;
}

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    for(int i=0;i<2;i++)
    {
        fread(buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}


Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char *dot = strrchr(encInfo->secret_fname,'.');
    char buffer[32] ;
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(strlen(dot),buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    printf("encode_secret_file_extn_size success\n");
    return e_success;
}

/* Encode secret file extenstion */
Status encode_secret_file_extn(EncodeInfo *encInfo)
{
    char *dot = strrchr(encInfo->secret_fname,'.');
    char buffer[8] ;
    for(int i=0;dot[i]!=0;i++)
    {
    fread(buffer,8,1,encInfo->fptr_src_image);
    encode_byte_to_lsb(dot[i],buffer);
    fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    printf("encode_secret_file_extn success\n");
    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(encInfo->size_secret_file,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    printf("encode_secret_file_size success\n");
    return e_success;
}

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);
    char buffer[8],data;
    while(fread(&data,1,1,encInfo->fptr_secret) == 1)
    {
        fread(buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(data, buffer);
        fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    printf("encode_secret_file_data\n");
    return e_success;
}


/* Encode function, which does the real encoding */

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=7;i>=0;i--)
    {
        if((data >> i) & 1)
        {
            image_buffer[7-i] = image_buffer[7-i] | 1; 
        }
        else
        {
            image_buffer[7-i] = image_buffer[7-i] & ~1; 
        }
    }
}

Status encode_size_to_lsb(char data, char *image_buffer)
{
    for(int i=31;i>=0;i--)
    {
        if((data >> i) & 1)
        {
            image_buffer[31-i] = image_buffer[31-i] | 1; 
        }
        else
        {
            image_buffer[31-i] = image_buffer[31-i] & ~1; 
        }
    }
}

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char data;
    while(fread(&data,1,1,fptr_src) == 1)
    {
        fwrite(&data,1,1,fptr_dest);
    }
    return e_success;
}
