#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "UdpChannelServer.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(UdpChannelServer);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	UdpChannelServer::s_func_hash_a;
const char* UdpChannelServer::s_class_name = "UdpChannelServer";

int UdpChannelServer::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

UdpChannelServer::UdpChannelServer()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, UdpChannelServer::FunctionProcessor);
}

int UdpChannelServer::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		//#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

UdpChannelServer::~UdpChannelServer()
{

}

void UdpChannelServer::init()
{
	BaseStateFunc::init();
}

void UdpChannelServer::release()
{
}

BaseStateFuncEx* UdpChannelServer::CreatorCallback(const void* _param)
{
	UdpChannelServer* bs_func;
    if(_param){
        bs_func = (UdpChannelServer*)_param;
    }else{
        PT_OAlloc(bs_func, UdpChannelServer);
        
        bs_func->init();
        StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, UdpChannelServer::FunctionProcessor);
    }
	return bs_func;
}

int UdpChannelServer::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int UdpChannelServer::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	UdpChannelServer* bs_func = (UdpChannelServer*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, UdpChannelServer::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int UdpChannelServer::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

//#SF_functionInsert
