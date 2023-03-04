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

int64_t thrift_zigzag_to_i32(uint64_t n)
{
	return (int) (n >> 1) ^ -(int) (n & 1);
}


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
	case THRIFT_BINARY:        return "BINARY";
	case THRIFT_LIST:          return "LIST";
	case THRIFT_SET:           return "SET";
	case THRIFT_MAP:           return "MAP";
	case THRIFT_STRUCT:        return "STRUCT";
	default:                   return "";
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
	case THRIFT_STOP: break;
	case THRIFT_BOOLEAN_TRUE: snprintf(buf, n, "true"); break;
	case THRIFT_BOOLEAN_FALSE: snprintf(buf, n, "false"); break;
	case THRIFT_BYTE: snprintf(buf, n, "%02X", value.value_u8); break;
	case THRIFT_I16: snprintf(buf, n, "%jd", value.value_i16); break;
	case THRIFT_I32: snprintf(buf, n, "%jd", value.value_i32); break;
	case THRIFT_I64: snprintf(buf, n, "%jd", value.value_i64); break;
	case THRIFT_DOUBLE: snprintf(buf, n, "%f", value.value_i64); break;
	case THRIFT_BINARY:
		//string_friendly(value.string_data, value.string_size);
		snprintf(buf, n, "%.*s", value.string_size, value.string_data);
		break;
	case THRIFT_LIST: snprintf(buf, n, "%i of %s", value.list_size, thrift_get_type_string(value.list_type)); break;
	case THRIFT_SET: snprintf(buf, n, ""); break;
	case THRIFT_MAP: snprintf(buf, n, ""); break;
	case THRIFT_STRUCT: snprintf(buf, n, ""); break;
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





int8_t const * thrift_read_varint_i64(uint8_t const *data, int64_t * result)
{
	ASSERT(data);
	ASSERT(result);
	int rsize = 0;
	int lo = 0;
	int64_t hi = 0;
	int shift = 0;
	while (1)
	{
		uint8_t b = data[0];
		data++;
		rsize ++;
		if (shift <= 25)
		{
			lo = lo | ((b & 0x7f) << shift);
		}
		else if (25 < shift && shift < 32)
		{
			lo = lo | ((b & 0x7f) << shift);
			hi = hi | ((b & 0x7f) >> (32-shift));
		}
		else
		{
			hi = hi | ((b & 0x7f) << (shift-32));
		}
		shift += 7;
		if (!(b & 0x80)){break;}
		if (rsize >= 10)
		{
			LOG_ERROR("Variable-length int over 10 bytes.");
			return NULL;
		}
	}
	//return new Int64(hi, lo);
	(*result) = (hi << 32) | (lo << 0);
	return data;
}

int8_t const * thrift_read_varint_i32(uint8_t const *data, int32_t * result)
{
	ASSERT(data);
	ASSERT(result);
	int64_t r64;
	data = thrift_read_varint_i64(data, &r64);
	(*result) = r64;
	return data;
}

int8_t const * thrift_read_varint_i16(uint8_t const *data, int16_t * result)
{
	ASSERT(data);
	ASSERT(result);
	int64_t r64;
	data = thrift_read_varint_i64(data, &r64);
	(*result) = r64;
	return data;
}





