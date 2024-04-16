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


extern ECS_COMPONENT_DECLARE(EgQuantitiesIsq);

extern ECS_TAG_DECLARE(EgQuantitiesVoltage);


void EgQuantitiesImport(ecs_world_t *world);