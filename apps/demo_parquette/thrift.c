#include "thrift.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>


// https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc




thrift_api_t thrift_api = {
    .malloc_ = NULL,
    .onerror_ = NULL,
};

#define LOG_ERROR(s) thrift_api.onerror_(s)
#define LOG_ERROR_CURSOR(cursor, s) thrift_api.onerror_(s)


char const * thrift_get_type_string(thrift_type_t type)
{
	switch(type)
	{
	case THRIFT_STOP: return "STOP";
	case THRIFT_BOOLEAN_TRUE: return "BOOLEAN_TRUE";
	case THRIFT_BOOLEAN_FALSE: return "BOOLEAN_FALSE";
	case THRIFT_BYTE: return "BYTE";
	case THRIFT_I16: return "I16";
	case THRIFT_I32: return "I32";
	case THRIFT_I64: return "I64";
	case THRIFT_DOUBLE: return "DOUBLE";
	case THRIFT_BINARY: return "BINARY";
	case THRIFT_LIST: return "LIST";
	case THRIFT_SET: return "SET";
	case THRIFT_MAP: return "MAP";
	case THRIFT_STRUCT: return "STRUCT";
	default: return "";
	}
}


void string_friendly(char s[], int n)
{
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
	switch(type)
	{
	case THRIFT_STOP: break;
	case THRIFT_BOOLEAN_TRUE: snprintf(buf, n, "true"); break;
	case THRIFT_BOOLEAN_FALSE: snprintf(buf, n, "false"); break;
	case THRIFT_BYTE: snprintf(buf, n, "%02X", value.value_u64); break;
	case THRIFT_I16: snprintf(buf, n, "%jd", value.value_i64); break;
	case THRIFT_I32: snprintf(buf, n, "%jd", value.value_i64); break;
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


int8_t const * thrift_read_u8(int8_t const *data, int8_t * result)
{
	*result = data[0];
	data++;
	return data;
}


int64_t thrift_zigzag_to_i32(uint64_t n)
{
	return (int) (n >> 1) ^ -(int) (n & 1);
}


int8_t const * thrift_read_varint_i64(uint8_t const *data, int64_t * result)
{
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
	*result = (hi << 32) | (lo << 0);
	return data;
}


uint8_t const * thrift_read_zigzag_i64(uint8_t const *data, int64_t * result)
{
	data = thrift_read_varint_i64(data, result);
	*result = thrift_zigzag_to_i32(*result);
	return data;
}




void pop(thrift_cursor_t * cursor)
{
	cursor->stack_id[cursor->sp] = 0;
	cursor->stack_type[cursor->sp] = 0;
	cursor->stack_list_type[cursor->sp] = 0;
	cursor->stack_list_size[cursor->sp] = 0;
	cursor->sp--;
}


void thrift_cursor_init(thrift_cursor_t * cursor)
{
	cursor->sp = 0;
	cursor->stack_id[cursor->sp] = 0;
	cursor->stack_type[cursor->sp] = THRIFT_STRUCT;
	cursor->stack_list_type[cursor->sp] = THRIFT_STOP;
	cursor->stack_list_size[cursor->sp] = 0;
}

uint8_t const * thrift_cursor_read_value(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_value_t * value)
{
	uint8_t byte;
	switch (cursor->stack_type[cursor->sp])
	{
	// The type of the value is struct which can not be decoded here.
	// Keep this in the stack so we can start decoding its fields in future iterations.
	case THRIFT_STRUCT:
		cursor->last_field_id = 0;
		break;
	
	// The value type is a list so start decoding the list size and list type:
	// Keep this in the stack so we can start decoding its elements in future iterations.
	case THRIFT_LIST:
		if(data >= (data_end))
		{
			LOG_ERROR_CURSOR(cursor, "No more data");
			return NULL;
		}
		data = thrift_read_u8(data, &byte);
		value->list_type = byte & 0x0F;
		value->list_size = (byte >> 4) & 0x0F;
		if(value->list_size == 0xF)
		{
			if(data >= (data_end))
			{
				LOG_ERROR_CURSOR(cursor, "No more data");
				return NULL;
			}
			int64_t list_size;
			data = thrift_read_varint_i64(data, &list_size);
			value->list_size = list_size;
		}
		cursor->stack_list_type[cursor->sp] = value->list_type;
		cursor->stack_list_size[cursor->sp] = value->list_size;
		cursor->stack_id[cursor->sp] = 0;
		break;

	// Decode primitive:
	case THRIFT_I32:
		if(data >= (data_end))
		{
			LOG_ERROR_CURSOR(cursor, "No more data");
			return NULL;
		}
		data = thrift_read_zigzag_i64(data, &value->value_i64);
		pop(cursor);
		break;

	// Decode primitive:
	case THRIFT_I64:
		if(data >= (data_end))
		{
			LOG_ERROR_CURSOR(cursor, "No more data");
			return NULL;
		}
		data = thrift_read_zigzag_i64(data, &value->value_i64);
		pop(cursor);
		break;

	// Decode string:
	case THRIFT_BINARY:
		if(data >= (data_end))
		{
			LOG_ERROR_CURSOR(cursor, "No more data");
			return NULL;
		}
		data = thrift_read_varint_i64(data, &value->value_i64);
		value->string_data = NULL;
		if(value->string_size < 0)
		{
			LOG_ERROR_CURSOR(cursor, "Negative string size");
			return NULL;
		}
		if(value->string_size >= THRIFT_MAX_STRING_SIZE)
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

	return data;
}


uint8_t const * thrift_cursor_read_type(thrift_cursor_t * cursor, uint8_t const * data, uint8_t const * data_end, thrift_type_t * type, int64_t * id)
{
	uint8_t byte;
	uint8_t modifier;

	switch (cursor->stack_type[cursor->sp])
	{
	// Decode struct member type:
	case THRIFT_STRUCT:
		if(data >= (data_end))
		{
			LOG_ERROR_CURSOR(cursor, "No more data");
			return NULL;
		}
		data = thrift_read_u8(data, &byte);
		modifier = (byte & 0xF0) >> 4;
		(*type) = byte & 0x0F;
		if((*type) == THRIFT_STOP)
		{
			// The end of a struct:
			if(cursor->sp == 0)
			{
				// The end of thrift data:
				return NULL;
			}
			cursor->last_field_id = cursor->stack_id[cursor->sp];
			pop(cursor);
			break;
		}
		if (modifier == 0)
		{
			if(data >= (data_end))
			{
				LOG_ERROR_CURSOR(cursor, "No more data");
				return NULL;
			}
			data = thrift_read_varint_i64(data, &cursor->last_field_id);
		}
		else
		{
			cursor->last_field_id = cursor->last_field_id + modifier;
		}
		cursor->stack_id[cursor->sp] = cursor->last_field_id;
		*id = cursor->last_field_id;
		// Push new member on to stack:
		cursor->sp++;
		if(cursor->sp > THRIFT_STACK_MAX_SIZE)
		{
			LOG_ERROR_CURSOR(cursor, "Stack overflow");
			return NULL;
		}
		cursor->stack_type[cursor->sp] = *type;
		// After this we neeed to decode the member value depending on which type:
		break;


	// Step forward in list:
	case THRIFT_LIST:
		// Check if end of list:
		if(cursor->stack_id[cursor->sp] >= cursor->stack_list_size[cursor->sp])
		{
			// The end of a list, pop out of list context:
			pop(cursor);
			// Restore last field id:
			cursor->last_field_id = cursor->stack_id[cursor->sp];
			break;
		}
		cursor->last_field_id = cursor->stack_id[cursor->sp];
		*id = cursor->stack_id[cursor->sp];
		// Iterate single element in the list:
		cursor->stack_id[cursor->sp]++;
		// Push new context for next element:
		cursor->sp++;
		if(cursor->sp > THRIFT_STACK_MAX_SIZE)
		{
			LOG_ERROR_CURSOR(cursor, "Stack overflow");
			return NULL;
		}
		cursor->stack_type[cursor->sp] = cursor->stack_list_type[cursor->sp-1];
		(*type) = cursor->stack_type[cursor->sp];
		break;

	default:
		LOG_ERROR_CURSOR(cursor, "Invalid state");
		return NULL;
	}

	return data;
}
