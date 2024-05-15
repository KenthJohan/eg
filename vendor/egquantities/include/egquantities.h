#pragma once

#include <flecs.h>


/*
The International System of Quantities (ISQ) is a standard system of quantities used in physics and in modern science in general. 
It includes basic quantities such as length and mass and the relationships between those quantities.[a] 
This system underlies the International System of Units (SI)[b] but does not itself determine the units of measurement used for the quantities.
The system is formally described in a multi-part ISO standard ISO/IEC 80000 (which also defines many other quantities used in science and technology), 
first completed in 2009 and subsequently revised and expanded. 
The base quantities of a given system of physical quantities is a subset of those quantities, 
where no base quantity can be expressed in terms of the others, 
but where every quantity in the system can be expressed in terms of the base quantities. 
Within this constraint, the set of base quantities is chosen by convention. 
There are seven ISQ base quantities. The symbols for them, as for other quantities, are written in italics.[1]
*/

typedef struct {
	char const * symbol;
	int32_t length;
	int32_t mass;
	int32_t time;
	int32_t current;
	int32_t temperature;
	int32_t substance;
	int32_t intensity;
} EgQuantitiesIsq;

typedef struct {
	float min;
	float max;
	float value;
} EgQuantitiesProgress;


typedef union {
	float val_f32;
	double val_f64;
	int8_t val_i8;
	uint8_t val_u8;
	int16_t val_i16;
	uint16_t val_u16;
	int32_t val_i32;
	uint32_t val_u32;
	int64_t val_i64;
	uint64_t val_u64;
} eg_generic_number_t;

typedef struct {
	ecs_primitive_kind_t kind;
	eg_generic_number_t min;
	eg_generic_number_t max;
	eg_generic_number_t rx;
	eg_generic_number_t tx;

	// TODO: Remove these:
	float min_f32;
	float max_f32;

	double min_f64;
	double max_f64;

	int64_t min_i64;
	int64_t max_i64;

	uint64_t min_u64;
	uint64_t max_u64;
} EgQuantitiesRangedGeneric;



extern ECS_COMPONENT_DECLARE(EgQuantitiesIsq);
extern ECS_COMPONENT_DECLARE(EgQuantitiesProgress);
extern ECS_COMPONENT_DECLARE(EgQuantitiesRangedGeneric);

extern ECS_TAG_DECLARE(EgQuantitiesVoltage);


void EgQuantitiesImport(ecs_world_t *world);