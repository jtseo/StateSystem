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
		STDEF_SFREGIST(ChannelGet_varF);
		STDEF_SFREGIST(ChannelAdd_varF);
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
    
    for(int i=0; i<STD_ChannelType::Max; i++)
        m_lastChannels[i] = -1;
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
		STDEF_SFFUNCALL(ChannelGet_varF);
		STDEF_SFFUNCALL(ChannelAdd_varF);
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


int UdpChannelServer::m_typedef[] = {0, 0, 0, 0};
STD_ChannelType UdpChannelServer::TypeGet(const char* _type)
{
    if(m_typedef[STD_ChannelType::None] == 0)
    {
        m_typedef[STD_ChannelType::None] = STRTOHASH("None");
        m_typedef[STD_ChannelType::Pro] = STRTOHASH("Pro");
        m_typedef[STD_ChannelType::Dev] = STRTOHASH("Dev");
        m_typedef[STD_ChannelType::Enter] = STRTOHASH("Enter");
    }
    
    int hash = STRTOHASH(_type);
    for(int i=0;i < STD_ChannelType::Max; i++)
    {
        if(hash == m_typedef[i])
            return (STD_ChannelType)i;
    }
    return STD_ChannelType::None;
}

int UdpChannelServer::ChannelGet(STD_ChannelType _type, int _last)
{
    int channel = 0;
    for(int i=0; i<m_stlVChannel.size(); i++)
    {
        if(m_stlVChannel[i].type == _type)
        {
            channel = i;
            if(i > _last)
                break;
        }
    }
    return channel;
}

int UdpChannelServer::ChannelGet_varF()
{
    const char* typeStr = (const char*)paramVariableGet();
    
    STD_ChannelType type = TypeGet(typeStr);
    
    if(m_stlVChannel.size() == 0)
        return 0;
    
    int channel = ChannelGet(type, m_lastChannels[type]);
    
    if(m_lastChannels[type] == channel)
        channel = ChannelGet(type, -1);
    
    m_lastChannels[type] = channel;
    
    paramFallowSet(0, m_stlVChannel[channel].name);
    paramFallowSet(1, m_stlVChannel[channel].ip);
	return 1;
}


int UdpChannelServer::ChannelAdd_varF()
{
    const char* type = (const char*)paramVariableGet();
    const char* name = (const char*)paramFallowGet(0);
    const char* ip = (const char*)paramFallowGet(1);
    
    STD_UdpChannel channel;
    channel.type = TypeGet(type);
    
    strcpy_s(channel.name, 50, name);
    strcpy_s(channel.ip, 20, ip);
    
    for(int i=0; i<m_stlVChannel.size(); i++)
    {
        if(strcmp(m_stlVChannel[i].ip, ip) == 0)
            return 0;
    }
    m_stlVChannel.push_back(channel);
    
	return 1;
}
//#SF_functionInsert
