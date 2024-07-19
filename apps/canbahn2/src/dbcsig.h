#pragma once

typedef struct {
	char name[128];
	int type;
	int order;
	int mode;
	int start;
	int length;
	double factor;
	double offset;
	double min;
	double max;
	char unit[128];
} dbcsig_meta_t;

int dbcsig_meta_bitpos_to_signal(dbcsig_meta_t meta[], int length, int bitpos);