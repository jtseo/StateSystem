#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseTextEditor.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

PtObjectCpp(BaseTextEditor);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseTextEditor::s_func_hash_a;
const char* BaseTextEditor::s_class_name = "BaseTextEditor";

int BaseTextEditor::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0)
		s_iId = STRTOHASH(s_class_name);

	return s_iId;
}

BaseTextEditor::BaseTextEditor()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseTextEditor::FunctionProcessor);
}

int BaseTextEditor::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enumtext_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
        STDEF_SFREGIST(Open_varF);
        STDEF_SFREGIST(Create_varF);
        STDEF_SFREGIST(Close_varF);
        STDEF_SFREGIST(classPathGet_varF);
		STDEF_SFREGIST(definePathGet_varF);
        STDEF_SFREGIST(functionLineNumGet_varF);
        STDEF_SFREGIST(updateStart_varIf);
        STDEF_SFREGIST(updateSearch_formatIf);
        STDEF_SFREGIST(updateAdd_formatF);
        STDEF_SFREGIST(updateEnd_nF);
		STDEF_SFREGIST(classNameGet_varF);
		STDEF_SFREGIST(functionNameGet_varF);
		STDEF_SFREGIST(updateBraceComment_nF);
		STDEF_SFREGIST(updateSearch2_formatIf);
		STDEF_SFREGIST(lineCast_nF);
		STDEF_SFREGIST(exportFile_varF);
		STDEF_SFREGIST(seperatorPush_strF);
		STDEF_SFREGIST(seperatorPop_nF);
		STDEF_SFREGIST(updateAdd_vformatF);
		STDEF_SFREGIST(classPathSet_varF);
		STDEF_SFREGIST(definePathSet_varF);
		STDEF_SFREGIST(defineClassPathGet_varF);
        //#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseTextEditor::~BaseTextEditor()
{

}

void BaseTextEditor::init()
{
	BaseStateFunc::init();
}

void BaseTextEditor::release()
{
}

BaseStateFuncEx* BaseTextEditor::CreatorCallback(const void* _param)
{
	BaseTextEditor* bs_func;
	PT_OAlloc(bs_func, BaseTextEditor);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseTextEditor::FunctionProcessor);

	return bs_func;
}

