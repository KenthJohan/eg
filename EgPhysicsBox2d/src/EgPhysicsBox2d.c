#include "EgPhysicsBox2d.h"
#include <EgSpatials.h>
#include <EgCameras.h>
#include <EgPhysics.h>
#include <EgShapes.h>
#include <ecsx.h>

ECS_COMPONENT_DECLARE(EgPhysicsBox2dOverlapChecking);
ECS_TAG_DECLARE(EgB2TargetTransform);

ECS_COMPONENT_DECLARE(b2WorldId);
ECS_COMPONENT_DECLARE(b2BodyId);
ECS_COMPONENT_DECLARE(b2ShapeId);

static void b2WorldId_Create(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	EgPhysicsWorldDef *def = ecs_field(it, EgPhysicsWorldDef, 0); // self
	for (int i = 0; i < it->count; ++i, ++def) {
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity    = (b2Vec2){def->gravity_x, def->gravity_y};
		b2WorldId bw        = b2CreateWorld(&worldDef);
		ecs_set_ptr(it->world, it->entities[i], b2WorldId, &bw);
	}
	ecs_log_set_level(-1);
}

static void b2BodyId_Create(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	b2WorldId        *bw  = ecs_field_shared(it, b2WorldId, 0);      //
	Position2        *pos = ecs_field_self(it, Position2, 1);        //
	EgPhysicsBodyDef *def = ecs_field_self(it, EgPhysicsBodyDef, 2); //
	for (int i = 0; i < it->count; ++i, ++def, ++pos) {
		b2BodyDef body_def = b2DefaultBodyDef();
		body_def.type      = def->type;
		body_def.position  = (b2Pos){pos->x, pos->y};
		body_def.userData  = (void *)(uintptr_t)it->entities[i]; // Use the ECS entity as user data
		b2BodyId body      = b2CreateBody(bw[0], &body_def);
		ecs_set_ptr(it->world, it->entities[i], b2BodyId, &body);
	}
	ecs_log_set_level(-1);
}

static void b2ShapeId_Create(ecs_iter_t *it)
{
	ecs_log_set_level(0);
	EgPhysicsShapesDef *def  = ecs_field_self(it, EgPhysicsShapesDef, 0);
	EgShapesRectangle  *r    = ecs_field_self(it, EgShapesRectangle, 1);
	b2BodyId           *body = ecs_field_self(it, b2BodyId, 2);
	for (int i = 0; i < it->count; ++i, ++def, ++r, ++body) {
		b2Polygon  poly      = b2MakeBox(r->w / 2.0f, r->h / 2.0f);
		b2ShapeDef shape_def = b2DefaultShapeDef();
		/*
		if (body_def.type == b2_dynamicBody) {
		    if (box->density <= 0.0f) {
		        ecs_warn("EgPhysicsBox density is %.3f for entity %llu (zero/negative values are allowed but can disable gravity response on dynamic bodies)", box->density, (unsigned long long)it->entities[i]);
		    }
		    if (box->friction <= 0.0f) {
		        ecs_warn("EgPhysicsBox friction is %.3f for entity %llu (zero/negative values are allowed)", box->friction, (unsigned long long)it->entities[i]);
		    }
		}
		*/
		shape_def.density           = def->density;
		shape_def.material.friction = def->friction;
		b2ShapeId shape             = b2CreatePolygonShape(body[0], &shape_def, &poly);
		ecs_set_ptr(it->world, it->entities[i], b2ShapeId, &shape);
	}
	ecs_log_set_level(-1);
}

void b2WorldId_Destroy(ecs_iter_t *it)
{
	b2WorldId *w = ecs_field(it, b2WorldId, 0);
	for (int i = 0; i < it->count; ++i, ++w) {
		b2DestroyWorld(w[0]);
	}
}

static void b2WorldId_Step(ecs_iter_t *it)
{
	float      timeStep     = 1.0f / 60.0f;
	int        subStepCount = 4;
	b2WorldId *w            = ecs_field(it, b2WorldId, 0);
	for (int i = 0; i < it->count; ++i, ++w) {
		b2World_Step(w[0], timeStep, subStepCount);
	}
}

