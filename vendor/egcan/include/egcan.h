#pragma once



#include <flecs.h>
#include <stdint.h>

typedef struct {
	void * internal;
} EgCanEpoll;

typedef struct {
	char const * interface;
	int32_t signals_amount;
	int32_t error;
} EgCanBusDescription;


#define EG_CAN_BOOK_CAP 1024
typedef struct {
	int len;
	int dirty;
	uint8_t payload[8];
} eg_can_book_packet8_t;

typedef struct {
	int32_t sock;
	int32_t cap;
	eg_can_book_packet8_t * tx;
	eg_can_book_packet8_t * rx;
} eg_can_book_t;

typedef struct {
	int32_t socket;
	eg_can_book_t * ptr;
} EgCanBus;

typedef struct {
	int32_t index;
	int32_t can_bitrate;
	int32_t can_clock;
	int32_t mtu;
	int32_t tso_max_size;
	int32_t numtxqueues;
	int32_t numrxqueues;
	int32_t minmtu;
	int32_t maxmtu;
	int32_t stats64_rx_bytes;
	int32_t stats64_rx_packets;
	int32_t stats64_rx_errors;
	int32_t stats64_tx_bytes;
	int32_t stats64_tx_packets;
	int32_t stats64_tx_errors;
} EgCanInterface;








/*
https://docs.openvehicles.com/en/latest/components/vehicle_dbc/docs/dbc-primer.html
https://www.csselectronics.com/pages/can-dbc-file-database-intro
https://canlogger.csselectronics.com/dbc-editor/v129/dbc-editor.html
*/

typedef struct {
	uint32_t canid;
	int32_t len;
	int32_t rx;
	int32_t tx;
	int32_t byte_offset;
	int32_t min;
	int32_t max;
	int32_t gui_index;
	/*
	uint8_t type;
	uint8_t order;
	uint8_t mode;
	uint8_t start;
	uint8_t length;
	float factor;
	uint8_t offset;
	uint8_t min;
	*/
} EgCanSignal;

extern ECS_COMPONENT_DECLARE(EgCanEpoll);
extern ECS_COMPONENT_DECLARE(EgCanBusDescription);
extern ECS_COMPONENT_DECLARE(EgCanBus);
extern ECS_COMPONENT_DECLARE(EgCanSignal);
extern ECS_COMPONENT_DECLARE(EgCanInterface);

void eg_can_book_prepare_send(eg_can_book_t * book, EgCanSignal * signal);

void EgCanImport(ecs_world_t *world);
