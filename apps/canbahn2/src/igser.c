#include "igser.h"

#include <stdlib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <egimgui.h>

static int flecs_expr_ser_type_ops(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, char const *label, int32_t in_array, bool is_expr);
static int flecs_expr_ser_type_op(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *ptr, char const *label, bool is_expr);
static int expr_ser_elements(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, int32_t elem_count, int32_t elem_size, char const *label, bool is_array);

static const char *flecs_meta_op_kind_str(ecs_meta_type_op_kind_t kind)
{
	switch (kind) {
	case EcsOpEnum:
		return "Enum";
	case EcsOpBitmask:
		return "Bitmask";
	case EcsOpArray:
		return "Array";
	case EcsOpVector:
		return "Vector";
	case EcsOpOpaque:
		return "Opaque";
	case EcsOpPush:
		return "Push";
	case EcsOpPop:
		return "Pop";
	case EcsOpPrimitive:
		return "Primitive";
	case EcsOpBool:
		return "Bool";
	case EcsOpChar:
		return "Char";
	case EcsOpByte:
		return "Byte";
	case EcsOpU8:
		return "U8";
	case EcsOpU16:
		return "U16";
	case EcsOpU32:
		return "U32";
	case EcsOpU64:
		return "U64";
	case EcsOpI8:
		return "I8";
	case EcsOpI16:
		return "I16";
	case EcsOpI32:
		return "I32";
	case EcsOpI64:
		return "I64";
	case EcsOpF32:
		return "F32";
	case EcsOpF64:
		return "F64";
	case EcsOpUPtr:
		return "UPtr";
	case EcsOpIPtr:
		return "IPtr";
	case EcsOpString:
		return "String";
	case EcsOpEntity:
		return "Entity";
	case EcsOpId:
		return "Id";
	case EcsOpScope:
		return "Scope";
	default:
		return "<< invalid kind >>";
	}
}

static ecs_primitive_kind_t flecs_expr_op_to_primitive_kind(ecs_meta_type_op_kind_t kind) {
    return kind - EcsOpPrimitive;
}

static int flecs_expr_ser_enum(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, char const * label)
{
	const EcsEnum *enum_type = ecs_get(world, op->type, EcsEnum);
	ecs_check(enum_type != NULL, ECS_INVALID_PARAMETER, NULL);

	int32_t val = *(const int32_t *)base;

	/* Enumeration constants are stored in a map that is keyed on the
	 * enumeration value. */
	ecs_enum_constant_t *c = ecs_map_get_deref(&enum_type->constants,
	ecs_enum_constant_t, (ecs_map_key_t)val);
	if (!c) {
		char *path = ecs_get_path(world, op->type);
		ecs_err("value %d is not valid for enum type '%s'", val, path);
		ecs_os_free(path);
		goto error;
	}

	//igInputText(item->label, item->input.data, item->input.data_size, 0, 0, 0);
	char const * lab = ecs_get_name(world, op->type);
	igLabelText(lab, "%s", ecs_get_name(world, c->constant));

	//ecs_strbuf_appendstr(str, ecs_get_name(world, c->constant));

	return 0;
error:
	return -1;
}

/* Serialize bitmask */
static int flecs_expr_ser_bitmask(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *ptr, char const * label)
{
	const EcsBitmask *bitmask_type = ecs_get(world, op->type, EcsBitmask);
	ecs_check(bitmask_type != NULL, ECS_INVALID_PARAMETER, NULL);
	uint32_t value = *(const uint32_t *)ptr;

	//ecs_strbuf_list_push(str, "", "|");

	/* Multiple flags can be set at a given time. Iterate through all the flags
	 * and append the ones that are set. */
	ecs_map_iter_t it = ecs_map_iter(&bitmask_type->constants);
	int count = 0;
	while (ecs_map_next(&it)) {
		ecs_bitmask_constant_t *c = ecs_map_ptr(&it);
		ecs_map_key_t key = ecs_map_key(&it);
		if ((value & key) == key) {
			//ecs_strbuf_list_appendstr(str, ecs_get_name(world, c->constant));
			count++;
			value -= (uint32_t)key;
		}
	}

	if (value != 0) {
		/* All bits must have been matched by a constant */
		char *path = ecs_get_path(world, op->type);
		ecs_err(
		"value for bitmask %s contains bits (%u) that cannot be mapped to constant",
		path, value);
		ecs_os_free(path);
		goto error;
	}

	if (!count) {
		//ecs_strbuf_list_appendstr(str, "0");
	}

	//ecs_strbuf_list_pop(str, "");

	return 0;
error:
	return -1;
}

