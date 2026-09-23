#include <EgGpusSdl.h>
#include <SDL3/SDL_gpu.h>
#include <string.h>

bool EgGpusSdlUploadBuffer(const EgGpusDevice *device, const EgGpusBuffer *buffer, const void *data, uint32_t size)
{
	if (!device || !device->object || !buffer || !buffer->object || !data || size == 0) {
		return false;
	}

	SDL_GPUTransferBufferCreateInfo transfer_info = {
	.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
	.size = size,
	};
	SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(device->object, &transfer_info);
	if (!transfer) {
		return false;
	}

	void *mapped = SDL_MapGPUTransferBuffer(device->object, transfer, false);
	if (!mapped) {
		SDL_ReleaseGPUTransferBuffer(device->object, transfer);
		return false;
	}
	memcpy(mapped, data, size);
	SDL_UnmapGPUTransferBuffer(device->object, transfer);

	SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(device->object);
	if (!command_buffer) {
		SDL_ReleaseGPUTransferBuffer(device->object, transfer);
		return false;
	}
	SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_GPUTransferBufferLocation source = {.transfer_buffer = transfer, .offset = 0};
	SDL_GPUBufferRegion destination = {.buffer = buffer->object, .offset = 0, .size = size};
	SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);
	SDL_EndGPUCopyPass(copy_pass);
	bool submitted = SDL_SubmitGPUCommandBuffer(command_buffer);
	SDL_ReleaseGPUTransferBuffer(device->object, transfer);
	return submitted;
}

bool EgGpusSdlUploadTexture2D(const EgGpusDevice *device, const EgGpusTexture *texture, const void *data, uint32_t size, uint32_t width, uint32_t height)
{
	if (!device || !device->object || !texture || !texture->object || !data || size == 0 || width == 0 || height == 0) {
		return false;
	}

	SDL_GPUTransferBufferCreateInfo transfer_info = {
	.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
	.size = size,
	};
	SDL_GPUTransferBuffer *transfer = SDL_CreateGPUTransferBuffer(device->object, &transfer_info);
	if (!transfer) {
		return false;
	}

	void *mapped = SDL_MapGPUTransferBuffer(device->object, transfer, false);
	if (!mapped) {
		SDL_ReleaseGPUTransferBuffer(device->object, transfer);
		return false;
	}
	memcpy(mapped, data, size);
	SDL_UnmapGPUTransferBuffer(device->object, transfer);

	SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(device->object);
	if (!command_buffer) {
		SDL_ReleaseGPUTransferBuffer(device->object, transfer);
		return false;
	}
	SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_GPUTextureTransferInfo source = {.transfer_buffer = transfer, .offset = 0};
	SDL_GPUTextureRegion destination = {.texture = texture->object, .w = width, .h = height, .d = 1};
	SDL_UploadToGPUTexture(copy_pass, &source, &destination, false);
	SDL_EndGPUCopyPass(copy_pass);
	bool submitted = SDL_SubmitGPUCommandBuffer(command_buffer);
	SDL_ReleaseGPUTransferBuffer(device->object, transfer);
	return submitted;
}
