#include "eg_fs.h"
#include <stdio.h>
#include <flecs.h>


int eg_fs_readfile(char ** out_result, char const * path)
{
    FILE * file;
    char* content = NULL;
    size_t size;
    int32_t bytes;

    ecs_os_fopen(&file, path, "r");
    fseek(file, 0 , SEEK_END);
    bytes = (int32_t)ftell(file);
    if (bytes == -1)
	{
        goto error;
    }
    rewind(file);

    /* Load contents in memory */
    content = ecs_os_malloc(bytes + 1);
    size = (size_t)bytes;
    if (!(size = fread(content, 1, size, file)) && bytes)
    {
        ecs_err("%s: read zero bytes instead of %d", path, size);
        ecs_os_free(content);
        content = NULL;
        goto error;
    }
    else
    {
        content[size] = '\0';
    }

    (*out_result) = content;
    return 0;

error:
    ecs_os_free(content);
    return -1;
}