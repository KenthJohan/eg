#include "eglws.h"
#include "ws.h"

ECS_COMPONENT_DECLARE(EglwsContext);

static ECS_COPY(EglwsContext, dst, src, {
	ews_t *impl = src->impl;
	if (impl) {
		impl->rc++;
	}

	ecs_os_strset(&dst->ipaddr, src->ipaddr);
	dst->port = src->port;
	dst->impl = impl;
})

static ECS_MOVE(EglwsContext, dst, src, {
	*dst = *src;
	src->ipaddr = NULL;
	src->impl = NULL;
})

static ECS_DTOR(EglwsContext, ptr, {
	ews_t *impl = ptr->impl;
	if (impl) {
		impl->rc--;
		if (!impl->rc) {
			ews_fini(impl);
			ecs_os_free(impl);
		}
	}
	ecs_os_free(ptr->ipaddr);
})

static void EglwsContext_on_set(ecs_iter_t *it)
{
	EglwsContext *ctx = it->ptrs[0];
	for (int i = 0; i < it->count; i++) {
		if (!ctx[i].port) {
			ctx[i].port = ECS_REST_DEFAULT_PORT;
		}
		ecs_dbg_2("ews_init");
		ews_t *srv = ews_init();
		if (!srv) {
			const char *ipaddr = ctx[i].ipaddr ? ctx[i].ipaddr : "0.0.0.0";
			ecs_err("failed to create REST server on %s:%u", ipaddr, ctx[i].port);
			continue;
		}
		ctx[i].impl = srv;
	}
}


static void SendBananas(ecs_iter_t *it)
{
	static int banana = 1;
    EglwsContext *ctx = ecs_field(it, EglwsContext, 1);
    for(int32_t i = 0; i < it->count; ++i, ++ctx) {
		if (ctx->impl) {
			ews_progress(ctx->impl);
			//printf("ews_send_binary\n");
			//char buf[128];
			//snprintf(buf, sizeof(buf), "Banana %i", banana++);
			// ews_send_string(ews, buf);
			//ews_send_binary(ctx->impl, &banana, sizeof(banana));
		} else {
			printf("no vhd\n");
		}
    } 
}




void WebsocketsImport(ecs_world_t *world)
{
	ECS_MODULE(world, Websockets);
	ecs_set_name_prefix(world, "Websockets");

	ECS_COMPONENT_DEFINE(world, EglwsContext);

    ecs_set_hooks(world, EglwsContext, { 
        .ctor = ecs_default_ctor,
        .move = ecs_move(EglwsContext),
        .copy = ecs_copy(EglwsContext),
        .dtor = ecs_dtor(EglwsContext),
        .on_set = EglwsContext_on_set
    });


	ecs_entity_t a = ecs_new_entity(world, "EglwsContext1");
	ecs_set(world, a, EglwsContext, {.port = 1234, "localhost"});

	
	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "SendBananas", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = SendBananas,
	.interval = 1.0,
	.query.filter.terms = {
	{.id = ecs_id(EglwsContext)}}});
	
}