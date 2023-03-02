#include "thrift.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc


thrift_api_t thrift_api = {
    .malloc_ = NULL
};


char const * thrift_get_type_string(uint32_t t)
{
	switch(t)
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

void thrift_get_field_str(int32_t type, thrift_value_t value, char * buf, int n)
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


int8_t const * thrift_read_varint_i64(int8_t const *data, int64_t * result)
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
			printf("Variable-length int over 10 bytes.");
			exit(1);
		}
	}
	//return new Int64(hi, lo);
	*result = (hi << 32) | (lo << 0);
	return data;
}


int8_t const * thrift_read_zigzag_i64(int8_t const *data, int64_t * result)
{
	data = thrift_read_varint_i64(data, result);
	*result = thrift_zigzag_to_i32(*result);
	return data;
}




void pop(thrift_cursor_t * ctx)
{
	ctx->stack_id[ctx->sp] = 0;
	ctx->stack_type[ctx->sp] = 0;
	ctx->stack_list_type[ctx->sp] = 0;
	ctx->stack_list_size[ctx->sp] = 0;
	ctx->sp--;
}


void thrift_cursor_init(thrift_cursor_t * ctx)
{
	ctx->sp = 0;
	ctx->stack_id[ctx->sp] = 0;
	ctx->stack_type[ctx->sp] = THRIFT_STRUCT;
	ctx->stack_list_type[ctx->sp] = THRIFT_STOP;
	ctx->stack_list_size[ctx->sp] = 0;
}



int thrift_cursor_read(thrift_cursor_t * ctx, int8_t const * data, int32_t data_length)
{
	int8_t const * current = data;
    thrift_value_t value = {0};
	uint8_t byte;
	uint8_t modifier;
	thrift_type_t type;

machine0:
	switch (ctx->stack_type[ctx->sp])
	{
	case THRIFT_STRUCT:
		//The start of a struct
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_STRUCT, value);
		ctx->last_field_id = 0;
		break;
	
	case THRIFT_LIST:
		//The start of a list
		if(current >= (data+data_length)){goto error_no_more_data;}
		current = thrift_read_u8(current, &byte);
		value.list_type = byte & 0x0F;
		value.list_size = (byte >> 4) & 0x0F;
		if(value.list_size == 0xF)
		{
			if(current >= (data+data_length)){goto error_no_more_data;}
			int64_t list_size;
			current = thrift_read_varint_i64(current, &list_size);
			value.list_size = list_size;
		}
		ctx->cb_field(ctx, ctx->last_field_id, type, value);
		ctx->stack_list_type[ctx->sp] = value.list_type;
		ctx->stack_list_size[ctx->sp] = value.list_size;
		ctx->stack_id[ctx->sp] = 0;
		break;

	default:
		// Nothing to do here.
		break;
	}
	goto machine_branch;



machine_branch:
	switch (ctx->stack_type[ctx->sp])
	{
	case THRIFT_I32:
		if(current >= (data+data_length)){goto error_no_more_data;}
		current = thrift_read_zigzag_i64(current, &value.value_i64);
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_I32, value);
		pop(ctx);
		goto machine_branch;

	case THRIFT_I64:
		if(current >= (data+data_length)){goto error_no_more_data;}
		current = thrift_read_zigzag_i64(current, &value.value_i64);
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_I64, value);
		pop(ctx);
		goto machine_branch;

	case THRIFT_BINARY:
		if(current >= (data+data_length)){goto error_no_more_data;}
		current = thrift_read_varint_i64(current, &value.value_i64);
		value.string_data = NULL;
		if(value.string_size < 0){goto error_invalid_state;}
		if(value.string_size >= THRIFT_MAX_STRING_SIZE){goto error_invalid_state;}
		if(value.string_size > 0)
		{
			value.string_data = thrift_api.malloc_(value.string_size + 1); // Allocate one extra for null-termination
			memcpy(value.string_data, current, value.string_size);
			value.string_data[value.string_size] = '\0';
			current += value.string_size;
		}
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_BINARY, value);
		pop(ctx);
		goto machine_branch;

	case THRIFT_STRUCT:
		// Reading a type of a struct member:
		if(current >= (data+data_length)){goto error_no_more_data;}
		current = thrift_read_u8(current, &byte);
		modifier = (byte & 0xF0) >> 4;
		type = byte & 0x0F;
		if(type == THRIFT_STOP)
		{
			// The end of a struct:
			if(ctx->sp == 0)
			{
				// The end of thrift data:
				goto success;
			}
			ctx->cb_field(ctx, ctx->last_field_id, THRIFT_STOP, value);
			ctx->last_field_id = ctx->stack_id[ctx->sp];
			pop(ctx);
			goto machine_branch;
		}
		if (modifier == 0)
		{
			if(current >= (data+data_length)){goto error_no_more_data;}
			current = thrift_read_varint_i64(current, &ctx->last_field_id);
		}
		else
		{
			ctx->last_field_id = ctx->last_field_id + modifier;
		}
		ctx->stack_id[ctx->sp] = ctx->last_field_id;
		ctx->sp++;
		if(ctx->sp > THRIFT_STACK_MAX_SIZE){goto error_stack_overflow;}
		// Start to decode the member value depending on which type:
		ctx->stack_type[ctx->sp] = type;
		goto machine0;

	case THRIFT_LIST:
		if(ctx->stack_id[ctx->sp] >= ctx->stack_list_size[ctx->sp])
		{
			//The end of a list
			pop(ctx);
			ctx->last_field_id = ctx->stack_id[ctx->sp];
			goto machine_branch;
		}
		ctx->last_field_id = ctx->stack_id[ctx->sp];
		ctx->stack_id[ctx->sp]++;
		ctx->sp++;
		if(ctx->sp > THRIFT_STACK_MAX_SIZE){goto error_stack_overflow;}
		ctx->stack_type[ctx->sp] = ctx->stack_list_type[ctx->sp-1];
		goto machine0;

	default:
		goto error_invalid_state;
	}

success:
	printf("Success!\n");
	return 0;

error_no_more_data:
	printf("No more data is available!\n");
	return -1;

error_invalid_state:
	printf("[ERROR] Invalid state!\n");
	return -1;

error_stack_overflow:
	printf("[ERROR] Stack overflow!\n");
	return -1;

}
