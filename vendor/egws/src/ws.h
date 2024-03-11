/*
 * Copyright (C) 2016-2022  Davidson Francis <davidsondfgl@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @dir include/
 * @brief wsServer include directory
 *
 * @file ws.h
 * @brief wsServer constants and functions.
 */
#ifndef WS_H
#define WS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

/* Opaque client connection type. */
typedef struct ws_connection ws_cli_conn_t;

/**
 * @brief events Web Socket events types.
 */
struct ws_events {
	/**
	 * @brief On open event, called when a new client connects.
	 */
	void (*onopen)(ws_cli_conn_t *client);
	/**
	 * @brief On close event, called when a client disconnects.
	 */
	void (*onclose)(ws_cli_conn_t *client);
	/**
	 * @brief On message event, called when a client sends a text
	 * or binary message.
	 */
	void (*onmessage)(ws_cli_conn_t *client,
	const unsigned char *msg, uint64_t msg_size, int type);
};

/**
 * @brief server Web Socket server parameters
 */
struct ws_server {
	/**
	 * @brief Required hostname that the wsServer will bind to
	 */
	const char *host;
	/**
	 * @brief Listening port
	 */
	uint16_t port;
	/**
	 * @brief Whether if the ws_socket() should create a new thread
	 * and be non-blocking (1) or not (0).
	 */
	int thread_loop;
	/**
	 * @brief Ping timeout in milliseconds
	 */
	uint32_t timeout_ms;
	/**
	 * @brief Server events.
	 */
	struct ws_events evs;
};

/* Forward declarations. */

/* Internal usage. */
extern int get_handshake_accept(char *wsKey, unsigned char **dest);
extern int get_handshake_response(char *hsrequest, char **hsresponse);

/* External usage. */
extern char *ws_getaddress(ws_cli_conn_t *client);
extern char *ws_getport(ws_cli_conn_t *client);
extern int ws_sendframe(
ws_cli_conn_t *cli, const char *msg, uint64_t size, int type);
extern int ws_sendframe_bcast(
uint16_t port, const char *msg, uint64_t size, int type);
extern int ws_sendframe_txt(ws_cli_conn_t *cli, const char *msg);
extern int ws_sendframe_txt_bcast(uint16_t port, const char *msg);
extern int ws_sendframe_bin(ws_cli_conn_t *cli, const char *msg,
uint64_t size);
extern int ws_sendframe_bin_bcast(uint16_t port, const char *msg,
uint64_t size);
extern int ws_get_state(ws_cli_conn_t *cli);
extern int ws_close_client(ws_cli_conn_t *cli);
extern int ws_socket(struct ws_server *ws_srv);

/* Ping routines. */
extern void ws_ping(ws_cli_conn_t *cli, int threshold);

#ifdef AFL_FUZZ
extern int ws_file(struct ws_events *evs, const char *file);
#endif

#ifdef __cplusplus
}
#endif

#endif /* WS_H */
