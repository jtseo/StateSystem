#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSFParser.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"
#include "BaseSCMysql.h"
#include "BaseJson.h"

PtObjectCpp(BaseSFParser);
// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSFParser::s_func_hash_a;
const char* BaseSFParser::s_class_name = "BaseSFParser";

int BaseSFParser::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseSFParser::BaseSFParser()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFParser::FunctionProcessor);
}

int BaseSFParser::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(Open_varF);
		STDEF_SFREGIST(wordTake_strIf);
		STDEF_SFREGIST(lineNext_nF);
		STDEF_SFREGIST(labelLoad_strF);
		STDEF_SFREGIST(jsonSet_varF);
		STDEF_SFREGIST(jsonGet_varF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseSFParser::~BaseSFParser()
{

}

void BaseSFParser::init()
{
	BaseStateFunc::init();
	m_parser = NULL;
}

void BaseSFParser::release()
{
	if (m_parser)
		PT_OFree(m_parser);
}

BaseStateFuncEx* BaseSFParser::CreatorCallback(const void* _param)
{
	BaseSFParser* bs_func;
	PT_OAlloc(bs_func, BaseSFParser);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFParser::FunctionProcessor);

	return bs_func;
}

int BaseSFParser::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(wordTake_strIf);
		STDEF_SFFUNCALL(lineNext_nF);
		STDEF_SFFUNCALL(labelLoad_strF);
		STDEF_SFFUNCALL(jsonSet_varF);
		STDEF_SFFUNCALL(jsonGet_varF);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}

int BaseSFParser::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSFParser* bs_func = (BaseSFParser*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSFParser::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseSFParser::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
	//GroupLevelSet(101);

	m_parsingText = (const char*)paramFallowGet(0);

	if (!m_parser)
		PT_OAlloc(m_parser, BaseFile);

	if (m_parser->OpenFile((void*)m_parsingText.c_str(), (UINT32)m_parsingText.size()))
		return 0;

	m_parser->read_asc_line();

	const char* filepath = (const char*)paramFallowGet(1);
	
	if (!filepath)
		return 0;

	BaseFile file;
	file.set_asc_seperator("\t");
	if(file.OpenFile(filepath, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return 0;

	if (!file.read_asc_line())
		return 0;

	m_label_a.clear();
	STLString str;
	do {
		str = file.read_asc_str();
		if (!str.empty())
			m_label_a.push_back(str);
	} while (!str.empty());

	file.CloseFile();

	return 1;
}

int BaseSFParser::wordTake_strIf()
{
	const char* seperator = (const char*)m_param_value;

	if(seperator)
		m_parser->set_asc_seperator(seperator);

	STLString tag = m_parser->read_asc_str();

	const char* target = (const char*)paramFallowGet(0);
	int i = 0; 
	while (!tag.empty())
	{
		if (m_label_a[i] == target)
		{
			paramFallowSet(1, tag.c_str());
			return 1;
		}
		tag = m_parser->read_asc_str();
		i++;
		if (m_label_a[i] == "Size")
			i++;
	}

	return 0;
}

int BaseSFParser::lineNext_nF()
{
	if (!m_parser->read_asc_line())
		return 0;
	return 1;
}

int BaseSFParser::labelLoad_strF()
{
	return 1;
}

int BaseSFParser::jsonSet_varF()
{
	const char *name = NULL;
	const void *value = NULL;
	STLString json = "{";
	int i=0;
	do{
		name = (const char*)paramFallowGet(i);
		if(name){
			if(i!=0)
				json += ',';
			json += name;
			json += ':';
			EDstType type;
			value = paramFallowGet(++i, &type);
			if(value)
			{
				switch(type)
				{
					case EDstType::TYPE_STRING:
						json += (const char*)value;
						break;
					case EDstType::TYPE_INT32:
						json += BaseFile::to_str(*(const int*)value);
						break;
					case EDstType::TYPE_INT64:
						json += BaseFile::to_str(*(const INT64*)value);
						break;
					default:
						return 0;
				}
			}
		}
		i++;
	}while(name);
	json += "}";
	paramVariableSet(json.c_str());
	return 1;
}

// extract value from the json param.
int BaseSFParser::jsonGet_varF()
{
	const char *param = (const char*)paramVariableGet();
	
	BaseJson json;
	json.set(param);
	
	const char *name;
	STLString value_str;
	int value_n;
	INT64 value_n64;
	int i=0;
	do{
		EDstType type;
		name = (const char*)paramFallowGet(i);
		if(name){
			i++;
			if(!paramFallowTypeGet(i, &type))
				return 0;
			
			switch(type){
				case TYPE_STRING:
					value_str = json[name];
					if(!value_str.empty())
						paramFallowSet(i, value_str.c_str());
					break;
				case TYPE_INT32:
					if(json.get(name, &value_n))
						paramFallowSet(i, &value_n);
					break;
				case TYPE_INT64:
					if(json.get(name, &value_n64))
						paramFallowSet(i, &value_n64);
					break;
				default:
					return 0;
			}
		}
		i++;
	}while(name);
	return 1;
}
//#SF_functionInsert