int BaseTextEditor::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
    int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

    if (ret == 0)
    {
        //#SF_FuncCallStart
        if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
        //STDEF_SFFUNCALL(Open_varF);
        STDEF_SFFUNCALL(Open_varF);
        STDEF_SFFUNCALL(Create_varF);
        STDEF_SFFUNCALL(Close_varF);
        STDEF_SFFUNCALL(classPathGet_varF);
        STDEF_SFFUNCALL(definePathGet_varF);
        STDEF_SFFUNCALL(functionLineNumGet_varF);
        STDEF_SFFUNCALL(updateStart_varIf);
        STDEF_SFFUNCALL(updateSearch_formatIf);
        STDEF_SFFUNCALL(updateAdd_formatF);
        STDEF_SFFUNCALL(updateEnd_nF);
        STDEF_SFFUNCALL(classNameGet_varF);
        STDEF_SFFUNCALL(functionNameGet_varF);
        STDEF_SFFUNCALL(updateBraceComment_nF);
        STDEF_SFFUNCALL(updateSearch2_formatIf);
        STDEF_SFFUNCALL(lineCast_nF);
        STDEF_SFFUNCALL(exportFile_varF);
        STDEF_SFFUNCALL(seperatorPush_strF);
        STDEF_SFFUNCALL(seperatorPop_nF);
        STDEF_SFFUNCALL(updateAdd_vformatF);
		STDEF_SFFUNCALL(classPathSet_varF);
		STDEF_SFFUNCALL(definePathSet_varF);
		STDEF_SFFUNCALL(defineClassPathGet_varF);
		//#SF_FuncCallInsert
        return 0;
    }
    return ret;
}
int BaseTextEditor::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseTextEditor* bs_func = (BaseTextEditor*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseTextEditor::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseTextEditor::Create()
{
	if (!BaseStateFunc::Create())
		return 0;
    //GroupLevelSet(0);

    const char *_columnName;
    if(!m_state_variable->get(STRTOHASH("TempString_strV"), (const void**)&_columnName))
        return 0;
	//m_dwg_p = BaseResFileDWG::dwg_get(m_param_base);
    STLString columnname = _columnName;
    m_definePath = "State/";
#ifdef _WIN32
    m_definePath = "../State/";
    STLString filepath = "";
#else
    STLString filepath;
#endif
    m_className = columnname.substr(0, columnname.find('.'));
    m_definePath += m_className;
    m_definePath += ".define";
    
    m_function = columnname.substr(columnname.find('.')+1, columnname.size());
    STLString path = m_definePath;
    
    filepath += path;
    BaseFile file;
    if(file.OpenFile(filepath.c_str(), BaseFile::OPEN_READ|BaseFile::OPEN_UTF8))
        return 0;

    file.read_asc_line();
    m_classPath = file.read_asc_str();
    m_classPath = m_classPath.substr(3, m_classPath.size());

    file.CloseFile();

    filepath = filepath.substr(0, filepath.find_last_of('/') + 1);
    filepath += "BaseStateClasses.define";
    if (file.OpenFile(filepath.c_str(), BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
        return 0;

    file.read_asc_line();
    file.read_asc_str(); // class name 

    while (file.read_asc_string(&path))
    m_projectPath_a.push_back(path);

    file.CloseFile();

    m_updatefileRead = NULL;
    m_updatefileWrite = NULL;

    return 1;
}

int BaseTextEditor::Create_varF()
{
  const char* filename;
  filename = (const char*)paramVariableGet();
  if (filename == NULL)
      return 0;

  if (m_updatefileWrite)
    PT_OFree(m_updatefileWrite);
  PT_OAlloc(m_updatefileWrite, BaseFile);
  m_updatefileWrite->set_asc_seperator("");
  m_updatefileWrite->set_asc_deletor("");
  if (m_updatefileWrite->OpenFile(filename, BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
    return 0;
  return 1;
}

int BaseTextEditor::Open_varF()
{
    const char *filename = (const char*)paramVariableGet();
    if(!filename)
        return 0;

    if (m_updatefileRead)
        PT_OFree(m_updatefileRead);
    PT_OAlloc(m_updatefileRead, BaseFile);
    m_updatefileRead->set_asc_seperator("");
    m_updatefileRead->set_asc_deletor("");
	
	if (m_updatefileRead->OpenFile(filename, BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return 0;

    return 1;
}

int BaseTextEditor::Close_varF()
{
    if(!m_updatefileRead)
        return 0;

    m_updatefileRead->CloseFile();
    PT_OFree(m_updatefileRead);
    m_updatefileRead = NULL;
    return 1;
}

int BaseTextEditor::classPathGet_varF()
{
    paramVariableSet(m_classPath.c_str());
    return 1;
}

int BaseTextEditor::functionLineNumGet_varF()
{
#ifdef _WIN32
    STLString filepath = "../";
#else
    STLString filepath;
#endif
    STLString path = m_classPath;
    path += ".cpp";// should get root path from global variable
    filepath += path;
    BaseFile file;
    if (file.OpenFile(filepath.c_str(), BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
        return 0;
    int linenum = 1;
    STLString find_str = m_className + "::" + m_function;

    file.set_asc_seperator(" (");
    file.set_asc_deletor(" \t");
    bool find = false;
    while (file.read_asc_line())
    {
        linenum++;
        STLString read_str = file.read_asc_str();
        read_str = file.read_asc_str();
        printf("%s\n", read_str.c_str());
        if (read_str == find_str)
        {
            find = true;
            break;
        }
    }

    paramVariableSet(path.c_str());
    paramFallowSet(0, &linenum);
    file.CloseFile();
    return 1;
}

int BaseTextEditor::definePathGet_varF()
{
    STLString path = m_definePath;

    paramVariableSet(path.c_str());
    //paramFallowSet(0, &linenum);
    return 1;
}

int BaseTextEditor::updateStart_varIf()
{
    const char* filepath = (const char*)paramVariableGet();
    if (!filepath)
        return 0;

    STLString path;
#ifdef _WIN32
    //path = "../";
#endif
    path += filepath;
    STLString bak_str = path + ".bak";

    remove(bak_str.c_str());

    if (rename(path.c_str(), bak_str.c_str()) != 0)
        return 0;

    if (m_updatefileRead)
        PT_OFree(m_updatefileRead);
    PT_OAlloc(m_updatefileRead, BaseFile);
    m_updatefileRead->set_asc_seperator("");
    m_updatefileRead->set_asc_deletor("");
    if (m_updatefileRead->OpenFile(bak_str.c_str(), BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
        return 0;
    
    if (m_updatefileWrite)
        PT_OFree(m_updatefileWrite);
    PT_OAlloc(m_updatefileWrite, BaseFile);
    m_updatefileWrite->set_asc_seperator("");
    m_updatefileWrite->set_asc_deletor("");
    if (m_updatefileWrite->OpenFile(path.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
        return 0;

    return 1;
}

int BaseTextEditor::updateSearch_formatIf()
{
	if(!m_updatefileRead
	   || !m_updatefileWrite)
		return 0;
	
    STLString out_format;
    STLString search_str = paramFormatGet();

    STLString buff;

    size_t pos;
    do {
        if (!m_updatefileRead->read_asc_line())
            return 0;
        buff = m_updatefileRead->read_asc_str();
		
        pos = buff.find(search_str);
        if (pos == STLString::npos)
        {
            m_updatefileWrite->write_asc_string(buff.c_str(), 0);
            m_updatefileWrite->write_asc_line();
        }
    } while (pos == STLString::npos);

    out_format = buff.substr(0, pos);
    out_format += "%s";
    if(buff.size() > pos+search_str.size())
        out_format += buff.substr(pos+search_str.size(), buff.size());

	const char *check = (const char*)paramFallowGet(0);
	if(check)
		paramFallowSet(1, out_format.c_str());
    return 1;
}

int BaseTextEditor::updateAdd_formatF()
{
    STLString add_str = paramFormatGet();
    m_updatefileWrite->write_asc_string(add_str.c_str(), 0);
    m_updatefileWrite->write_asc_line();
    return 1;
}

int BaseTextEditor::updateEnd_nF()
{
    STLString buff;
	if(m_updatefileRead)
	{
		while(m_updatefileRead->read_asc_line())
		{
			buff = m_updatefileRead->read_asc_str();
			
			m_updatefileWrite->write_asc_string(buff.c_str(), 0);
			m_updatefileWrite->write_asc_line();
		}
		
		m_updatefileRead->CloseFile();
		if (m_updatefileRead)
			PT_OFree(m_updatefileRead);
		m_updatefileRead = NULL;
	}
	
    m_updatefileWrite->CloseFile();
    if (m_updatefileWrite)
        PT_OFree(m_updatefileWrite);
    m_updatefileWrite = NULL;
    return 1;
}

int BaseTextEditor::classNameGet_varF()
{
    paramVariableSet(m_className.c_str());
    return 1;
}

int BaseTextEditor::functionNameGet_varF()
{
    paramVariableSet(m_function.c_str());
    return 1;
}

int BaseTextEditor::_updateBraceCommentInLine(STLString& _str)
{
    STLString str;
    int ret = 1;

    while (m_updatefileRead->read_asc_string(&str))
    {
        m_updatefileWrite->write_asc_string(str.c_str(), 0);
        int last = m_updatefileRead->get_seperator_last();
        if (last == '{')
        {
            m_updatefileWrite->write_asc_string("{", 0);
            ret = _updateBraceComment(str);
        }
        else if (last == '}')
        {
            m_updatefileWrite->write_asc_string("}", 0);
            return 1;
        }
    }
    return 0;
}

int BaseTextEditor::_updateBraceComment(STLString &_str)
{
    STLString str;
    int ret = 1;

    ret = _updateBraceCommentInLine(_str);
    if (ret == 1)
        return 1;
    while (!m_updatefileRead->read_asc_line())
    {
        m_updatefileWrite->write_asc_string("//", 0);
        ret = _updateBraceCommentInLine(_str);
        if (ret == 1)
            return 1;
        m_updatefileWrite->write_asc_line();
    }

    return ret;
}

int BaseTextEditor::updateBraceComment_nF()
{
    if (!m_updatefileRead
        || !m_updatefileWrite)
        return 0;

    STLString search_str = "{";

    STLString buff;
    size_t pos;

    if (!m_updatefileRead->read_asc_line())
        return 0;
    buff = m_updatefileRead->read_asc_str();

    pos = buff.find(search_str);
    if (pos == STLString::npos)
    {
        m_updatefileWrite->write_asc_string(buff.c_str(), 0);
        m_updatefileWrite->write_asc_line();
        return 0;
    }

    m_updatefileWrite->write_asc_string("//", 0);
    m_updatefileWrite->write_asc_string(buff.c_str(), 0);
    m_updatefileWrite->write_asc_line();
    m_updatefileRead->set_asc_seperator("{}");
    int ret = _updateBraceComment(buff);
    m_updatefileRead->set_asc_seperator("");
    return ret;
}


int BaseTextEditor::_replaceString(STLString &_loading, STLString &_target, STLString &_replace, size_t _pos)
{
	size_t pos;
	
	pos = _loading.find(_target, _pos);
	STLString out;
	if (pos == STLString::npos)
	{
		out = _loading.substr(_pos, _loading.size() - _pos);
		m_updatefileWrite->write_asc_string(out.c_str(), 0);
		return 0;
	}else{
		out = _loading.substr(_pos, pos - _pos);
		out += _replace;
		m_updatefileWrite->write_asc_string(out.c_str(), 0);
		if(_loading.size() > pos+_target.size())
			return 1+_replaceString(_loading, _target, _replace, pos+_target.size());
	}
	return 1;
}

int BaseTextEditor::updateSearch2_formatIf()
{
	if(!m_updatefileRead
	   || !m_updatefileWrite)
		return 0;
	
    STLString out_format;
    STLString search_str = paramFormatGet();
	STLString replace_str = (const char*)paramFallowGet(0);

    STLString buff;

	m_updatefileRead->read_asc_line();
	int cnt = 0;
    do {
        buff = m_updatefileRead->read_asc_str();
		cnt += _replaceString(buff, search_str, replace_str, 0);
		m_updatefileWrite->write_asc_line();
    } while (m_updatefileRead->read_asc_line());
	if(cnt == 0)
		return 0;
	return 1;
}
int BaseTextEditor::lineCast_nF()
{
    const int *event_pn = (const int*)m_param_value;
    if (!m_updatefileRead)
        return 0;
    
    while(m_updatefileRead->read_asc_line()){
        STLString line_str;
        line_str = m_updatefileRead->read_asc_str();

        BaseDStructureValue* evt = EventMake(*event_pn);
		evt->set_alloc("param1_str", (void*)line_str.c_str());
		EventPost(evt, m_state_p->obj_serial_get());
    }
	
	EventEndPost(*event_pn, m_state_p->obj_serial_get());
	return 1;
}
int BaseTextEditor::exportFile_varF()
{
    const char *filename = (const char*)paramVariableGet();

    if(!filename)
        return 0;
    BaseFile file;
    if(file.OpenFile(filename, BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
        return 0;

    file.Write(m_header_str.c_str(), (int)m_header_str.size());
    file.Write(m_contents_str.c_str(), (int)m_contents_str.size());
    file.Write(m_tail_str.c_str(), (int)m_tail_str.size());

    file.CloseFile();

	return 1;
}
int BaseTextEditor::seperatorPush_strF()
{
    const char*sep_str = (const char*)m_param_value;

    STLString sep;
    if(!m_updatefileRead)
        return 0;
    m_seperator_stack_a.push_back(m_seperator_str);
    m_seperator_str = sep_str;
    m_updatefileRead->set_asc_seperator(sep_str);
	return 1;
}
int BaseTextEditor::seperatorPop_nF()
{
    STLString sep;
    if(!m_updatefileRead)
        return 0;
    if(m_seperator_stack_a.empty())
        return 0;
    m_seperator_str = m_seperator_stack_a.back();
    m_seperator_stack_a.erase(m_seperator_stack_a.begin()+(m_seperator_stack_a.size()-1));
    m_updatefileRead->set_asc_seperator(m_seperator_str.c_str());
	return 1;
}

void replace(STLString *_str, STLString _find, STLString _new)
{
	size_t idx = 0;
	idx = _str->find(_find);
	while(idx >= 0)
	{
		_str->replace(idx, _find.length(), _new);
		idx = _str->find(_find);
	}
}
void convertfile_str(STLString *_str)
{
	replace(_str, "\\t", "\t");
	replace(_str, "\\n", "\n");
	replace(_str, "\\r", "\r");
}

void BaseTextEditor::analysisHeaderAdd(const STLString _str)
{
	STLString str = _str;
	convertfile_str(&str);
    m_header_str = str;
    m_contents_str = "";
    m_tail_str = "";
}

void BaseTextEditor::analysisContentsAdd(const STLString _str)
{
	STLString str = _str;
	convertfile_str(&str);
	m_contents_str += str;
}

void BaseTextEditor::analysisTailAdd(const STLString _str)
{
	STLString str = _str;
	convertfile_str(&str);
	m_tail_str = str;
}
int BaseTextEditor::updateAdd_vformatF()
{
    STLString add_str = paramVariableFormatGet();
    m_updatefileWrite->write_asc_string(add_str.c_str(), 0);
    m_updatefileWrite->write_asc_line();
    return 1;
}

int BaseTextEditor::classPathSet_varF()
{
	m_className = (const char*)paramVariableGet();
	m_definePath = "State/";
#ifdef _WIN32
    m_definePath = "../State/";
#endif
	m_definePath += m_className;
	m_definePath += ".define";
	return 1;
}

int BaseTextEditor::definePathSet_varF()
{
	return 1;
}

int BaseTextEditor::defineClassPathGet_varF()
{
	const char* path = "State/BaseStateClasses.define";
	if(!paramVariableSet(path))
		return 0;
	return 1;
}
//#SF_functionInsert
