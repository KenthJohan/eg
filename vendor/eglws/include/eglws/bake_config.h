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

#ifndef EGLWS_BAKE_CONFIG_H
#define EGLWS_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <egcomponents.h>

/* Convenience macro for exporting symbols */
#ifndef eglws_STATIC
#if defined(eglws_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define EGLWS_API __declspec(dllexport)
#elif defined(eglws_EXPORTS)
  #define EGLWS_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define EGLWS_API __declspec(dllimport)
#else
  #define EGLWS_API
#endif
#else
  #define EGLWS_API
#endif

#endif