void pop(thrift_cursor_t * cursor)
{
	ASSERT(cursor);
	ASSERT(cursor->sp >= 0);
	cursor->stack[cursor->sp].id = 0;
	cursor->stack[cursor->sp].type = 0;
	cursor->stack[cursor->sp].list_type = 0;
	cursor->stack[cursor->sp].list_size = 0;
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

int thrift_is_list_type(thrift_type_t type)
{
	switch (type)
	{
	case THRIFT_BYTE: return 1;
	case THRIFT_I16: return 1;
	case THRIFT_I32: return 1;
	case THRIFT_I64: return 1;
	case THRIFT_DOUBLE: return 1;
	case THRIFT_BINARY: return 1;
	case THRIFT_LIST: return 1;
	case THRIFT_SET: return 1;
	case THRIFT_MAP: return 1;
	case THRIFT_STRUCT: return 1;
	default:return 0;
	}
}


uint8_t const * thrift_cursor_read_value(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_value_t * value)
{
	ASSERT(cursor);
	ASSERT(data);
	ASSERT(data_end);
	ASSERT(value);
	ASSERT(cursor->stack);
	
	if(UNLIKELY(data >= (data_end)))
	{
		LOG_ERROR_CURSOR(cursor, "No more data");
		return NULL;
	}

	uint8_t byte;
	switch (cursor->stack[cursor->sp].type)
	{
	// The type of the value is struct which can not be decoded here.
	// Keep this in the stack so we can start decoding its fields in future iterations.
	case THRIFT_STRUCT:
		cursor->last_field_id = 0;
		break;
	
	// The value type is a list so start decoding the list size and list type:
	// Keep this in the stack so we can start decoding its elements in future iterations.
	case THRIFT_LIST:
		ASSERT(data <= data_end);
		data = thrift_read_u8(data, &byte);
		value->list_type = byte & 0x0F;
		if(UNLIKELY(thrift_is_list_type(value->list_type) == 0))
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
			data = thrift_read_varint_i32(data, &value->list_size);
		}
		cursor->stack[cursor->sp].list_type = value->list_type;
		cursor->stack[cursor->sp].list_size = value->list_size;
		cursor->stack[cursor->sp].id = 0;
		break;

	// Decode primitive:
	case THRIFT_BYTE:
		ASSERT(data <= data_end);
		data = thrift_read_u8(data, &value->value_u8);
		pop(cursor);
		break;

	// Decode primitive:
	case THRIFT_I16:
		ASSERT(data <= data_end);
		data = thrift_read_varint_i16(data, &value->value_i16);
		value->value_i16 = (int16_t)ZIGZAG_TO_I32(value->value_i16);
		pop(cursor);
		break;

	// Decode primitive:
	case THRIFT_I32:
		ASSERT(data <= data_end);
		data = thrift_read_varint_i32(data, &value->value_i32);
		value->value_i32 = (int32_t)ZIGZAG_TO_I32(value->value_i32);
		pop(cursor);
		break;

	// Decode primitive:
	case THRIFT_I64:
		ASSERT(data <= data_end);
		data = thrift_read_varint_i64(data, &value->value_i64);
		value->value_i64 = (int64_t)ZIGZAG_TO_I64(value->value_i64);
		pop(cursor);
		break;

	// Decode string:
	case THRIFT_BINARY:
		data = thrift_read_varint_i32(data, &value->value_i32);
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
		pop(cursor);
		break;

	default:
		// Nothing to do here.
		break;
	}

	// Return the current position of thrift byte array:
	return data;
}


uint8_t const * thrift_cursor_read_type(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id)
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
		//TODO: Add support for these:
		ASSERT((*type) != THRIFT_BOOLEAN_FALSE);
		ASSERT((*type) != THRIFT_BOOLEAN_TRUE);
		ASSERT((*type) != THRIFT_DOUBLE);
		ASSERT((*type) != THRIFT_SET);
		ASSERT((*type) != THRIFT_MAP);

		// If the field type is STOP then this is the end of the struct:
		if((*type) == THRIFT_STOP)
		{
			// If this is the end of thrift data:
			if(cursor->sp == 0)
			{
				// Decoding is successful and finnished here:
				return NULL;
			}
			cursor->last_field_id = cursor->stack[cursor->sp].id;
			// We are done in this struct so we will pop out of this context:
			pop(cursor);
			break;
		}

		// Check if field id is large or small then use incrament delta:
		if (modifier == 0)
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
		// Push new field on to stack:
		cursor->sp++;
		if(UNLIKELY(cursor->sp >= cursor->stack_size))
		{
			LOG_ERROR_CURSOR(cursor, "Stack overflow");
			return NULL;
		}
		cursor->stack[cursor->sp].type = (*type);
		// After this we neeed to decode the field value depending on which type:
		break;


	// Step forward in list:
	case THRIFT_LIST:
		// Check if end of list:
		if(cursor->stack[cursor->sp].id >= cursor->stack[cursor->sp].list_size)
		{
			// The end of a list, pop out of list context:
			pop(cursor);
			// Restore last field id:
			cursor->last_field_id = cursor->stack[cursor->sp].id;
			break;
		}
		cursor->last_field_id = cursor->stack[cursor->sp].id;
		(*id) = cursor->stack[cursor->sp].id;
		// Iterate single element in the list:
		cursor->stack[cursor->sp].id++;
		// Push new context for next element:
		cursor->sp++;
		if(UNLIKELY(cursor->sp >= cursor->stack_size))
		{
			LOG_ERROR_CURSOR(cursor, "Stack overflow");
			return NULL;
		}
		cursor->stack[cursor->sp].type = cursor->stack[cursor->sp-1].list_type;
		(*type) = cursor->stack[cursor->sp].type;
		break;

	default:
		LOG_ERROR_CURSOR(cursor, "Invalid state");
		return NULL;
	}

	// Return the current position of thrift byte array:
	return data;
}