static void EgB2Body_TargetTransform(ecs_iter_t *it)
{
	float timeStep = 1.0f / 60.0f;
	ecs_log_set_level(0);
	b2BodyId  *body = ecs_field(it, b2BodyId, 0);  // self
	Position2 *p    = ecs_field(it, Position2, 1); // shared, up
	for (int i = 0; i < it->count; ++i, ++body) {
		b2Vec2 targetPosition = {p->x, p->y};
		// printf("Setting target transform for body %llu to position (%.3f, %.3f)\n", (unsigned long long)it->entities[i], targetPosition.x, targetPosition.y);
		b2Body_SetTargetTransform(body[0], (b2WorldTransform){targetPosition, b2Rot_identity}, timeStep, true);
	}
	ecs_log_set_level(-1);
}

typedef struct
{
	b2Pos    point;
	b2BodyId bodyId;
} QueryContext;

bool QueryCallback(b2ShapeId shapeId, void *context)
{
	QueryContext *queryContext = (QueryContext *)context;

	b2BodyId   bodyId   = b2Shape_GetBody(shapeId);
	b2BodyType bodyType = b2Body_GetType(bodyId);
	if (bodyType != b2_dynamicBody) {
		// continue query
		return true;
	}

	bool overlap = b2Shape_TestPoint(shapeId, queryContext->point);
	if (overlap) {
		// found shape
		queryContext->bodyId = bodyId;
		return false;
	}

	return true;
}

static void System_Overlap_Checking_Clear(ecs_iter_t *it)
{
	b2BodyId                      *b = ecs_field_self(it, b2BodyId, 0);
	EgPhysicsBox2dOverlapChecking *c = ecs_field_shared(it, EgPhysicsBox2dOverlapChecking, 1);
	b2WorldId                     *w = ecs_field_shared(it, b2WorldId, 2);

	(void)b;
	(void)w;
	for (int i = 0; i < it->count; ++i) {
		if (!ecs_is_valid(it->world, c->tag)) {
			ecs_warn("tag entity %jX is not valid", c->tag);
			continue; // Skip if the tag entity is not valid
		}
		ecs_remove_id(it->world, it->entities[i], c->tag);
	}
}

static void System_Overlap_Checking_Update(ecs_iter_t *it)
{
	b2WorldId                     *w = ecs_field_self(it, b2WorldId, 0);
	EgPhysicsBox2dOverlapChecking *c = ecs_field_self(it, EgPhysicsBox2dOverlapChecking, 1);
	Position2                     *p = ecs_field_shared(it, Position2, 2);
	for (int i = 0; i < it->count; ++i, ++w) {
		b2Vec2       d            = {0.001f, 0.001f};
		b2AABB       box          = {b2Neg(d), d};
		QueryContext queryContext = {{p->x, p->y}, b2_nullBodyId};
		b2World_OverlapAABB(w[0], queryContext.point, box, b2DefaultQueryFilter(), QueryCallback, &queryContext);
		if (!B2_IS_NON_NULL(queryContext.bodyId)) {
			continue; // No overlap found, continue to next entity
		}
		// printf("Overlap found at position (%.3f, %.3f) with body ID %d\n", queryContext.point.x, queryContext.point.y, queryContext.bodyId.index1);
		if (!ecs_is_valid(it->world, c->tag)) {
			ecs_warn("tag entity %jX is not valid", c->tag);
			continue; // Skip if the tag entity is not valid
		}

		ecs_entity_t body_entity = (ecs_entity_t)(uintptr_t)b2Body_GetUserData(queryContext.bodyId);
		if (!ecs_is_valid(it->world, body_entity)) {
			ecs_warn("Entity %jX is not valid", body_entity);
			continue; // Skip invalid entities
		}
		// printf("name %s\n", ecs_get_name(it->world, body_entity));
		ecs_add_id(it->world, body_entity, c->tag);
	}
}

static void System_Get_Position(ecs_iter_t *it)
{
	b2BodyId  *b = ecs_field_self(it, b2BodyId, 0);
	Position2 *p = ecs_field_self(it, Position2, 1);
	for (int i = 0; i < it->count; ++i, ++b, ++p) {
		b2Vec2 pos = b2Body_GetPosition(b[0]);
		p->x       = pos.x;
		p->y       = pos.y;
	}
}

