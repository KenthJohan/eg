#include "EgDisplaysSdl.h"
#include <EgDisplays.h>
#include <EgShapes.h>

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_version.h>

static void private_list_modes(ecs_world_t *world, ecs_entity_t parent, SDL_DisplayID displayID)
{
	SDL_DisplayMode **modes;
	/* Print available fullscreen video modes */
	int                    m;
	const SDL_DisplayMode *mode;
	modes = SDL_GetFullscreenDisplayModes(displayID, &m);
	if (m == 0) {
		ecs_err("No available fullscreen video modes");
		return;
	}

	for (int j = 0; j < m; ++j) {
		mode = modes[j];
		int    bpp;
		Uint32 Rmask, Gmask, Bmask, Amask;
		SDL_GetMasksForPixelFormat(mode->format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);

		char buf[256];
		snprintf(buf, sizeof(buf), "Mode %i", j);
		ecs_entity_t e = ecs_entity_init(world, &(ecs_entity_desc_t){.name = buf, .parent = parent});
		ecs_set(world, e, EgDisplaysMode, {.bpp = bpp, .refresh_rate = mode->refresh_rate, .pixel_density = mode->pixel_density});
		ecs_set(world, e, EgShapesRectangle, {.w = mode->w, .h = mode->h});

		snprintf(buf, sizeof(buf), "%i: %dx%d@%gx %gHz, %d bits-per-pixel (%s)", j, mode->w, mode->h, mode->pixel_density, mode->refresh_rate, bpp, SDL_GetPixelFormatName(mode->format));
		ecs_doc_set_name(world, e, buf);
		// SDL_Log("Mode %d: %dx%d@%gx %gHz, %d bits-per-pixel (%s)\n", j, mode->w, mode->h, mode->pixel_density, mode->refresh_rate, bpp, SDL_GetPixelFormatName(mode->format));
	}

	SDL_free(modes);
}

static void private_list_displays(ecs_world_t *world, ecs_entity_t parent)
{
	SDL_DisplayID *displays;
	SDL_Rect       bounds, usablebounds;
	int            bpp;
	Uint32         Rmask, Gmask, Bmask, Amask;
#ifdef SDL_VIDEO_DRIVER_WINDOWS
	int adapterIndex = 0;
	int outputIndex  = 0;
#endif
	int n;
	displays = SDL_GetDisplays(&n);
	ecs_trace("Number of displays: %d", n);
	for (int i = 0; i < n; ++i) {
		SDL_DisplayID displayID = displays[i];
		ecs_trace("Display %" SDL_PRIu32 ": %s", displayID, SDL_GetDisplayName(displayID));

		char buf[64];
		snprintf(buf, sizeof(buf), "Display %i", displayID);
		ecs_entity_t e = ecs_entity_init(world, &(ecs_entity_desc_t){.name = buf, .parent = parent});
		ecs_set(world, e, EgDisplaysDevice, {.id = displayID});
		ecs_doc_set_name(world, e, SDL_GetDisplayName(displayID));

		SDL_zero(bounds);
		SDL_GetDisplayBounds(displayID, &bounds);

		SDL_zero(usablebounds);
		SDL_GetDisplayUsableBounds(displayID, &usablebounds);

		ecs_trace("Bounds: %dx%d at %d,%d", bounds.w, bounds.h, bounds.x, bounds.y);
		ecs_trace("Usable bounds: %dx%d at %d,%d", usablebounds.w, usablebounds.h, usablebounds.x, usablebounds.y);

		const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(displayID);
		SDL_GetMasksForPixelFormat(mode->format, &bpp, &Rmask, &Gmask,
		&Bmask, &Amask);
		ecs_trace("  Desktop mode: %dx%d@%gx %gHz, %d bits-per-pixel (%s)",
		mode->w, mode->h, mode->pixel_density, mode->refresh_rate, bpp,
		SDL_GetPixelFormatName(mode->format));
		if (Rmask || Gmask || Bmask) {
			ecs_trace("      Red Mask   = 0x%.8" SDL_PRIx32 "", Rmask);
			ecs_trace("      Green Mask = 0x%.8" SDL_PRIx32 "", Gmask);
			ecs_trace("      Blue Mask  = 0x%.8" SDL_PRIx32 "", Bmask);
			if (Amask) {
				ecs_trace("      Alpha Mask = 0x%.8" SDL_PRIx32 "", Amask);
			}
		}

		private_list_modes(world, e, displayID);

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && !defined(SDL_PLATFORM_XBOXONE) && !defined(SDL_PLATFORM_XBOXSERIES)
		/* Print the D3D9 adapter index */
		adapterIndex = SDL_GetDirect3D9AdapterIndex(displayID);
		SDL_Log("D3D9 Adapter Index: %d", adapterIndex);

		/* Print the DXGI adapter and output indices */
		SDL_GetDXGIOutputInfo(displayID, &adapterIndex, &outputIndex);
		SDL_Log("DXGI Adapter Index: %d  Output Index: %d", adapterIndex, outputIndex);
#endif
	}
	SDL_free(displays);
}

static void System_EgDisplaysUpdate(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; ++i) {
		private_list_displays(it->world, it->entities[i]);
		ecs_remove(it->world, it->entities[i], EgDisplaysUpdate);
	}
}

void EgDisplaysSdlImport(ecs_world_t *world)
{
	ECS_IMPORT(world, EgDisplays);
	ECS_IMPORT(world, EgShapes);

	ECS_MODULE(world, EgDisplaysSdl);
	ecs_set_name_prefix(world, "EgDisplaysSdl");

	const int compiled = SDL_VERSION;
	const int linked   = SDL_GetVersion();
	ecs_log(0, "EgDisplaysSdl imported (compiled SDL version: %d.%d.%d, linked SDL version: %d.%d.%d)",
	SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled),
	SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));


	SDL_InitSubSystem(SDL_INIT_VIDEO);

	ecs_system(world,
	{.entity     = ecs_entity(world, {.name = "System_EgDisplaysUpdate"}),
	.phase       = EcsOnUpdate,
	.callback    = System_EgDisplaysUpdate,
	.query.terms = {
	{.id = ecs_id(EgDisplaysUpdate), .src.id = EcsSelf},
	}});
}
