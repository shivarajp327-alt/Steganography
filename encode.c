#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

//-------------------------------------------------------------------------------//

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("\nImage width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("\nImage height = %u\n", height);

    // Return image capacity
    //printf("\nImage Capacity = %u\n",width * height * 3);
    return width * height * 3;
}

//-------------------------------------------------------------------------------//

uint get_file_size(FILE *fptr)
{
    //move the offset to last pos
    fseek(fptr,0,SEEK_END);

    //return ftell()
    //printf("\nFile size = %lu\n",ftell(fptr));
    return ftell(fptr);
}

//-------------------------------------------------------------------------------//

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file\nERRORs
 */

//-------------------------------------------------------------------------------//

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // CLA validation
    for(int i=2;i<4;i++)
    {
        if(argv[i]==NULL)
        {
            printf("\nValidation fails\n");
            return e_failure;
        }
    }

    //check ".bmp" exatention at last
    char *dot=strrchr(argv[2],'.');
    if(dot==NULL || strcmp(dot,".bmp")!=0)
    {
        printf("\nERROR : Source image file extention should me \".bmp\"\n");
            return e_failure;
            //if not ".bp extaction is not there" return e_failure
    }
    encInfo->src_image_fname=argv[2];  

    encInfo->secret_fname=argv[3];

    //check out file is given or not
    if(argv[4]!=NULL)
    {
        char *dot=strrchr(argv[4],'.');
        if(dot==NULL || strcmp(dot,".bmp")!=0)
        {
            printf("\nERROR : Output file extention should me \".bmp\"\n");
                return e_failure;
                //if not ".bp extaction is not there" return e_failure
        }
        encInfo->stego_image_fname=argv[4];
    }
    else
    {
        encInfo->stego_image_fname="output.bmp";
    }

    //open three file(source,screte,output)
    if(open_encode_files(encInfo)==e_failure)
    {
        printf("\nERROR : File doesn't open\n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] validations completed. Input data is valid\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status open_encode_files(EncodeInfo *encInfo)
{
    // source file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname,"r");
    if(encInfo->fptr_src_image==NULL)
    {
        printf("\nERROR : Unable to open source file \n");
        return e_failure;
    }

    //screte file
    encInfo->fptr_secret = fopen(encInfo->secret_fname,"r");
    if(encInfo->fptr_secret==NULL)
    {
        printf("\nERROR : Unable to open secret file \n");
        return e_failure;
    }

    //output file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname,"w");
    if(encInfo->fptr_stego_image==NULL)
    {
        printf("\nERROR : Unable to open output file\n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] Files opened \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status do_encoding(EncodeInfo *encInfo)
{
    // check capacity
    if(check_capacity(encInfo)==e_failure)
    {
        printf("\nERROR : Insufficient image capacity to store the screte file\n");
        return e_failure;
    }

    printf("\nEncoding...\n");
    // copy bmp header
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)
    {
        printf("\nERROR : unable to copy BMP Header\n");
        return e_failure;
    }

    // encode magic string
    if(encode_magic_string(MAGIC_STRING , encInfo)==e_failure)
    {
        printf("\nERROR : unable to encode magic string\n");
        return e_failure;
    }

    // encode_secret_file_extention size
    if(encode_secret_file_extn_size(encInfo)==e_failure)
    {
        printf("\nERROR : unable to encode secret file extention size\n");
        return e_failure;
    }

    // encode_secret_file_extention 
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo)==e_failure)
    {
        printf("\nERROR : unable to encode secret file extention\n");
        return e_failure;
    }

    // encode_secret_file_size
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo)==e_failure)
    {
        printf("\nERROR : unable to encode secret file size\n");
        return e_failure;
    }

    // encode_secret_file_data
    if(encode_secret_file_data(encInfo)==e_failure)
    {
        printf("\nERROR : unable to encode secret file data\n");
        return e_failure;
    }

    // copy remaining img data
    copy_remaining_img_data(encInfo->fptr_src_image ,encInfo->fptr_stego_image);


    //printf("\nEncoded successfuly.\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status check_capacity(EncodeInfo *encInfo)
{
    //get image capacity
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    
    //get file size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // check ((14+size_secret_file)*8)>image_capacity
    if(encInfo->image_capacity<((14+encInfo->size_secret_file)*8))
    {
        return e_failure;
    }

    printf("\n[ SUCCESS ] Sufficient image capacity available to store the screte file\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    //move the file ptr to the SEEK_SET or rewind()
    rewind(fptr_src_image);

    char buff[54];

    //read 54 bytes from src file
    if(fread(buff,54,1,fptr_src_image)!=1)
    {
        printf("\nERROR : failed tp read BMP header\n");
        return e_failure;
    }

    //write 54 byts to dest file
    if(fwrite(buff,54,1,fptr_dest_image)!=1)
    {
        printf("\nERROR : failed tp write BMP header\n");
        return e_failure;
    }

    printf("\n[ SUCCESS ] BMP header copied\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];

    for(int i=0;magic_string[i]!=0;i++)
    {
        //read 8byte from the source file to image_buffer
        if(fread(image_buffer,8,1,encInfo->fptr_src_image)!=1)
        {
            printf("\nERROR : Failed to read image data\n");
            return e_failure;
        }

        //encode image_buffer according to magic string
        if(encode_byte_to_lsb(magic_string[i], image_buffer)==e_failure)
        {
            return e_failure;
        }

        //write 8byte from image_buffer to output file
        if(fwrite(image_buffer,8,1,encInfo->fptr_stego_image)!=1)
        {
            printf("\nERROR : Failed to write image data\n");
            return e_failure;
        }
    }
    
    printf("\n[ SUCCESS ] Magic string encoded\n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_byte_to_lsb(unsigned char data, char *image_buffer)
{
    if (image_buffer == NULL)
    {
        printf("\nERROR : Invalid image buffer\n");
        return e_failure;
    }

    for(int i=7;i>=0;i--)
    {
        // get the ith bit set or not
        if((data>>i)&1) // set the lSB of image_buffer[]
        {
            image_buffer[7-i]=image_buffer[7-i] | 1;
        }
        else // clear the lSB of image_buffer[]
        {
            image_buffer[7-i]=image_buffer[7-i] & ~1;
        }
    }
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    
    char *dot = strrchr(encInfo->secret_fname,'.');
    strcpy(encInfo->extn_secret_file,dot);

    char buffer[32];

    //read 32 bytes from src_file into buffer
    fread(buffer,32,1,encInfo->fptr_src_image);
    
    //encode_size_to_lsb(strlen(extn_scr_file),buffer)
    if(encode_size_to_lsb(strlen(encInfo->extn_secret_file),buffer)==e_failure)
    {
        return e_failure;
    }
    
    //write 32 bytes buffer to output file
    fwrite(buffer,32,1,encInfo->fptr_stego_image);

    printf("\n[ SUCCESS ] Secret file extention size encoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_size_to_lsb(int size, char *image_buffer)
{
    if (image_buffer == NULL)
    {
        printf("\nERROR : Invalid image buffer\n");
        return e_failure;
    }

    for(int i=31;i>=0;i--)
    {
        // get the ith bit set or not
        if((size>>i)&1) // set the lSB of image_buffer[]
        {
            image_buffer[31-i]= image_buffer[31-i] | 1;
        }
        else  // clear the lSB of image_buffer[]
        {
            image_buffer[31-i]= image_buffer[31-i] & ~1;
        }
    }
      
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];

    for(int i=0;file_extn[i]!=0;i++)
    {
        //read 8 bytes from src_image
        if(fread(buffer,8,1,encInfo->fptr_src_image)!=1)
        {
            printf("\nERROR : Failed to read image data\n");
            return e_failure;   
        }

        //encode_byte_to_lsb(file_extn[],buffer)
        if(encode_byte_to_lsb(file_extn[i],buffer)==e_failure)
        {
            return e_failure;
        }

        //write 8 bytes buffer to output file
        if(fwrite(buffer,8,1,encInfo->fptr_stego_image)!=1)
        {
            printf("\nERROR : Failed to write image data\n");
            return e_failure;
        }
    }  
    
    printf("\n[ SUCCESS ] Secret file extention encoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    // read 32 bytes from src image to buffer
    if(fread(buffer,32,1,encInfo->fptr_src_image)!=1)
    {
        printf("\nERROR : Failed to read image data\n");
        return e_failure;
    }

    // encode size to lsb
    if(encode_size_to_lsb(file_size,buffer)==e_failure)
    {
        return e_failure;
    }

    // read 32 bytes from buffer to output file
    if(fwrite(buffer,32,1,encInfo->fptr_stego_image)!=1)
    {
        printf("\nERROR : Failed to write image data\n");
        return e_failure;
    }
    
    printf("\n[ SUCCESS ] Secret file size encoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    unsigned char data;
    rewind(encInfo->fptr_secret);
    while(fread(&data,1,1,encInfo->fptr_secret)==1)
    {
        // read 8 bytes from src_file to buffer
        if(fread(buffer,8,1,encInfo->fptr_src_image)!=1)
        {
            printf("\nERROR : Failed to read image data\n");
            return e_failure;
        }

        // encode_byte_to_lsb(data,buffer)
        if(encode_byte_to_lsb(data,buffer)==e_failure)
        {
            return e_failure;
        }

        // read 8 bytes from buffer to output file
        if(fwrite(buffer,8,1,encInfo->fptr_stego_image)!=1)
        {
            printf("\nERROR : Failed to write image data\n");
            return e_failure;
        }
    }

    printf("\n[ SUCCESS ] Secret file data encoded \n");
    return e_success;
}

//-------------------------------------------------------------------------------//

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char data;

    //read char from src_file and write to output_file untill EOF
    while(fread(&data,1,1,fptr_src)==1)
    {
        fwrite(&data,1,1,fptr_dest);
    }

    printf("\n[ SUCCESS ] Copied remaining img data \n");
}

//-------------------------------------------------------------------------------//