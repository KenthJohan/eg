// SPDX-License-Identifier: MIT
#include "thrift.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>




#define THRIFT_MAX_STRING_SIZE 1024
#define ENABLE_LOG
#define ENABLE_ASSERT

#ifdef __GNUC__
#define UNLIKELY(val) (__builtin_expect((val), 0))
#else
#define UNLIKELY(val) (val)
#endif

#ifdef ENABLE_LOG
#define LOG_ERROR(s) thrift_api.onerror_(s)
#define LOG_ERROR_CURSOR(cursor, s) thrift_api.onerror_(s)
#else
#define LOG_ERROR(s)
#define LOG_ERROR_CURSOR(cursor, s)
#endif

#ifdef ENABLE_ASSERT
#define ASSERT(c) if (!(c)) __builtin_trap()
#else
#define ASSERT(c)
#endif

#define bswap_64(n) \
	( (((n) & 0xff00000000000000ull) >> 56) \
	| (((n) & 0x00ff000000000000ull) >> 40) \
	| (((n) & 0x0000ff0000000000ull) >> 24) \
	| (((n) & 0x000000ff00000000ull) >> 8)  \
	| (((n) & 0x00000000ff000000ull) << 8)  \
	| (((n) & 0x0000000000ff0000ull) << 24) \
	| (((n) & 0x000000000000ff00ull) << 40) \
	| (((n) & 0x00000000000000ffull) << 56) )


#define ZIGZAG_TO_I32(n) ((n) >> 1) ^ (-(int32_t)((n) & 1));
#define ZIGZAG_TO_I64(n) ((n) >> 1) ^ (-(int64_t)((n) & 1));



thrift_api_t thrift_api = {
	.malloc_ = NULL,
	.onerror_ = NULL,
};



char const * thrift_get_type_string(thrift_type_t type)
{
	switch(type)
	{
	case THRIFT_STOP:          return "STOP";
	case THRIFT_BOOLEAN_TRUE:  return "BOOLEAN_TRUE";
	case THRIFT_BOOLEAN_FALSE: return "BOOLEAN_FALSE";
	case THRIFT_BYTE:          return "BYTE";
	case THRIFT_I16:           return "I16";
	case THRIFT_I32:           return "I32";
	case THRIFT_I64:           return "I64";
	case THRIFT_DOUBLE:        return "DOUBLE";
	case THRIFT_BINARY:         return "BINARY";
	case THRIFT_LIST:          return "LIST";
	case THRIFT_SET:           return "SET";
	case THRIFT_MAP:           return "MAP";
	case THRIFT_STRUCT:        return "STRUCT";
	default:                   return "";
	}
}

// Return true if type can be a element type in a list
int thrift_is_list_element_type(thrift_type_t type)
{
	switch (type)
	{
	case THRIFT_BYTE:    return 1;
	case THRIFT_I16:     return 1;
	case THRIFT_I32:     return 1;
	case THRIFT_I64:     return 1;
	case THRIFT_DOUBLE:  return 1;
	case THRIFT_BINARY:  return 1;
	case THRIFT_LIST:    return 1;
	case THRIFT_SET:     return 1;
	case THRIFT_MAP:     return 1;
	case THRIFT_STRUCT:  return 1;
	default:             return 0;
	}
}




void string_friendly(char s[], int n)
{
	ASSERT(s);
	// Temorary start
	for(int i = 0; i < n; ++i)
	{
		if (s[i] == '\0') {break;}
		if ((s[i] >= 32) && (s[i] <= 126)) {continue;}
		s[i] = '?';
	}
	// Temorary end
}

