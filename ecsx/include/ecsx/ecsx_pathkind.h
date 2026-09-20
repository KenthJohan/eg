#pragma once

typedef enum EcsxPathKind {
	ECSX_PATHKIND_NONE,
	ECSX_PATHKIND_DIR,
	ECSX_PATHKIND_FILE,
	ECSX_PATHKIND_LINK,
	ECSX_PATHKIND_OTHER,
	ECSX_PATHKIND_UDP,
	ECSX_PATHKIND_TCP,
	ECSX_PATHKIND_HTTP
} EcsxPathKind;

EcsxPathKind ecsx_pathkind_get_path_type(const char *path);