void EgPhysicsBox2dImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgPhysicsBox2d);
	ecs_set_name_prefix(world, "EgPhysicsBox2d");

	ECS_IMPORT(world, EgPhysics);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);

	ECS_COMPONENT_DEFINE(world, EgPhysicsBox2dOverlapChecking);
	ECS_TAG_DEFINE(world, EgB2TargetTransform);
	ecs_add_id(world, EgB2TargetTransform, EcsTraversable);
	ecs_add_id(world, ecs_id(EgPhysicsBox2dOverlapChecking), EcsTraversable);

	ECS_COMPONENT_DEFINE(world, b2BodyId);
	ECS_COMPONENT_DEFINE(world, b2WorldId);
	ECS_COMPONENT_DEFINE(world, b2ShapeId);

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(b2WorldId),
	.members = {
	{.name = "index1", .type = ecs_id(ecs_u16_t)},
	{.name = "generation", .type = ecs_id(ecs_u16_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(b2BodyId),
	.members = {
	{.name = "index1", .type = ecs_id(ecs_i32_t)},
	{.name = "world0", .type = ecs_id(ecs_u16_t)},
	{.name = "generation", .type = ecs_id(ecs_u16_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(b2ShapeId),
	.members = {
	{.name = "index1", .type = ecs_id(ecs_i32_t)},
	{.name = "world0", .type = ecs_id(ecs_u16_t)},
	{.name = "generation", .type = ecs_id(ecs_u16_t)},
	}});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity  = ecs_id(EgPhysicsBox2dOverlapChecking),
	.members = {
	{.name = "tag", .type = ecs_id(ecs_entity_t)},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "b2WorldId_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = b2WorldId_Create,
	.query.terms =
	{
	{.id = ecs_id(EgPhysicsWorldDef), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(b2WorldId), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "b2BodyId_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = b2BodyId_Create,
	.query.terms =
	{
	{.id = ecs_id(b2WorldId), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgPhysicsBodyDef), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(b2BodyId), .oper = EcsNot}, // Adds this
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "b2ShapeId_Create", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = b2ShapeId_Create,
	.query.terms =
	{
	{.id = ecs_id(EgPhysicsShapesDef), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(EgShapesRectangle), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(b2BodyId), .src.id = EcsSelf, .inout = EcsIn}, // Attaches to this
	{.id = ecs_id(b2ShapeId), .oper = EcsNot},                   // Adds this
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "b2WorldId_Step", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = b2WorldId_Step,
	.query.terms =
	{
	{.id = ecs_id(b2WorldId), .src.id = EcsSelf, .inout = EcsIn},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "EgB2Body_TargetTransform", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = EgB2Body_TargetTransform,
	.query.terms =
	{
	{.id = ecs_id(b2BodyId), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Position2), .trav = EgB2TargetTransform, .src.id = EcsUp, .inout = EcsIn},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "System_Overlap_Checking_Clear", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Overlap_Checking_Clear,
	.query.terms =
	{
	{.id = ecs_id(b2BodyId), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_pair(ecs_id(EgPhysicsBox2dOverlapChecking), EcsWildcard), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	{.id = ecs_id(b2WorldId), .trav = EcsChildOf, .src.id = EcsUp, .inout = EcsIn},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "System_Overlap_Checking_Update", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Overlap_Checking_Update,
	.query.terms =
	{
	{.id = ecs_id(b2WorldId), .inout = EcsIn},
	{.id = ecs_pair(ecs_id(EgPhysicsBox2dOverlapChecking), EcsWildcard), .inout = EcsIn},
	{.id = ecs_id(Position2), .trav = ecs_id(EgPhysicsBox2dOverlapChecking), .src.id = EcsUp, .inout = EcsIn},
	}});

	ecs_system(world,
	{.entity  = ecs_entity(world, {.name = "System_Get_Position", .add = ecs_ids(ecs_dependson(EcsOnUpdate))}),
	.callback = System_Get_Position,
	.query.terms =
	{
	{.id = ecs_id(b2BodyId), .src.id = EcsSelf, .inout = EcsIn},
	{.id = ecs_id(Position2), .src.id = EcsSelf, .inout = EcsOut},
	}});

	ecs_observer(world,
	{.query   = {.terms = {{.id = ecs_id(b2WorldId), .src.id = EcsSelf, .inout = EcsIn}}},
	.events   = {EcsOnRemove},
	.callback = b2WorldId_Destroy});
}
