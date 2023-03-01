#include "thrift.h"
#include <stdint.h>
#include <ctype.h>
#include "flecs.h"

// https://github.com/apache/thrift/blob/master/lib/cpp/src/thrift/protocol/TCompactProtocol.tcc

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


uint8_t thrift_read_u8(thrift_reader_t * reader)
{
	uint8_t b = reader->data_current[0];
	reader->data_current++;
	return b;
}


int64_t thrift_zigzag_to_i32(uint64_t n)
{
	return (int) (n >> 1) ^ -(int) (n & 1);
}


int64_t thrift_read_varint_i64(thrift_reader_t * reader)
{
	int rsize = 0;
	int lo = 0;
	int64_t hi = 0;
	int shift = 0;
	while (1)
	{
		uint8_t b = thrift_read_u8(reader);
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
	return (hi << 32) | (lo << 0);
}


int64_t thrift_read_zigzag_i64(thrift_reader_t * reader)
{
	int64_t value = thrift_read_varint_i64(reader);
	return thrift_zigzag_to_i32(value);
}




void pop(thrift_stack_t * ctx)
{
	ctx->stack_id[ctx->sp] = 0;
	ctx->stack_type[ctx->sp] = 0;
	ctx->sp--;
}






void read_primitive(thrift_stack_t * ctx, int32_t n, thrift_t type)
{
	thrift_value_t value;
	for(int32_t i = 0; i < n; ++i)
	{
		switch (type)
		{
		case THRIFT_I32:
			value.value_i64 = thrift_read_zigzag_i64(&ctx->reader);
			ctx->cb_field(ctx, ctx->last_field_id, THRIFT_I32, value);
			break;

		case THRIFT_I64:
			value.value_i64 = thrift_read_zigzag_i64(&ctx->reader);
			ctx->cb_field(ctx, ctx->last_field_id, THRIFT_I64, value);
			break;
		}
	}
}



















void thrift_stacked_read(thrift_stack_t * ctx)
{
	if(ctx->reader.data_current >= ctx->reader.data_end){goto success_no_more_data;}
    thrift_value_t value = {0};
	uint8_t byte;
	uint8_t modifier;
	thrift_t type;

	ctx->sp = 0;
	ctx->stack_type[ctx->sp] = THRIFT_STRUCT;





machine_branch:
	switch (ctx->stack_type[ctx->sp])
	{
	case THRIFT_I32:
		value.value_i64 = thrift_read_zigzag_i64(&ctx->reader);
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_I32, value);
		pop(ctx);
		goto machine_branch;

	case THRIFT_I64:
		value.value_i64 = thrift_read_zigzag_i64(&ctx->reader);
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_I64, value);
		pop(ctx);
		goto machine_branch;

	case THRIFT_BINARY:
		value.string_size = thrift_read_varint_i64(ctx);
		value.string_data = NULL;
		if(value.string_size < 0){goto error_invalid_state;}
		if(value.string_size > 0)
		{
			value.string_size = value.string_size < 100 ? value.string_size : 100; // temprary safeguard
			value.string_data = ecs_os_malloc(value.string_size+1);
			memcpy(value.string_data, ctx->reader.data_current, value.string_size);
			value.string_data[value.string_size] = '\0';
			ctx->reader.data_current += value.string_size;
		}
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_BINARY, value);
		pop(ctx);
		goto machine_branch;

	case THRIFT_STRUCT:
		byte = thrift_read_u8(&ctx->reader);
		modifier = (byte & 0xF0) >> 4;
		type = byte & 0x0F;
		if(type == THRIFT_STOP)
		{
			ctx->cb_field(ctx, ctx->last_field_id, THRIFT_STOP, value);
			pop(ctx);
			goto machine_branch;
		}
		if (modifier == 0)
		{
			ctx->last_field_id = thrift_read_varint_i64(&ctx->reader);
		}
		else
		{
			ctx->last_field_id = ctx->last_field_id + modifier;
		}
		ctx->sp++;
		ctx->stack_type[ctx->sp] = type;
		ctx->stack_id[ctx->sp] = ctx->last_field_id;
		goto machine0;

	case THRIFT_LIST:
		if(ctx->stack_list_repeat[ctx->sp] == 0)
		{
			pop(ctx);
			goto machine_branch;
		}
		ctx->stack_list_repeat[ctx->sp]--;
		ctx->sp++;
		ctx->stack_type[ctx->sp] = ctx->stack_list_type[ctx->sp-1];
		goto machine0;

	}

machine0:
	switch (ctx->stack_type[ctx->sp])
	{
	case THRIFT_STRUCT:
		ctx->cb_field(ctx, ctx->last_field_id, THRIFT_STRUCT, value);
		ctx->last_field_id = 0;
		break;
	
	case THRIFT_LIST:
		byte = thrift_read_u8(&ctx->reader);
		value.list_type = byte & 0x0F;
		value.list_size = (byte >> 4) & 0x0F;
		if(ctx->reader.data_current >= ctx->reader.data_end){goto success_no_more_data;}
		if(value.list_size == 0xF)
		{
			value.list_size = thrift_read_varint_i64(&ctx->reader);
		}
		ctx->cb_field(ctx, ctx->last_field_id, type, value);
		ctx->stack_list_type[ctx->sp] = value.list_type;
		ctx->stack_list_repeat[ctx->sp] = value.list_size;
		break;
	}
	goto machine_branch;

success_no_more_data:
	printf("No more data is available!\n");
	return;

error_invalid_state:
	printf("[ERROR] Invalid state!\n");
	return;

error_stack_overflow:
	printf("[ERROR] Stack overflow!\n");
	return;

}
