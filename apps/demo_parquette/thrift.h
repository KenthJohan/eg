#pragma once
#include <stdint.h>

typedef enum
{
	THRIFT_STOP = 0x00,
	THRIFT_BOOLEAN_TRUE = 0x01,
	THRIFT_BOOLEAN_FALSE = 0x02,
	THRIFT_BYTE = 0x03,
	THRIFT_I16 = 0x04,
	THRIFT_I32 = 0x05,
	THRIFT_I64 = 0x06,
	THRIFT_DOUBLE = 0x07,
	THRIFT_BINARY = 0x08,
	THRIFT_LIST = 0x09,
	THRIFT_SET = 0x0A,
	THRIFT_MAP = 0x0B,
	THRIFT_STRUCT = 0x0C
} thrift_type_t;


typedef union
{
	uint64_t value_u64;
	int64_t value_i64;
	struct
	{
		thrift_type_t list_type;
		int32_t list_size;
	};
	struct
	{
		int32_t string_size;
		char * string_data;
	};
} thrift_value_t;



typedef void* (*thrift_api_malloc_t)(int size);

typedef struct {
    thrift_api_malloc_t malloc_;
} thrift_api_t;

extern thrift_api_t thrift_api;



void thrift_get_field_str(int32_t type, thrift_value_t value, char * buf, int n);
char const * thrift_get_type_string(uint32_t t);







#define THRIFT_STACK_MAX_SIZE 64
#define THRIFT_MAX_STRING_SIZE 1024
typedef struct thrift_stack_t thrift_stack_t;
struct thrift_stack_t
{
	int64_t last_field_id;
	int32_t sp;
	int32_t       stack_id        [THRIFT_STACK_MAX_SIZE];
	thrift_type_t stack_type      [THRIFT_STACK_MAX_SIZE];
	thrift_type_t stack_list_type [THRIFT_STACK_MAX_SIZE];
	int32_t       stack_list_size [THRIFT_STACK_MAX_SIZE];
    void (*cb_field)(thrift_stack_t * ctx, int32_t id, int32_t type, thrift_value_t value);
};


int thrift_read(thrift_stack_t * ctx, int8_t const * data, int32_t data_length);