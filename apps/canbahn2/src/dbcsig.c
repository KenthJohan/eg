#include "dbcsig.h"


int dbcsig_meta_bitpos_to_signal(dbcsig_meta_t metas[], int length, int bitpos)
{
	for(int i = 0; i < length; ++i) {
		if (metas[i].start > bitpos) {
			continue;
		}
		if ((metas[i].start + metas[i].length) <= bitpos) {
			continue;
		}
		return i;
	}
	return -1;
}