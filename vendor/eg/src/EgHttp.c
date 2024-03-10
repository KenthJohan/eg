#include "eg/EgHttp.h"
#include "eg/EgFs.h"
#include "eg/EgStr.h"
#include "eg/eg_fs.h"
#include "eg/eg_http.h"
#include <strings.h>

ECS_COMPONENT_DECLARE(EgWebServer);



static bool OnRequest(const ecs_http_request_t *request, ecs_http_reply_t *reply, void *ctx)
{
	printf("OnRequest %s\n", request->path);
	eg_webserver_t *web = ctx;
	ecs_entity_t root = web->root;
	ecs_world_t *world = web->world;

	char path[256] = {0};
	ecs_os_strncpy(path, request->path, sizeof(path));

	char const * ext = strrchr(path, '.');
	if (ext == NULL) {
		if (path[0]) {
			int len = ecs_os_strlen(path);
			if(path[len - 1] != '/') {
				reply->code = 301;
				reply->status = "Moved Permanently";
				ecs_strbuf_appendstr(&reply->headers, "Location: http://");
				char const * host = request->headers[0].value;
				ecs_strbuf_appendstr(&reply->headers, host);
				ecs_strbuf_appendch(&reply->headers, '/');
				return true;
			}
		}
		ecs_os_strcat(path, "index.html");
		ext = ".html";
	}

	eg_str_replace_ab(path, '.', ',');
	ecs_entity_t b = ecs_lookup_path_w_sep(world, root, path, "/", NULL, true);
	eg_str_replace_ab(path, ',', '.');



	char const * mime = eg_http_get_mime_type(ext+1);

	EgBuffer const *content = NULL;
	if (b && mime) {
		content = ecs_get(world, b, EgBuffer);
	}

	if(content) {
		reply->code = 200;
		reply->content_type = mime;
		//ecs_strbuf_appendstrn(&reply->body, content->data, content->size-1);
		ecs_strbuf_appendstr(&reply->body, content->data);
	} else {
		reply->code = 404;
		reply->content_type = "text/html";
		ecs_strbuf_appendstr(&reply->body, "File not found");
	}

	return true;
}

ecs_http_server_t *eg_server_init(ecs_world_t *world, ecs_entity_t root, const ecs_http_server_desc_t *desc)
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

static void Dequeue(ecs_iter_t *it)
{
	EgWebServer *h = ecs_field(it, EgWebServer, 1);

	/*
	if (it->delta_system_time > (ecs_ftime_t)1.0) {
	    ecs_warn("detected large progress interval (%.2fs), REST request may timeout",(double)it->delta_system_time);
	}
	*/

	for (int i = 0; i < it->count; i++) {
		eg_webserver_t *ctx = h[i].impl;
		if (ctx) {
			ecs_http_server_dequeue(ctx->srv, it->delta_time);
		}
	}
}

static ECS_COPY(EgWebServer, dst, src, {
	eg_webserver_t *impl = src->impl;
	if (impl) {
		impl->rc++;
	}
	dst->impl = impl;
	dst->root = src->root;
})

static ECS_MOVE(EgWebServer, dst, src, {
	*dst = *src;
	src->impl = NULL;
})

static ECS_DTOR(EgWebServer, ptr, {
	eg_webserver_t *impl = ptr->impl;
	if (impl) {
		impl->rc--;
		if (!impl->rc) {
			ecs_http_server_fini(impl->srv);
			ecs_os_free(impl);
		}
	}
})

static void EgWebServer_on_set(ecs_iter_t *it)
{
	EgWebServer *h = it->ptrs[0];

	for (int i = 0; i < it->count; ++i, ++h) {
		ecs_http_server_t *srv = eg_server_init(it->real_world, h->root, &(ecs_http_server_desc_t){.port = 27756});
		ecs_http_server_start(srv);
		h->impl = ecs_http_server_ctx(srv);
	}
}

void EgHttpServersImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgHttpServers);
	ECS_IMPORT(world, EgFs);
	ECS_IMPORT(world, EgStr);

	ECS_COMPONENT_DEFINE(world, EgWebServer);

	ECS_SYSTEM(world, Dequeue, EcsPostFrame, EgWebServer);

	ecs_set_hooks(world, EgWebServer, {.ctor = ecs_default_ctor, .move = ecs_move(EgWebServer), .copy = ecs_copy(EgWebServer), .dtor = ecs_dtor(EgWebServer), .on_set = EgWebServer_on_set});
}




ecs_entity_t EgHttp_add_file(ecs_world_t * world, ecs_entity_t rootnode, char const * filename)
{
	char name_flecs[256];
	ecs_os_strncpy(name_flecs, filename, sizeof(name_flecs));
	eg_str_replace_ab(name_flecs, '.', ',');
	eg_str_replace_ab(name_flecs, '/', '.');
	ecs_entity_t f = ecs_new_entity(world, name_flecs);
	ecs_add_pair(world, f, EcsIsA, rootnode);
	ecs_add(world, f, EgFsLoad); // Load file once
	return f;
}

/*
	{
		ecs_entity_t root = ecs_new_entity(world, "webroot");
		ecs_set_pair(world, root, EgText, EgFsRoot, {"./website"});
		ecs_entity_t f1 = ecs_new_entity(world, "webroot.index,html");
		ecs_entity_t f2 = ecs_new_entity(world, "webroot.example,js");
		ecs_entity_t f3 = ecs_new_entity(world, "webroot.favicon,ico");
		ecs_entity_t f4 = ecs_new_entity(world, "webroot.sub1.index,html");
		ecs_entity_t f5 = ecs_new_entity(world, "webroot.sub1.script,js");
		ecs_entity_t f6 = ecs_new_entity(world, "webroot.webdesign,css");
		ecs_add(world, f1, EgFsLoad); // Load file once
		ecs_add(world, f2, EgFsLoad); // Load file once
		ecs_add(world, f3, EgFsLoad); // Load file once
		ecs_add(world, f4, EgFsLoad); // Load file once
		ecs_add(world, f5, EgFsLoad); // Load file once
		ecs_add(world, f6, EgFsLoad); // Load file once
		ecs_add_pair(world, f1, EcsIsA, root);
		ecs_add_pair(world, f2, EcsIsA, root);
		ecs_add_pair(world, f3, EcsIsA, root);
		ecs_add_pair(world, f4, EcsIsA, root);
		ecs_add_pair(world, f5, EcsIsA, root);
		ecs_add_pair(world, f6, EcsIsA, root);
		ecs_entity_t e = ecs_new_entity(world, "MyHttpServer");
		ecs_set(world, e, EgWebServer, {.root = root});
	}
	*/