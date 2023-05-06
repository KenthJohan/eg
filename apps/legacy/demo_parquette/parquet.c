#include <stdlib.h>
#include <stdint.h>
#include <flecs.h>

#include "parquet.h"
#include "thrift.h"
#include "base64.h"

#define ASSERT(c) if (!(c)) __builtin_trap()


char const * parquet_get_id_string(uint32_t id)
{
	switch(id)
	{
    case PARQUET_ID_TYPE: return "PARQUET_ID_TYPE";
    case PARQUET_ID_NAME: return "PARQUET_ID_NAME";
	default: return NULL;
	}
}


char const * parquet_get_type_string(uint32_t t)
{
	switch(t)
	{
    case PARQUET_BOOLEAN: return "PARQUET_BOOLEAN";
    case PARQUET_INT32: return "PARQUET_INT32";
    case PARQUET_INT64: return "PARQUET_INT64";
    case PARQUET_INT96: return "PARQUET_INT96";
    case PARQUET_FLOAT: return "PARQUET_FLOAT";
    case PARQUET_DOUBLE: return "PARQUET_DOUBLE";
    case PARQUET_BYTE_ARRAY: return "PARQUET_BYTE_ARRAY";
    case PARQUET_FIXED_LEN_BYTE_ARRAY: return "PARQUET_FIXED_LEN_BYTE_ARRAY";
	default: return "";
	}
}



struct ByteArray
{
    uint32_t len;
    uint8_t * ptr;
};

int parquet_read_bytearray(uint8_t* data, int64_t data_size, int num_values, int type_length, struct ByteArray* out)
{
    int bytes_decoded = 0;
    int increment;
    for (int i = 0; i < num_values; ++i)
    {
        uint32_t len = (uint32_t)(*data);
        out[i].len = len;
        increment = sizeof(uint32_t) + len;
        if (data_size < increment) {return EOF;}
        out[i].ptr = data + sizeof(uint32_t);
        data += increment;
        data_size -= increment;
        bytes_decoded += increment;
    }
    return bytes_decoded;
}


void parquet_assigner_filemetadata(parquet_filemetadata_t *data, int32_t id, int32_t type, thrift_value_t value)
{
    switch (id)
    {
    case 1:data->version = value.value_i64; break;
    case 2:data->schema = ecs_vector_new(parquet_schema_element_t, 0); break;
    case 3:data->num_rows = value.value_i64; break;
    default:break;
    }
};






void parquet_read1(parquet_reader1_t * reader, char const * filename)
{
    ASSERT(reader);
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
    int8_t * data = ecs_os_malloc(l);
    fread(data, l, 1, file);

    

    // column first_name
    {
        int data_size = 1000000;
        int n = 1000;
        fseek(file, 17317, SEEK_SET);
        struct ByteArray * a = ecs_os_malloc_n(struct ByteArray, n);
        uint8_t* data = ecs_os_malloc_n(uint8_t, data_size);
        fread(data, n, 1, file);
        parquet_read_bytearray(data, data_size, n, 1, a);
        printf("parquet_read_bytearray\n");
        for(int i = 0; i < n; ++i)
        {
            ptrdiff_t diff = a[i].ptr - data;
		    printf("%4i %4i %08jd: %.*s\n", i, a[i].len, diff, a[i].len, a[i].ptr);
        }
        printf("parquet_read_bytearray\n");
    }

	if(file) {fclose(file);}
	if(data) {ecs_os_free(data);}
}




