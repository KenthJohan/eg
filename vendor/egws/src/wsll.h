#pragma once
#include <stdint.h>

/**
 * @name Global configurations
 */
/**@{*/
/**
 * @brief Max clients connected simultaneously.
 */
#ifndef MAX_CLIENTS
#define MAX_CLIENTS    8
#endif

/**
 * @name Key and message configurations.
 */
/**@{*/
/**
 * @brief Message buffer length.
 */
#define MESSAGE_LENGTH 2048
/**
 * @brief Maximum frame/message length.
 */
#define MAX_FRAME_LENGTH (16*1024*1024)
/**
 * @brief WebSocket key length.
 */
#define WS_KEY_LEN     24
/**
 * @brief Magic string length.
 */
#define WS_MS_LEN      36
/**
 * @brief Accept message response length.
 */
#define WS_KEYMS_LEN   (WS_KEY_LEN + WS_MS_LEN)
/**
 * @brief Magic string.
 */
#define MAGIC_STRING   "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
/**@}*/

/**
 * @name Handshake constants.
 */
/**@{*/
/**
 * @brief Alias for 'Sec-WebSocket-Key'.
 */
#define WS_HS_REQ      "Sec-WebSocket-Key"

/**
 * @brief Handshake accept message length.
 */
#define WS_HS_ACCLEN   130

/**
 * @brief Handshake accept message.
 */
#define WS_HS_ACCEPT                       \
	"HTTP/1.1 101 Switching Protocols\r\n" \
	"Upgrade: websocket\r\n"               \
	"Connection: Upgrade\r\n"              \
	"Sec-WebSocket-Accept: "
/**@}*/

/**
 * @name Frame types.
 */
/**@{*/
/**
 * @brief Frame FIN.
 */
#define WS_FIN      128

/**
 * @brief Frame FIN shift
 */
#define WS_FIN_SHIFT  7

/**
 * @brief Continuation frame.
 */
#define WS_FR_OP_CONT 0

/**
 * @brief Text frame.
 */
#define WS_FR_OP_TXT  1

/**
 * @brief Binary frame.
 */
#define WS_FR_OP_BIN  2

/**
 * @brief Close frame.
 */
#define WS_FR_OP_CLSE 8

/**
 * @brief Ping frame.
 */
#define WS_FR_OP_PING 0x9

/**
 * @brief Pong frame.
 */
#define WS_FR_OP_PONG 0xA

/**
 * @brief Unsupported frame.
 */
#define WS_FR_OP_UNSUPPORTED 0xF
/**@}*/

/**
 * @name Close codes
 */
/**@{*/
/**
 * @brief Normal close
 */
#define WS_CLSE_NORMAL  1000
/**
 * @brief Protocol error
 */
#define WS_CLSE_PROTERR 1002
/**@}*/
/**
 * @brief Inconsistent message (invalid utf-8)
 */
#define WS_CLSE_INVUTF8 1007

/**
 * @name Connection states
 */
/**@{*/
/**
 * @brief Connection not established yet.
 */
#define WS_STATE_CONNECTING 0
/**
 * @brief Communicating.
 */
#define WS_STATE_OPEN       1
/**
 * @brief Closing state.
 */
#define WS_STATE_CLOSING    2
/**
 * @brief Closed.
 */
#define WS_STATE_CLOSED     3
/**@}*/

/**
 * @name Timeout util
 */
/**@{*/
/**
 * @brief Nanoseconds macro converter
 */
#define MS_TO_NS(x) ((x)*1000000)
/**
 * @brief Timeout in milliseconds.
 */
#define TIMEOUT_MS (500)
/**@}*/


/**
 * @name Handshake constants.
 */
/**@{*/
/**
 * @brief Debug
 */
#ifdef VERBOSE_MODE
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG(...)
#endif
/**@}*/



/**
 * @brief Checks is a given opcode @p opcode is valid or not.
 *
 * @param opcode Frame opcode to be checked.
 *
 * @return Returns 1 if valid, 0 otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
int is_valid_frame(int opcode);

/**
 * @brief Checks is a given opcode @p frame
 * belongs to a control frame or not.
 *
 * @param frame Frame opcode to be checked.
 *
 * @return Returns 1 if is a control frame, 0 otherwise.
 *
 * @attention This is part of the internal API and is documented just
 * for completeness.
 */
int is_control_frame(int frame);


/**
 * @brief Given a PONG message, decodes the content
 * as a int32_t number that corresponds to our
 * PONG id.
 *
 * @param msg Content to be decoded.
 *
 * @return Returns the PONG id.
 */
int32_t pong_msg_to_int32(uint8_t *msg);


/**
 * @brief Given a PING id, encodes the content to be sent
 * as payload of a PING frame.
 *
 * @param ping_id PING id to be encoded.
 * @param msg Target buffer.
 */
void int32_to_ping_msg(int32_t ping_id, uint8_t *msg);