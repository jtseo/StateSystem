#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "DevCashReader.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"


PtObjectCpp(DevCashReader);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	DevCashReader::s_func_hash_a;
const char* DevCashReader::s_class_name = "DevCashReader";

int DevCashReader::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

DevCashReader::DevCashReader()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, DevCashReader::FunctionProcessor);
}

int DevCashReader::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(DataARead_nF);
		STDEF_SFREGIST(DataAShift_nF);
		STDEF_SFREGIST(DataComplete_nIf);
		STDEF_SFREGIST(DataGet_varF);
		STDEF_SFREGIST(DataWriteSet_varF);
		STDEF_SFREGIST(ConfigWrite_nF);
		STDEF_SFREGIST(CashReadStart_nF);
		STDEF_SFREGIST(MoneyGet_varIf);
		STDEF_SFREGIST(CashReadStop_nF);
		STDEF_SFREGIST(PortInit_nF);
		STDEF_SFREGIST(DataFullCheck_nIf);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

DevCashReader::~DevCashReader()
{

}

void DevCashReader::init()
{
	BaseStateFunc::init();
}

void DevCashReader::release()
{
}

BaseStateFuncEx* DevCashReader::CreatorCallback(const void* _param)
{
	DevCashReader* bs_func;
	PT_OAlloc(bs_func, DevCashReader);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, DevCashReader::FunctionProcessor);

	return bs_func;
}

int DevCashReader::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(DataARead_nF);
		STDEF_SFFUNCALL(DataAShift_nF);
		STDEF_SFFUNCALL(DataComplete_nIf);
		STDEF_SFFUNCALL(DataGet_varF);
		STDEF_SFFUNCALL(DataWriteSet_varF);
		STDEF_SFFUNCALL(ConfigWrite_nF);
		STDEF_SFFUNCALL(CashReadStart_nF);
		STDEF_SFFUNCALL(MoneyGet_varIf);
		STDEF_SFFUNCALL(CashReadStop_nF);
		STDEF_SFFUNCALL(PortInit_nF);
		STDEF_SFFUNCALL(DataFullCheck_nIf);
		//#SF_FuncCallInsert
		return 0;
    }
    return ret;
}
int DevCashReader::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	DevCashReader* bs_func = (DevCashReader*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, DevCashReader::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int DevCashReader::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);


    return 1;
}

int DevCashReader::DataARead_nF()
{
	BYTE red;

	if (!m_serial.ReadByte(red))
		return 0;

	m_datas.push_back(red);
	return 1;
}
int DevCashReader::DataAShift_nF()
{
	m_datas.erase(m_datas.begin());
	return 1;
}
int DevCashReader::DataComplete_nIf()
{
	if (m_datas.size() < 5) // check data length
		return 0;
	if (m_datas[0] != '$') // check start value
		return 0;

	unsigned char sum = m_datas[1] + m_datas[2] + m_datas[3];

	if (sum != m_datas[4]) // check parity
		return 0;

	return 1;
}
int DevCashReader::DataGet_varF()
{
	if (m_datas.size() < 5)
		return 0;

	const char* cmd = (const char*) & m_datas.at(1);
	char buf[4];
	buf[3] = 0;
	memcpy(buf, cmd, 3);
	if (!paramVariableSet(buf))
		return 0;
	return 1;
}
int DevCashReader::DataWriteSet_varF()
{
	const char* cmd = (const char*)paramVariableGet();
	if (cmd)
		PacketMake(cmd, m_packet);
	if (!m_serial.WriteBytes(m_packet, 5))
		return 0;

	return 1;
}

bool DevCashReader::PacketMake(const char* _str, char* _packet)
{
	*_packet = '$';
	memcpy(_packet + 1, _str, 3);
	*(_packet + 4) = *(_str) + *(_str + 1) + *(_str + 2);
	return true;
}

int DevCashReader::ConfigWrite_nF()
{
	int timeout = *(int*)m_param_value;

	m_serial.SetCommunicationTimeouts(timeout, timeout, timeout, timeout, timeout);
	char cmd[3];
	cmd[0] = 'S';
	cmd[1] = 'C';
	cmd[2] = 0xE; // 50000, 10000, 5000, 1000
	PacketMake(cmd, m_packet);
	if (!m_serial.WriteBytes(m_packet, 5))
		return 0;
	return 1;
}
int DevCashReader::CashReadStart_nF()
{
	char cmd[3];
	cmd[0] = 'S';
	cmd[1] = 'A';
	cmd[2] = 0xD;
	PacketMake(cmd, m_packet);
	if (!m_serial.WriteBytes(m_packet, 5))
		return 0;
	return 1;
}
int DevCashReader::MoneyGet_varIf()
{
	if (m_datas[1] != 'g' || m_datas[2] != 'b')
		return 0;

	if (m_datas[3] != 1
		&& m_datas[3] != 5
		&& m_datas[3] != 10
		&& m_datas[3] != 50)
		return 0;

	int money = m_datas[3];
	money *= 1000;
	paramVariableSet(&money);
	return 1;
}
int DevCashReader::CashReadStop_nF()
{
	char cmd[3];
	cmd[0] = 'S';
	cmd[1] = 'A';
	cmd[2] = 0xE;
	PacketMake(cmd, m_packet);
	if (!m_serial.WriteBytes(m_packet, 5))
		return 0;
	return 1;
}
int DevCashReader::PortInit_nF()
{
	int port = *(int*)m_param_value;
	if (port < 0 || 10 < port)
		return 0;

	char buf[200];
	sprintf(buf, "COM%d", port);

	if (!m_serial.OpenPort(buf))
		return 0;

	m_serial.Configure_Port(CBR_9600, 8, FALSE, NOPARITY, ONESTOPBIT);
	m_serial.SetCommunicationTimeouts(10, 10, 10, 10, 10);
	return 1;
}
int DevCashReader::DataFullCheck_nIf()
{
	if (m_datas.size() < 5) // check data length
		return 0;
	return 1;
}
//#SF_functionInsert
