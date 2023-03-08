#include "thrift_testing.h"
#include "thrift.h"
#include "base64.h"
#include <stdlib.h>
#include <stdio.h>
#include <flecs.h>

#define ASSERT(c) if (!(c)) __builtin_trap()


void print_field1(int32_t id, thrift_type_t type, thrift_value_t value, int32_t sp)
{
	char buf[100] = {0};
    printf("%02i|%-6s|%02i|", sp, thrift_get_type_string(type), id);
	for(int i = 0; i < sp; ++i){printf("    ");}
	switch (type)
	{
	case THRIFT_STOP:
        printf("}\n");
    break;
	default:break;
	}
	switch (type)
	{
	case THRIFT_STRUCT:
		printf("{\n");
		break;
	case THRIFT_LIST:
	    thrift_get_field_str(type, value, buf, 100);
		printf("%s [\n", buf);
		break;
	case THRIFT_STOP:break;
	default:
	    thrift_get_field_str(type, value, buf, 100);
		printf("%-20s\n", buf);
		break;
	}
}


void thrift_onerror(char const * text)
{
    printf("[THRIFT_ERROR] %s\n", text);
}



void thrift_testing_print(uint8_t const * data, int32_t l)
{
    thrift_api.malloc_ = malloc;
    thrift_api.onerror_ = thrift_onerror;

    uint8_t const * current = data;
    thrift_type_t type = THRIFT_STRUCT;
    thrift_value_t value;
    int64_t id;
 	thrift_cursor_t cursor = {0};
    cursor.stack_size = 64;
    thrift_cursor_init(&cursor);
    while(current)
    {
        //print_field1(id, type, value, cursor.sp);
        current = thrift_cursor_next(&cursor, current, data+l, &type, &id, &value);
        print_field1(id, type, value, cursor.sp - ((type==THRIFT_STRUCT) || (type==THRIFT_LIST)));
    }
}






void thrift_testing_parquette(char const * filename)
{
    ASSERT(filename);
	FILE * file = NULL;
	ecs_os_fopen(&file, filename, "rb");
    ASSERT(file);
    char par1[4] = {0};
    char par2[4] = {0};
    int32_t l = 0;
    fseek(file, 0, SEEK_SET);
    fread(par1, sizeof(par1), 1, file);
    fseek(file, -8, SEEK_END);
    fread(&l, sizeof(int32_t), 1, file);
    fseek(file, 0, SEEK_SET);
    fread(par2, sizeof(par2), 1, file);
    fseek(file, -8-l, SEEK_END);
    uint8_t * data = ecs_os_malloc(l);
    fread(data, l, 1, file);

	thrift_testing_print(data, l);



    




    // column first_name
    /*
    {
        int data_size = 1000000;
        int n = 1000;
        fseek(file, 17317, SEEK_SET);
        struct ByteArray * a = ecs_os_malloc_n(struct ByteArray, n);
        const uint8_t* data = ecs_os_malloc_n(uint8_t, data_size);
        fread(data, n, 1, file);
        parquet_read_bytearray(data, data_size, n, 1, a);
        printf("parquet_read_bytearray\n");
        for(int i = 0; i < n; ++i)
        {
		    printf("%4i %4i %08i: %.*s\n", i, a[i].len, (intptr_t)a[i].ptr - (intptr_t)data, a[i].len, a[i].ptr);
        }
        printf("parquet_read_bytearray\n");
    }
    */
	if(file) {fclose(file);}
	if(data) {ecs_os_free(data);}
}




void thrift_testing_demo1()
{
    base64_testunit1();

    //char const *b64 = "SGVsbG8gV29ybGQh";
    char const *b64 = "gAEAAQAAAAVsb2dpbgAAAAAMAAALAAAAAAAIY29vbHVzZXILAAEAAAALcGFzc3dvcmQxMjMAAA==";
    size_t l = base64_decoded_len(b64, strlen(b64));
    uint8_t * d = malloc(l);
	base64_decode(b64, strlen(b64), d, l);

    thrift_testing_print((uint8_t const *)d, (int32_t)l);

}
