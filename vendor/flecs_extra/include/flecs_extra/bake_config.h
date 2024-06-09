/*
                                   )
                                  (.)
                                  .|.
                                  | |
                              _.--| |--._
                           .-';  ;`-'& ; `&.
                          \   &  ;    &   &_/
                           |"""---...---"""|
                           \ | | | | | | | /
                            `---.|.|.|.---'

 * This file is generated by bake.lang.c for your convenience. Headers of
 * dependencies will automatically show up in this file. Include bake_config.h
 * in your main project file. Do not edit! */

#ifndef FLECS_EXTRA_BAKE_CONFIG_H
#define FLECS_EXTRA_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <flecs.h>

/* Convenience macro for exporting symbols */
#ifndef flecs_extra_STATIC
#if defined(flecs_extra_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define FLECS_EXTRA_API __declspec(dllexport)
#elif defined(flecs_extra_EXPORTS)
  #define FLECS_EXTRA_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define FLECS_EXTRA_API __declspec(dllimport)
#else
  #define FLECS_EXTRA_API
#endif
#else
  #define FLECS_EXTRA_API
#endif

#endif

