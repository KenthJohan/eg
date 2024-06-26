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

#ifndef EGSOKOL_BAKE_CONFIG_H
#define EGSOKOL_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <egcomponents.h>
#include <sokol_gfx.h>
#include <sokol_debugtext.h>
#include <sokol_app.h>
#include <sokol_shape.h>

/* Convenience macro for exporting symbols */
#ifndef egsokol_STATIC
#if defined(egsokol_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define EGSOKOL_API __declspec(dllexport)
#elif defined(egsokol_EXPORTS)
  #define EGSOKOL_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define EGSOKOL_API __declspec(dllimport)
#else
  #define EGSOKOL_API
#endif
#else
  #define EGSOKOL_API
#endif

#endif