static int expr_ser_type_elements(const ecs_world_t *world, ecs_entity_t type, const void *base, int32_t elem_count, char const *label, bool is_array)
{
	const EcsTypeSerializer *ser = ecs_get(
	world, type, EcsTypeSerializer);
	ecs_assert(ser != NULL, ECS_INTERNAL_ERROR, NULL);

	const EcsComponent *comp = ecs_get(world, type, EcsComponent);
	ecs_assert(comp != NULL, ECS_INTERNAL_ERROR, NULL);

	ecs_meta_type_op_t *ops = ecs_vec_first_t(&ser->ops, ecs_meta_type_op_t);
	int32_t op_count = ecs_vec_count(&ser->ops);
	return expr_ser_elements(world, ops, op_count, base, elem_count, comp->size, label, is_array);
}

static int expr_ser_array(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *ptr, char const * label)
{
	const EcsArray *a = ecs_get(world, op->type, EcsArray);
	ecs_assert(a != NULL, ECS_INTERNAL_ERROR, NULL);
	return expr_ser_type_elements(world, a->type, ptr, a->count, label, true);
}

static int expr_ser_vector(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *base, char const * label)
{
	const ecs_vec_t *value = base;
	const EcsVector *v = ecs_get(world, op->type, EcsVector);
	ecs_assert(v != NULL, ECS_INTERNAL_ERROR, NULL);

	int32_t count = ecs_vec_count(value);
	void *array = ecs_vec_first(value);

	/* Serialize contiguous buffer of vector */
	return expr_ser_type_elements(world, v->type, array, count, label, false);
}

