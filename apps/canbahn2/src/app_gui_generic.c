#include "app_gui_generic.h"

#include <egpolynomials.h>
#include "CanDbc.h"
#include "ig.h"

void app_gui_generic(ecs_world_t *world, ecs_query_t *q)
{
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		EgPolynomialsLinear *f0 = ecs_field(&it, EgPolynomialsLinear, 0);
		for (int i = 0; i < it.count; ++i) {
			ecs_entity_t e = it.entities[i];
			generic_gui(&(generic_gui_t){
			.label = "##Select",
			.kind = GENERIC_GUI_KIND_TEXT_INT,
			.text_int.value = (int)e});
		}
	}
}