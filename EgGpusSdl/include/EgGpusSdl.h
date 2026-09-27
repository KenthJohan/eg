#pragma once
#include <flecs.h>
#include <EgGpus.h>

void EgGpusSdlImport(ecs_world_t *world);
bool EgGpusSdlUploadBuffer(const EgGpusDevice *device, const EgGpusBuffer *buffer, const void *data, uint32_t size);
bool EgGpusSdlUploadTexture2D(const EgGpusDevice *device, const EgGpusTexture *texture, const void *data, uint32_t size, uint32_t width, uint32_t height);
bool EgGpusSdlUploadTextureArrayLayer(const EgGpusDevice *device, const EgGpusTexture *texture, const void *data, uint32_t size, uint32_t layer, uint32_t width, uint32_t height);
