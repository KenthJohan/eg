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

#ifndef NUKLEAR_BAKE_CONFIG_H
#define NUKLEAR_BAKE_CONFIG_H

/* Headers of public dependencies */
/* No dependencies */

/* Convenience macro for exporting symbols */
#ifndef nuklear_STATIC
#if defined(nuklear_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define NUKLEAR_API __declspec(dllexport)
#elif defined(nuklear_EXPORTS)
  #define NUKLEAR_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define NUKLEAR_API __declspec(dllimport)
#else
  #define NUKLEAR_API
#endif
#else
  #define NUKLEAR_API
#endif

#endif