int flecs_expr_ser_primitive(const ecs_world_t *world, ecs_primitive_kind_t kind, const void *base, char const *label, bool is_expr)
{
	switch (kind) {
	case EcsBool:
		if (*(const bool *)base) {
			//ecs_strbuf_appendlit(str, "true");
		} else {
			//ecs_strbuf_appendlit(str, "false");
		}
		break;
	case EcsChar: {
		/*
		char chbuf[3];
		char ch = *(const char *)base;
		if (ch) {
			flecs_chresc(chbuf, *(const char *)base, '"');
			if (is_expr)
				ecs_strbuf_appendch(str, '"');
			ecs_strbuf_appendstr(str, chbuf);
			if (is_expr)
				ecs_strbuf_appendch(str, '"');
		} else {
			ecs_strbuf_appendch(str, '0');
		}
		*/
		break;
	}
	case EcsByte:
		// ecs_strbuf_appendint(str, flecs_uto(int64_t, *(const uint8_t*)base));
		break;
	case EcsU8:
		// ecs_strbuf_appendint(str, flecs_uto(int64_t, *(const uint8_t*)base));
		break;
	case EcsU16:
		// ecs_strbuf_appendint(str, flecs_uto(int64_t, *(const uint16_t*)base));
		break;
	case EcsU32:
		// ecs_strbuf_appendint(str, flecs_uto(int64_t, *(const uint32_t*)base));
		break;
	case EcsU64:
		// ecs_strbuf_append(str, "%llu", *(const uint64_t*)base);
		break;
	case EcsI8:
		// ecs_strbuf_appendint(str, flecs_ito(int64_t, *(const int8_t*)base));
		break;
	case EcsI16:
		// ecs_strbuf_appendint(str, flecs_ito(int64_t, *(const int16_t*)base));
		break;
	case EcsI32:
		// ecs_strbuf_appendint(str, flecs_ito(int64_t, *(const int32_t*)base));
		break;
	case EcsI64:
		// ecs_strbuf_appendint(str, *(const int64_t*)base);
		break;
	case EcsF32:
		// ecs_strbuf_appendflt(str, (double)*(const float*)base, 0);
		break;
	case EcsF64:
		igInputDouble(label, (double*)base, 0, 0, "%f", 0);
		// ecs_strbuf_appendflt(str, *(const double*)base, 0);
		break;
	case EcsIPtr:
		// ecs_strbuf_appendint(str, flecs_ito(int64_t, *(const intptr_t*)base));
		break;
	case EcsUPtr:
		// ecs_strbuf_append(str, "%u", *(const uintptr_t*)base);
		break;
	case EcsString: {
		/*
		const char *value = *ECS_CONST_CAST(const char **, base);
		if (value) {
			if (!is_expr) {
				//ecs_strbuf_appendstr(str, value);
			} else {
				//ecs_size_t length = flecs_stresc(NULL, 0, '"', value);
				if (length == ecs_os_strlen(value)) {
					//ecs_strbuf_appendch(str, '"');
					//ecs_strbuf_appendstrn(str, value, length);
					//ecs_strbuf_appendch(str, '"');
				} else {
					char *out = ecs_os_malloc(length + 3);
					flecs_stresc(out + 1, length, '"', value);
					out[0] = '"';
					out[length + 1] = '"';
					out[length + 2] = '\0';
					ecs_strbuf_appendstr(str, out);
					ecs_os_free(out);
				}
			}
		} else {
			ecs_strbuf_appendlit(str, "null");
		}
		*/
		break;
	}
	case EcsEntity: {
		ecs_entity_t e = *(const ecs_entity_t *)base;
		if (!e) {
			//ecs_strbuf_appendlit(str, "#0");
		} else {
			//ecs_get_path_w_sep_buf(world, 0, e, ".", NULL, str);
		}
		break;
	}
	case EcsId: {
		/*
		ecs_id_t id = *(const ecs_id_t *)base;
		if (!id) {
			ecs_strbuf_appendlit(str, "#0");
		} else {
			ecs_id_str_buf(world, id, str);
		}
		*/
		break;
	}
	default:
		ecs_err("invalid primitive kind");
		return -1;
	}

	return 0;
}

static int flecs_expr_ser_type_op(const ecs_world_t *world, ecs_meta_type_op_t *op, const void *ptr, char const *label, bool is_expr)
{
	switch (op->kind) {
	case EcsOpPush:
	case EcsOpPop:
		/* Should not be parsed as single op */
		ecs_throw(ECS_INVALID_PARAMETER, NULL);
		break;
	case EcsOpEnum:
		if (flecs_expr_ser_enum(world, op, ECS_OFFSET(ptr, op->offset), label)) {
			goto error;
		}
		break;
	case EcsOpBitmask:
		if (flecs_expr_ser_bitmask(world, op, ECS_OFFSET(ptr, op->offset), label)) {
			goto error;
		}
		break;
	case EcsOpArray:
		if (expr_ser_array(world, op, ECS_OFFSET(ptr, op->offset), label)) {
			goto error;
		}
		break;
	case EcsOpVector:
		if (expr_ser_vector(world, op, ECS_OFFSET(ptr, op->offset), label)) {
			goto error;
		}
		break;
	case EcsOpScope:
	case EcsOpPrimitive:
	case EcsOpBool:
	case EcsOpChar:
	case EcsOpByte:
	case EcsOpU8:
	case EcsOpU16:
	case EcsOpU32:
	case EcsOpU64:
	case EcsOpI8:
	case EcsOpI16:
	case EcsOpI32:
	case EcsOpI64:
	case EcsOpF32:
	case EcsOpF64:
	case EcsOpUPtr:
	case EcsOpIPtr:
	case EcsOpEntity:
	case EcsOpId:
	case EcsOpString:
	case EcsOpOpaque:
		if (flecs_expr_ser_primitive(world, flecs_expr_op_to_primitive_kind(op->kind),
		    ECS_OFFSET(ptr, op->offset), op->name, is_expr)) {
			/* Unknown operation */
			ecs_err("unknown serializer operation kind (%d)", op->kind);
			goto error;
		}
		break;
	default:
		ecs_throw(ECS_INVALID_PARAMETER, "invalid operation");
	}

	return 0;
error:
	return -1;
}

