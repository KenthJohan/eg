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

#ifndef EG_CAN_BAKE_CONFIG_H
#define EG_CAN_BAKE_CONFIG_H

/* Headers of public dependencies */
/* No dependencies */

/* Convenience macro for exporting symbols */
#ifndef eg_can_STATIC
#if defined(eg_can_EXPORTS) && (defined(_MSC_VER) || defined(__MINGW32__))
  #define EG_CAN_API __declspec(dllexport)
#elif defined(eg_can_EXPORTS)
  #define EG_CAN_API __attribute__((__visibility__("default")))
#elif defined(_MSC_VER)
  #define EG_CAN_API __declspec(dllimport)
#else
  #define EG_CAN_API
#endif
#else
  #define EG_CAN_API
#endif

#endif

