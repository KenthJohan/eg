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


#define THRIFT_MAX_STRING_SIZE 1024

typedef struct
{
	int32_t id;
	thrift_type_t type;
	thrift_type_t list_type;
	int32_t list_size;
} thrift_stack_t;

typedef struct thrift_cursor_t thrift_cursor_t;
struct thrift_cursor_t
{
	int64_t last_field_id;
	int32_t sp;
	int32_t stack_size;
	thrift_stack_t * stack;
};

typedef void* (*thrift_api_malloc_t)(int size);
typedef void (*thrift_api_onerror)(char const *);

typedef struct {
    thrift_api_malloc_t malloc_;
    thrift_api_onerror onerror_;
} thrift_api_t;

extern thrift_api_t thrift_api;




void thrift_cursor_init(thrift_cursor_t * cursor);
uint8_t const * thrift_cursor_read_value(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_value_t * value);
uint8_t const * thrift_cursor_read_type(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id);

// Convert type to string
char const * thrift_get_type_string(thrift_type_t type);

// Convert type and value to string
void thrift_get_field_str(thrift_type_t type, thrift_value_t value, char * buf, int n);