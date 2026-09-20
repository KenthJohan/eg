#include <EgFs_test.h>
#include <ecsx/ecsx_file.h>
#include <ecsx/ecsx_pathkind.h>

static ecs_world_t *world;

void TestEgFs_setup(void)
{
	world = ecs_init();
	ECS_IMPORT(world, EgFs);
}

void TestEgFs_teardown(void)
{
	ecs_fini(world);
}

void TestEgFs_test_path_type_schemes(void)
{
	test_int(ecsx_pathkind_get_path_type("udp://127.0.0.1:5000"), ECSX_PATHKIND_UDP);
	test_int(ecsx_pathkind_get_path_type("tcp://127.0.0.1:5000"), ECSX_PATHKIND_TCP);
	test_int(ecsx_pathkind_get_path_type("http://example.com"), ECSX_PATHKIND_HTTP);
}

void TestEgFs_test_path1_script_function(void)
{
	const char *script =
	"using eg\n"
	"const result: entity = fs.path1(\"./project.json\")";
	ecs_entity_t function = ecs_lookup(world, "eg.fs.path1");
	test_assert(function != 0);

	test_int(ecs_script_run(world, "path1", script, NULL), 0);

	ecs_entity_t file = ecs_lookup_path_w_sep(world, EgFsCwd, "project.json", "/", NULL, false);
	test_assert(file != 0);
	test_assert(ecs_has_id(world, file, EgFsFile));
}