void thrift_get_field_str(thrift_type_t type, thrift_value_t value, char * buf, int n)
{
	ASSERT(buf);
	ASSERT(n >= 0);
	switch(type)
	{
	case THRIFT_STOP: snprintf(buf, n, "}"); break;
	case THRIFT_BOOLEAN_TRUE: snprintf(buf, n, "true"); break;
	case THRIFT_BOOLEAN_FALSE: snprintf(buf, n, "false"); break;
	case THRIFT_BYTE: snprintf(buf, n, "%02X", value.value_u8); break;
	case THRIFT_I16: snprintf(buf, n, "%jd", (intmax_t)value.value_i16); break;
	case THRIFT_I32: snprintf(buf, n, "%jd", (intmax_t)value.value_i32); break;
	case THRIFT_I64: snprintf(buf, n, "%jd", value.value_i64); break;
	case THRIFT_DOUBLE: snprintf(buf, n, "%f", value.value_f64); break;
	case THRIFT_BINARY:
		//string_friendly(value.string_data, value.string_size);
		snprintf(buf, n, "%.*s", value.string_size, value.string_data);
		break;
	case THRIFT_LIST: snprintf(buf, n, "%i of %s", value.list_size, thrift_get_type_string(value.list_type)); break;
	case THRIFT_SET: snprintf(buf, n, "SET"); break;
	case THRIFT_MAP: snprintf(buf, n, "MAP"); break;
	case THRIFT_STRUCT: snprintf(buf, n, "{"); break;
	default: snprintf(buf, n, "?"); break;
	}
}


uint8_t const * thrift_read_u8(uint8_t const *data, uint8_t * result)
{
	ASSERT(data);
	ASSERT(result);
	*result = data[0];
	data++;
	return data;
}


//TODO: Test if this work:
uint8_t const * thrift_read_f64(uint8_t const *data, double * result)
{
	ASSERT(data);
	ASSERT(result);
	union {
	uint64_t bits;
	uint8_t b[8];
	double v;
	} u;
	u.b[0] = data[0];
	u.b[1] = data[1];
	u.b[2] = data[2];
	u.b[3] = data[3];
	u.b[4] = data[4];
	u.b[5] = data[5];
	u.b[6] = data[6];
	u.b[7] = data[7];
	u.bits = bswap_64(u.bits);
	(*result) = u.v;
	data += 8;
	return data;
}


uint8_t const * thrift_read_varint_i64(uint8_t const *data, int64_t * result)
{
	ASSERT(data);
	ASSERT(result);
	uint8_t rsize = 0;
	uint8_t shift = 0;
	uint64_t val = 0;
	while(1)
	{
		uint8_t byte = data[rsize];
		rsize++;
		val |= (uint64_t)(byte & 0x7F) << shift;
		shift += 7;
		if (!(byte & 0x80))
		{
			(*result) = val;
			break;
		}
		// Have to check for invalid data so we don't crash.
		if (rsize >= 10)
		{
			LOG_ERROR("Variable-length int over 10 bytes.");
			return NULL;
		}
	}
	return data + rsize;
}



uint8_t const * thrift_read_varint(uint8_t const *data, int32_t bytes, void * result)
{
	ASSERT(data);
	ASSERT(result);
	//uint8_t const *data_start = data;
	int64_t val;
	data = thrift_read_varint_i64(data, &val);
	switch (bytes)
	{
	case 2:
		*(int16_t*)result = val;
		break;
	case 4:
		*(int32_t*)result = val;
		break;
	case 8:
		*(int64_t*)result = val;
		break;
	default:
		ASSERT(0);
		return NULL;
	}
	//ASSERT((data_start + bytes) == data);
	return data;
}












void pop(thrift_cursor_t * cursor)
{
	ASSERT(cursor);
	ASSERT(cursor->sp >= 0);
	/*
	cursor->stack[cursor->sp].id = 0;
	cursor->stack[cursor->sp].type = 0;
	cursor->stack[cursor->sp].list_type = 0;
	cursor->stack[cursor->sp].list_size = 0;
	*/
	cursor->sp--;
}


void thrift_cursor_init(thrift_cursor_t * cursor)
{
	ASSERT(cursor);
	cursor->sp = 0;
	cursor->stack = thrift_api.malloc_(cursor->stack_size);
	ASSERT(cursor->stack);
	cursor->stack[cursor->sp].id = 0;
	cursor->stack[cursor->sp].type = THRIFT_STRUCT;
	cursor->stack[cursor->sp].list_type = THRIFT_STOP;
	cursor->stack[cursor->sp].list_size = 0;
}


