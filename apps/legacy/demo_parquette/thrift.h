// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
 
// Apache reference:
// https://github.com/apache/thrift/blob/0bc818f363a50282910fa19ae605689954239fcb/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc#L51
typedef enum
{
	THRIFT_STOP		  = 0x00, // End of struct
	THRIFT_BOOLEAN_TRUE  = 0x01, // Field type and value is true
	THRIFT_BOOLEAN_FALSE = 0x02, // Field type and value is false
	THRIFT_BYTE		  = 0x03, // Field type
	THRIFT_I16		   = 0x04, // Field type
	THRIFT_I32		   = 0x05, // Field type
	THRIFT_I64		   = 0x06, // Field type
	THRIFT_DOUBLE		= 0x07, // Field type
	THRIFT_BINARY		= 0x08, // Field type. This type is also used for string.
	THRIFT_LIST		  = 0x09, // Field type
	THRIFT_SET		   = 0x0A, // Field type
	THRIFT_MAP		   = 0x0B, // Field type
	THRIFT_STRUCT		= 0x0C, // Field type, start of struct
	THRIFT_UUID		  = 0x0D, // Field type
} thrift_type_t;

// Generic thrift value:
typedef union
{
	double value_f64;
	uint64_t value_u64;
	int64_t value_i64;
	int32_t value_i32;
	int16_t value_i16;
	int8_t value_i8;
	uint8_t value_u8;
	int value_bool;
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
	//	   Is there a way to reduce stack bloat?
	thrift_type_t list_type;
	int32_t	   list_size;
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
typedef void* (*thrift_api_malloc_t)(size_t size);
typedef void (*thrift_api_on_error_t)(char const *);
typedef struct {
	// Allocator callback:
	thrift_api_malloc_t malloc_;
	// Error callback:
	thrift_api_on_error_t onerror_;
} thrift_api_t;
extern thrift_api_t thrift_api;

// Set the cursor.stack_size before calling init:
void thrift_cursor_init(thrift_cursor_t * cursor);

// This will not push anything to the stack:
uint8_t const * thrift_cursor_next_type(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id);

// This might push things to the stack:
uint8_t const * thrift_cursor_next_value(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t type, thrift_value_t * value);

// Call this to parse a single item:
// This is just a wrapper of thrift_cursor_next_type and thrift_cursor_next_value:
uint8_t const * thrift_cursor_next(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id, thrift_value_t * value);

// Convert type to string
char const * thrift_get_type_string(thrift_type_t type);

// Convert type and value to string
void thrift_get_field_str(thrift_type_t type, thrift_value_t value, char * buf, int n);