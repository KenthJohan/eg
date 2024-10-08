#pragma once
#include "app.h"
#include "CanDbc.h"


typedef struct {
	bool selected;
	bool hover1; // Hover on signal table
	bool hover2; // Hover on bit table
	bool clicked; // Hover on bit table
} app_gui_cansig_state_t;

void app_gui_cansig_table1(app_gui_cansig_state_t guistates[], CanDbcSignal metas[], int cansig_count, int message_length);
void app_gui_cansig_table2(app_gui_cansig_state_t guistates[], CanDbcSignal metas[], int cansig_count);
void app_gui_cansig_table3(app_gui_cansig_state_t guisigs[], CanDbcSignal metas[], int cansig_count);