uint8_t const * thrift_cursor_next_type(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id)
{
	ASSERT(cursor);
	ASSERT(data);
	ASSERT(data_end);
	ASSERT(type);
	ASSERT(id);
	ASSERT(cursor->stack);

	uint8_t byte;
	uint8_t modifier;

	if(data >= (data_end))
	{
		LOG_ERROR_CURSOR(cursor, "No more data");
		return NULL;
	}

	switch (cursor->stack[cursor->sp].type)
	{
	// In the context of an struct there are fields we need to decode:
	case THRIFT_STRUCT:
		ASSERT(data <= data_end);
		
		// Decode field type:
		data = thrift_read_u8(data, &byte);
		modifier = (byte & 0xF0) >> 4;
		(*type) = byte & 0x0F;
		//TODO: Test these:
		//ASSERT((*type) != THRIFT_BOOLEAN_FALSE);
		//ASSERT((*type) != THRIFT_BOOLEAN_TRUE);
		//TODO: Add support for these:
		//ASSERT((*type) != THRIFT_DOUBLE);
		ASSERT((*type) != THRIFT_SET);
		ASSERT((*type) != THRIFT_MAP);
		ASSERT((*type) != THRIFT_UUID);

		// If the field type is STOP then this is the end of the struct:
		if((*type) == THRIFT_STOP)
		{
			break;
		}

		// Check if field id is large or small then use incrament delta:
		if(modifier == 0)
		{
			if(UNLIKELY(data >= (data_end)))
			{
				LOG_ERROR_CURSOR(cursor, "No more data");
				return NULL;
			}
			// The field id is large then we need to read again for an larger id:
			data = thrift_read_varint_i64(data, &cursor->last_field_id);
		}
		else
		{
			// The field id is small thus use incrament delta:
			cursor->last_field_id = cursor->last_field_id + modifier;
		}
		cursor->stack[cursor->sp].id = cursor->last_field_id;
		(*id) = cursor->last_field_id;
		break;


	// Step forward in list:
	case THRIFT_LIST:
		// Check if end of list:
		if(cursor->stack[cursor->sp].id >= cursor->stack[cursor->sp].list_size)
		{
			(*type) = THRIFT_STOP;
			break;
		}
		cursor->last_field_id = cursor->stack[cursor->sp].id;
		(*id) = cursor->stack[cursor->sp].id;
		// Iterate single element in the list:
		cursor->stack[cursor->sp].id++;
		(*type) = cursor->stack[cursor->sp].list_type;
		break;

	default:
		LOG_ERROR_CURSOR(cursor, "Invalid state");
		return NULL;
	}

	// Return the current position of thrift byte array:
	return data;
}




