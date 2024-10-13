#include "app_gui_generic.h"

#include <stdlib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <egimgui.h>

#include <egpolynomials.h>

#include "CanDbc.h"
#include "ig.h"
#include "igser.h"

/*
https://github.com/SanderMertens/flecs/blob/cf2f11c4b20efd67f12055650d204981fa3a6de3/src/addons/script/serialize.c#L404



*/

void app_gui_generic(ecs_world_t *world, ecs_query_t *q)
{
	for (int j = 0; j < q->field_count; ++j) {
		// iterate_component_members(world, q->ids[j]);
	}
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {

		for (int i = 0; i < it.count; ++i) {
			igPushID_Int(i);
			for (int j = 0; j < it.field_count; ++j) {
				void *p = ecs_field_w_size(&it, it.sizes[j], j);
				void *ptr = ECS_OFFSET(p, it.sizes[j] * i);
				int r = igser_draw(world, it.ids[j], ptr);
			}
			igPopID();
		}

		/*
		EgPolynomialsLinear *f0 = ecs_field(&it, EgPolynomialsLinear, 0);
		for (int i = 0; i < it.count; ++i) {
		    ecs_entity_t e = it.entities[i];
		    generic_gui(&(generic_gui_t){
		    .label = "##Select",
		    .kind = GENERIC_GUI_KIND_TEXT_INT,
		    .text_int.value = (int)e});
		}
		*/
	}
}