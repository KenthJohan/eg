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

#ifndef EGWEBSOCKETS_BAKE_CONFIG_H
#define EGWEBSOCKETS_BAKE_CONFIG_H

/* Headers of public dependencies */
/* No dependencies */

/* Convenience macro for exporting symbols */
#ifndef egwebsockets_STATIC
#if defined(egwebsockets_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define EGWEBSOCKETS_API __declspec(dllexport)
#elif defined(egwebsockets_EXPORTS)
  #define EGWEBSOCKETS_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define EGWEBSOCKETS_API __declspec(dllimport)
#else
  #define EGWEBSOCKETS_API
#endif
#else
  #define EGWEBSOCKETS_API
#endif

#endif

