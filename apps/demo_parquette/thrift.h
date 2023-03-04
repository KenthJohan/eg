
/*
Offical apache thrift implementation:
	* Contain decoding function to read thrift protocol binary byte array:
		https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc
	* Contain a recursive implementation of thrift protocol parser:
		https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TProtocol.h#L670




This implementation differs from offical apache thrift implementation.
This uses a non recursive stack based thrift parser where you can set the max depth.


Usage example:
	//uint8_t const * data = <Start of thrift byte array>
    uint8_t const * current = data;
    thrift_type_t type;
    thrift_value_t value;
    int64_t id;
    while(current)
    {
        current = thrift_cursor_read_type(&cursor, current, data + data_length, &type, &id);
        if(type != THRIFT_STOP)
        {
            current = thrift_cursor_read_value(&cursor, current, data + data_length, &value);
        }
        print_field1(id, type, value, cursor.sp - ((type==THRIFT_STRUCT) || (type==THRIFT_LIST)));
    }

*/


#pragma once
#include <stdint.h>
 
// Apache reference:
// https://github.com/apache/thrift/blob/0bc818f363a50282910fa19ae605689954239fcb/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc#L51
typedef enum
{
	THRIFT_STOP          = 0x00, // End of struct
	THRIFT_BOOLEAN_TRUE  = 0x01, // Field type and value is true
	THRIFT_BOOLEAN_FALSE = 0x02, // Field type and value is false
	THRIFT_BYTE          = 0x03, // Field type
	THRIFT_I16           = 0x04, // Field type
	THRIFT_I32           = 0x05, // Field type
	THRIFT_I64           = 0x06, // Field type
	THRIFT_DOUBLE        = 0x07, // Field type
	THRIFT_BINARY        = 0x08, // Field type. This type is also used for string.
	THRIFT_LIST          = 0x09, // Field type
	THRIFT_SET           = 0x0A, // Field type
	THRIFT_MAP           = 0x0B, // Field type
	THRIFT_STRUCT        = 0x0C, // Field type, start of struct
} thrift_type_t;

// Generic thrift value:
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

// This is used for a non recursive thrift parser:
typedef struct
{
	int32_t id;
	thrift_type_t type;
	// TODO: These are only used when (type == THRIFT_LIST) which seems wasteful:
	thrift_type_t list_type;
	int32_t list_size;
} thrift_stack_t;

// Iterable thrift parse reader
typedef struct thrift_cursor_t thrift_cursor_t;
struct thrift_cursor_t
{
	int64_t last_field_id;
	int32_t sp;
	int32_t stack_size;
	thrift_stack_t * stack;
};

// Global API:
typedef void* (*thrift_api_malloc_t)(int size);
typedef void (*thrift_api_onerror)(char const *);
typedef struct {
	// Allocator callback:
    thrift_api_malloc_t malloc_;
	// Error callback:
    thrift_api_onerror onerror_;
} thrift_api_t;
extern thrift_api_t thrift_api;

// Set the cursor.stack_size before calling init:
void thrift_cursor_init(thrift_cursor_t * cursor);

// Both of this need to be called to iterate one step in thrift binary data
uint8_t const * thrift_cursor_read_type(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id);
uint8_t const * thrift_cursor_read_value(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_value_t * value);

// Convert type to string
char const * thrift_get_type_string(thrift_type_t type);

// Convert type and value to string
void thrift_get_field_str(thrift_type_t type, thrift_value_t value, char * buf, int n);