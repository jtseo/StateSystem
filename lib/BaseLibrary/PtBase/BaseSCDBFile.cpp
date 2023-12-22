#include "stdafx.h"

#include <winsock.h>
#include <windows.h>

//#include <sys/ioctl.h>
#include <inttypes.h>

//#include "../PtBase/BaseStateFunc.h"
//#include "../PtBase/BaseObject.h"

#include "BaseSCDBFile.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"
#include "../PtBase/BaseSystem.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseSCDBFile);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSCDBFile::s_func_hash_a;
const char* BaseSCDBFile::s_class_name = "BaseSCDBFile";

int BaseSCDBFile::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSCDBFile::BaseSCDBFile()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCDBFile::FunctionProcessor);
}

int BaseSCDBFile::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(open_astrF);
		STDEF_SFREGIST(close_nF);
		STDEF_SFREGIST(connect_astrF);
		STDEF_SFREGIST(apiReqCast_varIf);
		STDEF_SFREGIST(apiReturn_varF);
		STDEF_SFREGIST(read_strF);
		STDEF_SFREGIST(update_strF);
		STDEF_SFREGIST(remove_strF);
		STDEF_SFREGIST(create_strF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSCDBFile::~BaseSCDBFile()
{

}

void BaseSCDBFile::init()
{
	BaseStateFunc::init();
}

void BaseSCDBFile::release()
{
}

BaseStateFuncEx* BaseSCDBFile::CreatorCallback(const void *_param)
{
	BaseSCDBFile* bs_func;
	PT_OAlloc(bs_func, BaseSCDBFile);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSCDBFile::FunctionProcessor);

	return bs_func;
}

int BaseSCDBFile::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])		return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(open_astrF);
		STDEF_SFFUNCALL(close_nF);
		STDEF_SFFUNCALL(connect_astrF);
		STDEF_SFFUNCALL(apiReqCast_varIf);
		STDEF_SFFUNCALL(apiReturn_varF);
		STDEF_SFFUNCALL(read_strF);
		STDEF_SFFUNCALL(update_strF);
		STDEF_SFFUNCALL(remove_strF);
		STDEF_SFFUNCALL(create_strF);
		//#SF_FuncCallInsert

		return 0;
	}
	return ret;
}

int BaseSCDBFile::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSCDBFile* bs_func = (BaseSCDBFile*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSCDBFile::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSCDBFile::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

    const char *filename = (const char*)paramFallowGet(0);

	
	return 1;
}

int BaseSCDBFile::read_strF()
{
	return 1;
}
int BaseSCDBFile::update_strF()
{
	return 1;
}
int BaseSCDBFile::remove_strF()
{
	return 1;
}
int BaseSCDBFile::create_strF()
{
	return 1;
}
//#SF_functionInsert
