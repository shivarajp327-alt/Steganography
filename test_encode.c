#include <stdio.h>
#include "encode.h"
#include "types.h"


int main(int argc,char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    if(check_operation_type(argv[1][1])==e_encode)
    {
        if(read_and_validate_encode_args(argv,&encInfo) == e_failure)
        {
            printf("validation does not pass\n");
            return 0;
        }
        if(do_encoding(&encInfo) == e_failure)
        {
            printf("Encoding failed\n");
            return 0;
        }
    }
    else if(check_operation_type(argv[1][1]) == e_decode)
    {
        if(read_and_validate_decode_args(argv,&decInfo) == e_failure)
        {
            printf("Validation does not pass\n");
            return 0;
        }
        if(do_decoding(&decInfo) == e_failure)
        {
            printf("Decoding failed\n");
            return 0;
        }
    }
    
    return 0;
}


OperationType check_operation_type(char op_t)
{
    if(op_t== 'e')
    {
        return e_encode;
    }
    else if(op_t == 'd')
    {
        return e_decode;
    }
    else{
        return e_unsupported;
    }
}





