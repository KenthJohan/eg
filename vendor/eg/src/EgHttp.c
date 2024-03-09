#include "eg/EgHttp.h"
#include "eg/EgFs.h"
#include "eg/EgStr.h"



ECS_COMPONENT_DECLARE(EgHttp);


static void replace_ab(char * str, char a, char b)
{
	char * p = str;
	while(p[0]) {
		if (p[0] == a) {
			p[0] = b;
		}
		p++;
	}
}


static bool OnRequest(const ecs_http_request_t* request, ecs_http_reply_t *reply, void *ctx)
{
	printf("OnRequest %s\n", request->path);
	eg_webserver_t * web = ctx;
	ecs_entity_t root = web->root;
	ecs_world_t * world = web->world;

	replace_ab(request->path, '.', ',');
	ecs_entity_t b = ecs_lookup_path_w_sep(world, root, request->path, "/", NULL, true);
	replace_ab(request->path, ',', '.');

	if (b) {
		EgBuffer const * content = ecs_get(world, b, EgBuffer);
		reply->code = 200;
		reply->content_type = "text/html";
		ecs_strbuf_appendstrn(&reply->body, content->data, content->size);
	}
	else
	{
		reply->code = 404;
		reply->content_type = "text/html";
		ecs_strbuf_appendstr(&reply->body, "File not found");
	}



    return true;
}



ecs_http_server_t* eg_server_init(ecs_world_t *world, ecs_entity_t root, const ecs_http_server_desc_t *desc)
{
    eg_webserver_t *ctx = ecs_os_calloc_t(eg_webserver_t);
    ecs_http_server_desc_t desc0 = {0};
    if (desc) {
        desc0 = *desc;
    }
    desc0.callback = OnRequest;
    desc0.ctx = ctx;

    ecs_http_server_t *srv = ecs_http_server_init(&desc0);
    if (!srv) {
        ecs_os_free(ctx);
        return NULL;
    }

    ctx->world = world;
	ctx->root = root;
    ctx->srv = srv;
    ctx->rc = 1;
    return srv;
}


















static void Dequeue(ecs_iter_t *it) {
    EgHttp *h = ecs_field(it, EgHttp, 1);

	/*
    if (it->delta_system_time > (ecs_ftime_t)1.0) {
        ecs_warn("detected large progress interval (%.2fs), REST request may timeout",(double)it->delta_system_time);
    }
	*/

    for(int i = 0; i < it->count; i ++) {
        eg_webserver_t *ctx = h[i].impl;
        if (ctx) {
            ecs_http_server_dequeue(ctx->srv, it->delta_time);
        }
    } 
}


static ECS_COPY(EgHttp, dst, src, {
    eg_webserver_t *impl = src->impl;
    if (impl) {
        impl->rc ++;
    }
    dst->impl = impl;
    dst->root = src->root;
})

static ECS_MOVE(EgHttp, dst, src, {
    *dst = *src;
    src->impl = NULL;
})

static ECS_DTOR(EgHttp, ptr, { 
    eg_webserver_t *impl = ptr->impl;
    if (impl) {
        impl->rc --;
        if (!impl->rc) {
            ecs_http_server_fini(impl->srv);
            ecs_os_free(impl);
        }
    }
})

static void EgHttp_on_set(ecs_iter_t *it)
{
    EgHttp *h = it->ptrs[0];

    for(int i = 0; i < it->count; ++i, ++h) {
		ecs_http_server_t *srv = eg_server_init(it->real_world, h->root, &(ecs_http_server_desc_t){
			.port = 27756
		});
        ecs_http_server_start(srv);
		h->impl = ecs_http_server_ctx(srv);
    }
}





void EgHttpServersImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgHttpServers);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgStr);


	ECS_COMPONENT_DEFINE(world, EgHttp);

	ECS_SYSTEM(world, Dequeue, EcsPostFrame, EgHttp);


    ecs_set_hooks(world, EgHttp, { 
        .ctor = ecs_default_ctor,
        .move = ecs_move(EgHttp),
        .copy = ecs_copy(EgHttp),
        .dtor = ecs_dtor(EgHttp),
        .on_set = EgHttp_on_set
    });

}