uint8_t const * thrift_cursor_next_value(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t type, thrift_value_t * value)
{
	ASSERT(cursor);
	ASSERT(data);
	ASSERT(data_end);
	ASSERT(value);
	ASSERT(cursor->stack);
	
	if((type == THRIFT_STOP) && (cursor->sp == 0))
	{
		// Decoding is successful and finnished here:
		return NULL;
	}

	if(UNLIKELY(data >= (data_end)))
	{
		LOG_ERROR_CURSOR(cursor, "No more data");
		return NULL;
	}

	switch (type)
	{
	case THRIFT_STOP:
		// We are done in this struct so we will pop out of this context:
		pop(cursor);
		cursor->last_field_id = cursor->stack[cursor->sp].id;
		break;

	// This will push new in the stack
	case THRIFT_STRUCT:
		cursor->last_field_id = 0;
		cursor->sp++;
		if(UNLIKELY(cursor->sp >= cursor->stack_size))
		{
			LOG_ERROR_CURSOR(cursor, "Stack overflow");
			return NULL;
		}
		cursor->stack[cursor->sp].type = THRIFT_STRUCT;
		break;
	
	// This will push new in the stack
	case THRIFT_LIST:{
		ASSERT(data <= data_end);
		uint8_t byte = 0;
		data = thrift_read_u8(data, &byte);
		value->list_type = byte & 0x0F;
		if(UNLIKELY(thrift_is_list_element_type(value->list_type) == 0))
		{
			LOG_ERROR_CURSOR(cursor, "List type is not valid");
			return NULL;
		}
		value->list_size = (byte >> 4) & 0x0F;
		// Check if list size is large:
		if(value->list_size == 0xF)
		{
			if(UNLIKELY(data >= (data_end)))
			{
				LOG_ERROR_CURSOR(cursor, "No more data");
				return NULL;
			}
			data = thrift_read_varint(data, sizeof(int32_t), &value->list_size);
		}
		cursor->sp++;
		if(UNLIKELY(cursor->sp >= cursor->stack_size))
		{
			LOG_ERROR_CURSOR(cursor, "Stack overflow");
			return NULL;
		}
		cursor->stack[cursor->sp].type = THRIFT_LIST;
		cursor->stack[cursor->sp].id = 0;
		//TODO: Try making a less bloated stack by reducing this:
		cursor->stack[cursor->sp].list_type = value->list_type;
		cursor->stack[cursor->sp].list_size = value->list_size;
		break;}
 
	case THRIFT_BOOLEAN_FALSE:
		value->value_bool = 0;
		break;

	case THRIFT_BOOLEAN_TRUE:
		value->value_bool = 1;
		break;

	// Decode primitive:
	case THRIFT_BYTE:
		ASSERT(data <= data_end);
		data = thrift_read_u8(data, &value->value_u8);
		break;

	// Decode primitive:
	case THRIFT_I16:
		ASSERT(data <= data_end);
		data = thrift_read_varint(data, sizeof(int16_t), &value->value_i16);
		value->value_i16 = (int16_t)ZIGZAG_TO_I32(value->value_i16);
		break;

	// Decode primitive:
	case THRIFT_I32:
		ASSERT(data <= data_end);
		data = thrift_read_varint(data, sizeof(int32_t), &value->value_i32);
		value->value_i32 = (int32_t)ZIGZAG_TO_I32(value->value_i32);
		break;

	// Decode primitive:
	case THRIFT_I64:
		ASSERT(data <= data_end);
		data = thrift_read_varint(data, sizeof(int64_t), &value->value_i64);
		value->value_i64 = (int64_t)ZIGZAG_TO_I64(value->value_i64);
		break;

	// Decode primitive:
	case THRIFT_DOUBLE:
		ASSERT(data <= data_end);
		data = thrift_read_f64(data, &value->value_f64);
		break;

	// Decode string:
	case THRIFT_BINARY:{
		int32_t string_size = 0;
		data = thrift_read_varint(data, sizeof(int32_t), &string_size);
		value->string_size = string_size;
		value->string_data = NULL;
		if(UNLIKELY(value->string_size < 0))
		{
			LOG_ERROR_CURSOR(cursor, "Negative string size");
			return NULL;
		}
		if(UNLIKELY(value->string_size >= THRIFT_MAX_STRING_SIZE))
		{
			LOG_ERROR_CURSOR(cursor, "Too big string size");
			return NULL;
		}
		if(value->string_size > 0)
		{
			value->string_data = thrift_api.malloc_(value->string_size + 1); // Allocate one extra for null-termination
			memcpy(value->string_data, data, value->string_size);
			value->string_data[value->string_size] = '\0';
			data += value->string_size;
		}
		break;}

	default:
		LOG_ERROR_CURSOR(cursor, "Invalid state");
		return NULL;
	}

	// Return the current position of thrift byte array:
	return data;
}



uint8_t const * thrift_cursor_next(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id, thrift_value_t * value)
{
	data = thrift_cursor_next_type(cursor, data, data_end, type, id);
	data = thrift_cursor_next_value(cursor, data, data_end, (*type), value);
	return data;
}
