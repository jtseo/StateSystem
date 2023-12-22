#include "stdafx.h"
#include <assert.h>
#include <math.h>


BaseMemoryPoolMultiThread	s_mpool;

BaseMemoryPoolMultiThread& mpool_get()
{
	return s_mpool;
}

void check_debug_m_privateCreateType(int _nCheck)
{
	B_ASSERT(_nCheck==1);
}

void pt_optionSet(char *_opt_member, char _opt)
{
	if((*_opt_member & _opt) != 0)
		return;
	*_opt_member ^= _opt;
}