static int flecs_expr_ser_type_ops(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, char const *label, int32_t in_array, bool is_expr)
{
	for (int i = 0; i < op_count; i++) {
		ecs_meta_type_op_t *op = &ops[i];

		if (in_array <= 0) {
			if (op->name) {
				//ecs_strbuf_list_next(str);
				//ecs_strbuf_append(str, "%s: ", op->name);
			}

			int32_t elem_count = op->count;
			if (elem_count > 1) {
				/* Serialize inline array */
				if (expr_ser_elements(world, op, op->op_count, base,
				    elem_count, op->size, label, true)) {
					return -1;
				}

				i += op->op_count - 1;
				continue;
			}
		}

		switch (op->kind) {
		case EcsOpPush:
			//ecs_strbuf_list_push(str, "{", ", ");
			in_array--;
			break;
		case EcsOpPop:
			//ecs_strbuf_list_pop(str, "}");
			in_array++;
			break;
		case EcsOpArray:
		case EcsOpVector:
		case EcsOpEnum:
		case EcsOpBitmask:
		case EcsOpScope:
		case EcsOpPrimitive:
		case EcsOpBool:
		case EcsOpChar:
		case EcsOpByte:
		case EcsOpU8:
		case EcsOpU16:
		case EcsOpU32:
		case EcsOpU64:
		case EcsOpI8:
		case EcsOpI16:
		case EcsOpI32:
		case EcsOpI64:
		case EcsOpF32:
		case EcsOpF64:
		case EcsOpUPtr:
		case EcsOpIPtr:
		case EcsOpEntity:
		case EcsOpId:
		case EcsOpString:
		case EcsOpOpaque:
			if (flecs_expr_ser_type_op(world, op, base, op->name, is_expr)) {
				goto error;
			}
			break;
		default:
			ecs_throw(ECS_INVALID_PARAMETER, "invalid operation");
		}
	}

	return 0;
error:
	return -1;
}

static int expr_ser_elements(const ecs_world_t *world, ecs_meta_type_op_t *ops, int32_t op_count, const void *base, int32_t elem_count, int32_t elem_size, char const *label, bool is_array)
{
	//ecs_strbuf_list_push(str, "[", ", ");

	const void *ptr = base;

	int i;
	for (i = 0; i < elem_count; i++) {
	//	ecs_strbuf_list_next(str);
		if (flecs_expr_ser_type_ops(
		    world, ops, op_count, ptr, label, is_array, true)) {
			return -1;
		}
		ptr = ECS_OFFSET(ptr, elem_size);
	}

	//ecs_strbuf_list_pop(str, "]");

	return 0;
}

static int flecs_expr_ser_type(const ecs_world_t *world, const ecs_vec_t *v_ops, const void *base, char const *label, bool is_expr)
{
	ecs_meta_type_op_t *ops = ecs_vec_first_t(v_ops, ecs_meta_type_op_t);
	int32_t count = ecs_vec_count(v_ops);
	return flecs_expr_ser_type_ops(world, ops, count, base, label, 0, is_expr);
}

// ecs_ptr_to_expr_buf()
int igser_draw(const ecs_world_t *world, ecs_entity_t type, const void *ptr)
{
	const EcsTypeSerializer *ser = ecs_get(
	world, type, EcsTypeSerializer);
	if (ser == NULL) {
		char *path = ecs_get_path(world, type);
		ecs_err("cannot serialize value for type '%s'", path);
		ecs_os_free(path);
		goto error;
	}

	if (flecs_expr_ser_type(world, &ser->ops, ptr, NULL, true)) {
		goto error;
	}

	return 0;
error:
	return -1;
}


