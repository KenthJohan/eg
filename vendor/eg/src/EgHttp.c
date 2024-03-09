#include "eg/EgHttp.h"
#include "eg/EgFs.h"
#include "eg/EgStr.h"



ECS_COMPONENT_DECLARE(EgHttp);



static bool OnRequest(const ecs_http_request_t* request, ecs_http_reply_t *reply, void *ctx)
{
	printf("OnRequest %s\n", request->path);
    return true;
}






static void Dequeue(ecs_iter_t *it) {
    EgHttp *h = ecs_field(it, EgHttp, 1);

	/*
    if (it->delta_system_time > (ecs_ftime_t)1.0) {
        ecs_warn("detected large progress interval (%.2fs), REST request may timeout",(double)it->delta_system_time);
    }
	*/

    for(int i = 0; i < it->count; i ++) {
        ecs_http_server_t *srv = h[i].srv;
        if (srv) {
            ecs_http_server_dequeue(srv, it->delta_time);
        }
    } 
}


static ECS_COPY(EgHttp, dst, src, {

})

static ECS_MOVE(EgHttp, dst, src, {

})

static ECS_DTOR(EgHttp, ptr, { 

})

static void EgHttp_on_set(ecs_iter_t *it)
{
    EgHttp *h = it->ptrs[0];

    for(int i = 0; i < it->count; i ++) {
		h->srv = ecs_http_server_init(&(ecs_http_server_desc_t){
			.port = 27756,
			.callback = OnRequest,
			.ctx = NULL
		});
        ecs_http_server_start(h->srv);
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