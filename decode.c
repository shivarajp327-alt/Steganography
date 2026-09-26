#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"


//-------------------------------------------------------------------------------//

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // CLA validation
    for(int i=2;i<3;i++)
    {
        if(argv[i]==NULL)
        {
            printf("\nInvalid input\n");
            return e_failure;
        }
    }

    //check ".bmp" exatention at last
    char *dot=strrchr(argv[2],'.');
    if(dot==NULL || strcmp(dot,".bmp")!=0)
    {
        printf("\n\nERROR : Source image file extention should me \".bmp\"\n");
            return e_failure;
            //if not ".bp extaction is not there" return e_failure
    }
    decInfo->src_image_fname=argv[2];  

    //check out file is given or not
    if(argv[3]!=NULL)
    {
        char name_buffer[20];
        sscanf(argv[3],"%[^.]",name_buffer);
        strcat(name_buffer,".txt");
        decInfo->stego_image_fname=name_buffer;
    }
    else
    {
        decInfo->stego_image_fname="decoded.txt";
    }

    //open three file(source,outout)
    if(open_decode_files(decInfo)==e_failure)
    {
        printf("\n\nERROR : File doesn't open\n");
        return e_failure;
    }

    // check BM signature
    char signature[2];
    // read 2 bytes of data from image
    if(fread(signature, 2, 1, decInfo->fptr_src_image) != 1)
    {
        printf("\nERROR : Failed to read image data\n");
        return e_failure;
    }
    signature[2]=0;
    if(strcmp(signature,"BM")!=0)
    {
        printf("Error: Invalid BMP signature\n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] Validations completed. Input data is valid\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status open_decode_files(DecodeInfo *decInfo)
{
    // source file
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname,"r");
    if(decInfo->fptr_src_image==NULL)
    {
        printf("\nERROR : Unable to open Source file\n");
        return e_failure;
    }
    
    //outout file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"w");
    if(decInfo->fptr_stego_image==NULL)
    {
        printf("\nERROR : Unable to open outout file \n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] Files opened\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status do_decoding(DecodeInfo *decInfo)
{
    printf("\nDecoding...\n");
    // skip 54 bmp header
    fseek(decInfo->fptr_src_image,54,SEEK_SET);
    //printf("\n%lu\n",ftell(decInfo->fptr_src_image));

    // decode magic string
    if(decode_magic_string(MAGIC_STRING , decInfo)==e_failure)
    {
        printf("\n\nERROR : Unable to decode magic string\n");
        return e_failure;
    }

    // decode file extention size
    if(decode_secret_file_extn_size(decInfo)==e_failure)
    {
        printf("\n\nERROR : Unable to decode secret file extention size\n");
        return e_failure;
    }

    // decode file extention 
    if(decode_secret_file_extn(decInfo)==e_failure)
    {
        printf("\n\nERROR : Unable to decode secret file extention\n");
        return e_failure;
    }

    // decode file size
    if(decode_secret_file_size(decInfo)==e_failure)
    {
        printf("\n\nERROR : Unable to decode secret file size\n");
        return e_failure;
    }

    // decode file data
    if(decode_secret_file_data(decInfo)==e_failure)
    {
        printf("\n\nERROR : Unable to decode secret file data\n");
        return e_failure;
    }

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_byte_to_lsb(char *image_buffer, unsigned char *data)
{
    if (image_buffer == NULL)
    {
        printf("\nERROR : Invalid image buffer\n");
        return e_failure;
    }

    int j=7;
    *data=0;
    for(int i=0;i<8;i++)
    {
        // get the lsb bit set or not
        if(image_buffer[i] & 1) // set the ith bit of data
        {
            *data = (*data) | (1<<j);
        }
        j--;
    }

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char de_magic_buffer[2];
    char image_buffer[8];

    for(int i=0;i<2;i++)
    {
        char data=0;
        
        if(fread(image_buffer,8,1,decInfo->fptr_src_image)!=1)
        {
            printf("\nERROR : Failed to read image data\n");
            return e_failure;
        }

        if(decode_byte_to_lsb(image_buffer,&data)==e_failure)
        {
            return e_failure;
        }

        //printf("\n magic str = %c\n",data);
        de_magic_buffer[i]=data;
    }
    de_magic_buffer[2]='\0';

    if(strcmp(de_magic_buffer,magic_string)!=0)
    {
        printf("\n\nERROR : Magic string doesnt match\n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] Magic string decoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_size_to_lsb(char *image_buffer,int *data)
{
    if (image_buffer == NULL)
    {
        printf("\nERROR : Invalid image buffer\n");
        return e_failure;
    }

    int j=31;
    for(int i=0;i<32;i++)
    {
        // get the lsb bit set or not
        if(image_buffer[i] & 1) // set the ith bit of data
        {
            *data = (*data) | (1<<j) ;
        }
        j--;
    }

    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_extn_size( DecodeInfo *decInfo)
{

    char buffer[32];
    int extn_size=0;
    //read 32 bytes from src_file into buffer
    if(fread(buffer,32,1,decInfo->fptr_src_image)!=1)
    {
        printf("\nERROR : Failed to read image data\n");
            return e_failure;
    }
    
    //decode_size_to_lsb(strlen(extn_scr_file),buffer)
    if(decode_size_to_lsb(buffer,&extn_size)==e_failure)
    {
        return e_failure;
    }

    //printf("\n%d\n",extn_size);
    if(extn_size!=4)
    {
        printf("\n\nERROR : Secret file extention size doesnt match\n");
        return e_failure;
    }

    decInfo->secret_file_extn_size=extn_size;
    
    printf("\n[ SUCCESS ] Secret file extention size decoded\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
  char buffer[8];
  char extn[4];
    for(int i=0;i<decInfo->secret_file_extn_size;i++)
    {
        char data=0;
        //read 8 bytes from src_image
        if(fread(buffer,8,1,decInfo->fptr_src_image)!=1)
        {
            printf("\nERROR : Failed to read image data\n");
            return e_failure;
        }

        //decode_byte_to_lsb
        if(decode_byte_to_lsb(buffer,&data)==e_failure)
        {
            return e_failure;
        }

        extn[i]=data;
    }  
    extn[4]='\0';
    //printf("\n%s\n",extn);

    if(strcmp(extn,".txt")!=0)
    {
        printf("\n\nERROR : Secret file extention doesnt match\n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] Secret file extention decoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    int data=0;
    // read 32 bytes from src image to buffer
    if(fread(buffer,32,1,decInfo->fptr_src_image)!=1)
    {
        printf("\nERROR : Failed to read image data\n");
        return e_failure;
    }

    // decode size to lsb
    if(decode_size_to_lsb(buffer,&data)==e_failure)
    {
        return e_failure;
    }

    decInfo->size_secret_file=data;

    //printf("\n%ld\n",decInfo->size_secret_file);
    printf("\n[ SUCCESS ] Secret file size decoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    int i;
    char data;

    for(i=0;i<decInfo->size_secret_file;i++)
    {
        //read 8 bytes from src_image
        if(fread(buffer,8,1,decInfo->fptr_src_image)!=1)
        {
            printf("\nERROR : Failed to read image data\n");
            return e_failure;
        }

        //decode_byte_to_lsb
        if(decode_byte_to_lsb(buffer,&data)==e_failure)
        {
            return e_failure;
        }
        
       if(fwrite(&data,1,1,decInfo->fptr_stego_image)!=1)
       {
            printf("\nERROR : Failed to write image data\n");
            return e_failure;
       }
    }

    printf("\n[ SUCCESS ] Secret file data decoded\n");
    return e_success;
}

//-------------------------------------------------------------------------------//