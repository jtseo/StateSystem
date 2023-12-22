#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseSFuncXML.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include "BaseXML.h"
#include "BaseResGrid.h"

PtObjectCpp(BaseSFuncXML);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseSFuncXML::s_func_hash_a;
const char* BaseSFuncXML::s_class_name = "BaseSFuncXML";

int BaseSFuncXML::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0) {
		s_iId = STRTOHASH(s_class_name);
	}

	return s_iId;
}

BaseSFuncXML::BaseSFuncXML()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFuncXML::FunctionProcessor);
}

int BaseSFuncXML::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//func_str = _class_name + ".text_info_cast_nF";	(*_func_hash)[Enum_text_info_cast_nF] = STRTOHASH(func_str.c_str());		BaseDStructure::processor_list_add(func_str.c_str(), _func, __FILE__, __LINE__);
		STDEF_SFREGIST(textRemove_strF);
		STDEF_SFREGIST(svgSave_varF);
		STDEF_SFREGIST(fileIs_nIf);
		STDEF_SFREGIST(svgAIMakeTrainDataCast_nF);
		STDEF_SFREGIST(objectAllCast_nF);
		//STDEF_SFREGIST(dwgSave_varF);
		STDEF_SFREGIST(propertyInfoExport_varF);
		STDEF_SFREGIST(svgExport_varF);
		STDEF_SFREGIST(paramEqual_astrIf);
		STDEF_SFREGIST(paramEqualBigger_astrIf);
		STDEF_SFREGIST(colorChange_nF);
		STDEF_SFREGIST(svgExportSave_varF);
		//STDEF_SFREGIST(dwgConvert_varF);
		//STDEF_SFREGIST(dwgLoad_varF);
		STDEF_SFREGIST(titleSave_varF);
		STDEF_SFREGIST(layerExist_strIf);
		STDEF_SFREGIST(layerRemove_strF);
		STDEF_SFREGIST(exportRemove_nF);
		STDEF_SFREGIST(svgFileMerge_varF);
		STDEF_SFREGIST(title2dbtitle_nF);
		STDEF_SFREGIST(dbtitle2title_nF);
		STDEF_SFREGIST(title2json_nF);
		STDEF_SFREGIST(json2title_nF);
        //#SF_FuncRegistInsert
		return _size;
	}
	else
		return 0;
}

BaseSFuncXML::~BaseSFuncXML()
{

}

void BaseSFuncXML::init()
{
	BaseStateFunc::init();
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFuncXML::FunctionProcessor);
}

void BaseSFuncXML::release()
{
	if(m_xml_p)
		m_xml_p->dec_ref(this);
	m_xml_p = NULL;
}

BaseStateFuncEx* BaseSFuncXML::CreatorCallback(const void* _param)
{
	BaseSFuncXML* bs_func;
	PT_OAlloc(bs_func, BaseSFuncXML);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseSFuncXML::FunctionProcessor);

	return bs_func;
}

int BaseSFuncXML::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	if (ret == 0)
	{
		//#SF_FuncCallStart
		if (m_func_hash == s_func_hash_a[Enum_ext_start])        return 0;
		//STDEF_SFFUNCALL(Open_varF);
		STDEF_SFFUNCALL(textRemove_strF);
		STDEF_SFFUNCALL(svgSave_varF);
		STDEF_SFFUNCALL(fileIs_nIf);
		STDEF_SFFUNCALL(svgAIMakeTrainDataCast_nF);
		STDEF_SFFUNCALL(objectAllCast_nF);
		//STDEF_SFFUNCALL(dwgSave_varF);
		STDEF_SFFUNCALL(propertyInfoExport_varF);
		STDEF_SFFUNCALL(svgExport_varF);
		STDEF_SFFUNCALL(paramEqual_astrIf);
		STDEF_SFFUNCALL(paramEqualBigger_astrIf);
		STDEF_SFFUNCALL(colorChange_nF);
		STDEF_SFFUNCALL(svgExportSave_varF);
		//STDEF_SFFUNCALL(dwgConvert_varF);
		//STDEF_SFFUNCALL(dwgLoad_varF);
		STDEF_SFFUNCALL(titleSave_varF);
		STDEF_SFFUNCALL(layerExist_strIf);
		STDEF_SFFUNCALL(layerRemove_strF);
		STDEF_SFFUNCALL(exportRemove_nF);
		STDEF_SFFUNCALL(svgFileMerge_varF);
		STDEF_SFFUNCALL(title2dbtitle_nF);
		STDEF_SFFUNCALL(dbtitle2title_nF);
		STDEF_SFFUNCALL(title2json_nF);
		STDEF_SFFUNCALL(json2title_nF);
		//#SF_FuncCallInsert
		return 0;
	}
	return ret;
}

int BaseSFuncXML::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseSFuncXML* bs_func = (BaseSFuncXML*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseSFuncXML::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

bool node_name(BaseXMLNode* _node, void* _data)
{
	const char* name = (const char*)_data;

	if (_node->name == name)
	{
		return true;
	}
	return false;
}

int BaseSFuncXML::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	//m_dwg_p = BaseResFileDWG::dwg_get(m_param_base);
	//GroupLevelSet(0);

	const char* filename = (const char*)paramFallowGet(0);
	
	if (!filename)
		return 0;
	m_xml_p = (BaseXML*) m_resmanager_p->reload_resource(filename, BaseXML::GetObjectId(), NULL, NULL, 0);

	if (!m_xml_p)
		return 0;

	m_export = NULL;
	m_xml_p->inc_ref(this);

	BaseXMLNode* root = m_xml_p->root();
	if (!root)
		return 0;

	BaseXMLNode part;
	root->take((void*)"defs", node_name, &part);
	return 1;
}

bool node_name_c(const BaseXMLNode* _node, void* _data)
{
	const char* name = (const char*)_data;

	if (_node->name == name)
	{
		return true;
	}
	return false;
}

bool nodeTextEraser(BaseXMLNode* _node, void* _data)
{
	const char* name = (const char*)_data;

	if (_node->name == "text")
	{
		STLString text = _node->text_get();
		if (BaseStringTable::find(text, name) != -1)
			return true;
	}
	return false;
}

int BaseSFuncXML::textRemove_strF()
{
	const char* tagName = (const char*)m_param_value;
	BaseXMLNode* root;
	root = m_xml_p->root();

	BaseXMLNode node;
	
	int ret = 0;
	while(root->take((void*)tagName, nodeTextEraser, &node))
	{
		ret = 1;
	}
	return ret;
}

int BaseSFuncXML::fileIs_nIf()
{
	if (m_xml_p)
		return 1;
	return 0;
}

int BaseSFuncXML::svgSave_varF()
{
	const int *hash = (const int*)m_param_value;
	const char* filename;
	if (!m_state_variable->get(*hash, (const void**)&filename))
		return 0;

	STLString filepath;
	filepath = filename;
	filepath = filepath.substr(0, filepath.find_last_of('.'));
	filepath += ".svg";

	BaseXMLNode* root = m_xml_p->root()->node_get("svg");
	STLString svgfile = root->get(0);

	BaseFile file;
	if (file.OpenFile(filepath.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return 0;

	file.Write(svgfile.c_str(), (int)svgfile.size());

	file.CloseFile();

	return 1;
}

bool node_first(BaseXMLNode* _node, void* _data)
{
	float* max = (float*)_data;
	PtMatrix4 m4;
	PtBound bnd;
	if (_node->child_pa.empty() && _node->bound_get(&m4, &bnd))
	{
		if (bnd[1].x - bnd[0].x <= *max
			&& bnd[1].y - bnd[0].y <= *max)
			return true;
	}
	return false;
}

class ParamCol
{
public:
	PtBound* bnd;
	float   max;
};

bool node_collision(BaseXMLNode* _node, void* _data)
{
	ParamCol* param = (ParamCol*)_data;
	PtBound* _bnd = param->bnd;
	PtMatrix4 m4;
	PtBound bnd;
	if ((_node->child_pa.empty() || _node->name == "text")
		&& _node->bound_get(&m4, &bnd))
	{

		if (bnd[1].x - bnd[0].x <= param->max
			&& bnd[1].y - bnd[0].y <= param->max
			&& bnd.collision(*_bnd))
			return true;
	}
	return false;
}

bool BaseSFuncXML::svgSave(BaseXMLNode* _svg, const STLString& _filename)
{
	BaseXML* arrow = (BaseXML*)m_resmanager_p->get_resource("pnid_arrow.svg", BaseXML::GetObjectId(), NULL, NULL, 0);
	_svg->child_pa.insert(_svg->child_pa.begin(), *arrow->root());

	STLString svgfile = _svg->get(0);
	
	BaseFile file;
	if (file.OpenFile(_filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return false;
	file.Write(svgfile.c_str(), (int)svgfile.size());
	file.CloseFile();
	return true;
}

bool BaseSFuncXML::boundRemove(PtBound _bnd)
{
	BaseXMLNode part;
	
	PtMatrix4 m4;
	m4.MakeIdentity();
	
	ParamCol param;
	param.bnd = &_bnd;
	param.max = 600;
	PtVector3 org_v3;
	BaseXMLNode* root_p = m_xml_p->root();
	do {		
	} while (root_p->take(&param, node_collision, &part));

	return true;
}

BaseXMLNode* nodeTake(BaseXMLNode *_root, float _max, BaseXMLNode *_out, float _extend)
{
	_out->child_pa.clear();
	BaseXMLNode part;
	if (!_root->take(&_max, node_first, &part))
		return NULL;
	
	PtBound bnd;
	PtMatrix4 m4;
	m4.MakeIdentity();
	bnd.init();

	PtVector3 org_v3;
	ParamCol param;
	do {
		_out->child_pa.push_back(part);
		_out->bound_get(&m4, &bnd);
		org_v3 = bnd[0] * -1.f;
		bnd.extend(_extend);

		param.bnd = &bnd;
		param.max = _max;
	} while (_root->take(&param, node_collision, &part));

	return _out;
}

int BaseSFuncXML::svgAIMakeTrainDataCast_nF()
{ // fallow variable for export filename
	const int* event = (const int *)m_param_value;
	float gap_af[2];
	gap_af[0] = *(const float*)paramFallowGet(0); // 15, 20 // size of image, extend bound box 
	gap_af[1] = *(const float*)paramFallowGet(1); // 15, 20 // size of image, extend bound box 
	BaseXMLNode svg;
	BaseXMLNode* root = m_xml_p->root();

	PtBound bnd;
	PtMatrix4 m4;
	m4.MakeIdentity();
	bnd.init();
	root->transform_remove(m4);

	float max = 500.f;
	BaseXMLNode part;

	svg = *root->node_get("svg");

	int grid_size = (int)gap_af[0];
	char grid_size_str[128];
	sprintf_s(grid_size_str, 128, "%d", grid_size);

	svg.set("viewBox", "");
	svg.set("style", "background-color:white");
	root->bound_get(&m4, &bnd);
	
	char buf[255];
	int cnt = 0;
	STLVString file_a;

	STLVXMLNode svg_a;
	while (root->take((void*)"svg", node_name, &part))
	{
		svg_a.push_back(part);
	}

	for (int k = 0; k < svg_a.size(); k++)
	{
		root = &svg_a[k];
		while (nodeTake(root, max, &svg, gap_af[1]))
		{
			STLString filename;
			filename = (const char*)paramFallowGet(2);
			if (filename.empty())
				return 0;

			if (cnt == 0)
			{
				STLString model = filename;
				STLString originPath = "./train_data/original/";

				svg.int_set("width", 2000);
				svg.int_set("height", 2000);
				model = model.substr(model.find_last_of('\\')+1, model.find_last_of('.'));
				model = model.substr(0, model.find_last_of('.'));
				model += ".svg";
				originPath += model;
				svgSave(&svg, originPath);
			}

			BaseProjectedVector prjVector;
			BaseProjectedVector* prjVec_p;
			PT_OAllocS(prjVec_p, BaseProjectedVector, prjVector);

			if (!prjVec_p->set(grid_size, grid_size, 4, &svg))
			{
				PT_OFreeS(prjVec_p);
				continue;
			}

			filename = filename.substr(0, filename.find_last_of('.'));
			prjVec_p->nameSet(filename.substr(filename.find_last_of('\\') + 1));
			prjVec_p->index() = cnt;

			svg.set("width", grid_size_str);
			svg.set("height", grid_size_str);

			BaseDStructureValue* evt = EventMake(*event);
			evt->set_point("ParamInt64", prjVec_p);
			EventPost(evt);

			sprintf_s(buf, 255, "_%03d.svg", cnt++);
			filename += buf;

			//if (paramFallowGet(3))
			//{
			//	prjVec_p->svgSave(filename);
			//}

			//svgSave(&svg, filename);
		}
	}

	EventEndPost(*event);
	
	return 1;
}

int BaseSFuncXML::prjectedVectorChartSave(STLString _filename)
{
	STLString filename = _filename.substr(0, _filename.find_last_of('.'));

	filename += "_diff.text";
	BaseFile file;
	file.OpenFile(filename.c_str(), BaseFile::OPEN_WRITE);
	for (int i = 0; i < m_prjVec_a.size(); i++)
	{
		for (int j = 0; j < m_prjVec_a.size(); j++)
		{
			float diff = m_prjVec_a[i].differ(&m_prjVec_a[j]);
			file.write_asc_float(diff);
		}
		file.write_asc_line();
	}

	file.CloseFile();
	return 1;
}

static BaseXMLNode s_node;

int BaseSFuncXML::objectAllCast_nF()
{
	const int* event_pn = (const int*)m_param_value;

	int serial_obj = m_state_p->obj_serial_get();

    char buf[128];
    strcpy(buf, "defs");
    
    BaseXMLNode* root = m_xml_p->root();

	PtBound bnd;
	PtMatrix4 m4;
	m4.MakeIdentity();
	bnd.init();
	root->transform_remove(m4);

	while (root->take(buf, node_name, &s_node));
	
	root->bound_get(&m4, &bnd);

	int cnt = 0;
	STLVString file_a;
	ParamCol param;
	param.bnd = &bnd;
	param.max = 100000;

	//int order = 1000;
	BaseXMLNode part;
	while (root->take(&param, node_collision, &part))
	{	
		BaseXMLNode* obj_p = NULL;
		PT_OAllocS(obj_p, BaseXMLNode, s_node);

		*obj_p = part;
		//obj_p->set("pn_order", order);
		BaseDStructureValue* evt = EventMake(*event_pn);
		evt->set_point("ParamInt64", (void*)obj_p);
		EventPost(evt);
		//order++;
	}

	EventEndPost(*event_pn);

	return 1;
}

BaseSFuncXML::NodeType BaseSFuncXML::nodeType(BaseXMLNode& _node)
{
	STLString str = _node.get("d");
	if (str.empty())
	{
		if (_node.name == "text")
			return TYPE_TEXT;
		return TYPE_UNKNOW;
	}

	if (str.back() == 'z')
		return TYPE_POLYGON;

	return TYPE_LINE;
}

void BaseSFuncXML::nodeAdd(NodeType _type, BaseStateFunc* _node)
{
	m_node[_type].push_back(_node);
}

STLVpSFunc& BaseSFuncXML::nodeGet(NodeType _type)
{
	return m_node[_type];
}

BaseStateFunc* BaseSFuncXML::nodeGet(NodeType _type, int _id)
{
	for (int i = 0; i < m_node[_type].size(); i++)
	{
		if (m_node[_type][i]->GroupIdGet() == _id)
			return m_node[_type][i];
	}
	return NULL;
}

bool BaseSFuncXML::nodeShowing(int _id)
{
	for (int type = TYPE_LINE; type < TYPE_MAX; type++)
	{
		for (int i = 0; i < m_node[type].size(); i++)
		{
			if (m_node[type][i]->GroupIdGet() == _id)
				return true;
		}
	}
	return false;
}

bool BaseSFuncXML::nodeHiden(NodeType _type, BaseStateFunc* _node, int _holder)
{
	m_hidenList_a.push_back(_node);
	return nodeErase(_type, _node, _holder);
}

const STLVpVoid & BaseSFuncXML::nodeHidenGet()
{
	return m_hidenList_a;
}

BaseStateFunc *BaseSFuncXML::nodeHidenGet(int _id)
{
	BaseStateFunc* node;
	for (int i = 0; i < m_hidenList_a.size(); i++)
	{
		node = (BaseStateFunc*)m_hidenList_a[i];
		if (node->GroupIdGet() == _id)
			return node;
	}
	return NULL;
}

bool BaseSFuncXML::nodeErase(NodeType _type, BaseStateFunc* _node, int _holder)
{
	for (int i = 0; i < m_node[_type].size(); i++)
	{
		if (_node == m_node[_type][i])
		{
			if(_holder != -1)
				propertyInfo_remove(_node->GroupIdGet(), _holder);
			m_node[_type].erase(m_node[_type].begin() + i);
			return true;
		}
	}
	return false;
}

BaseXMLNode* BaseSFuncXML::rootGet()
{
	if (!m_xml_p)
		return NULL;

	return m_xml_p->root();
}

BaseXMLNode* BaseSFuncXML::exportGet()
{
	if (!m_export)
	{
		BaseXMLNode node;

		PT_OAllocS(m_export, BaseXMLNode, node);
	}
	return m_export;
}

void BaseSFuncXML::titleAdd(const STLVString& _title_a)
{
	m_title_aa.push_back(_title_a);
}

void BaseSFuncXML::exportRelease()
{
	if(m_export)
		PT_OFreeS(m_export);
	m_export = NULL;
}

int BaseSFuncXML::propertyInfoExport_varF()
{
	const char* _filename = (const char*)paramVariableGet();
	if (!_filename)
		return 0;

	BaseFile file;
	file.set_asc_seperator("\t");

	STLString filename = _filename;
	filename += ".txt";

	if (file.OpenFile(filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return 0;

	for (int i = 0; i < m_propertyInfo_aa.size(); i++)
	{
		STLVInt& info_a = m_propertyInfo_aa[i];

		if (m_nameHash_m[info_a[0]].empty())
			continue;

		for (int j = 0; j < info_a.size(); j++)
		{
			STLString name = m_nameHash_m[info_a[j]];
			file.write_asc_string(name.c_str(), 0);
		}
		file.write_asc_line();
	}

	file.CloseFile();
	return 1;
}

void BaseSFuncXML::propertyInfo_add(STLVInt   _info_a)
{
	for (int i = 0; i < m_propertyInfo_aa.size(); i++)
	{
		if (m_propertyInfo_aa[i][0] == _info_a[0])
		{
			m_propertyInfo_aa[i] = _info_a;
			return;
		}
	}

	m_propertyInfo_aa.push_back(_info_a);
}

void BaseSFuncXML::propertyInfo_name_set(int _id, STLString _name)
{
	m_nameHash_m[_id] = _name;
}

bool BaseSFuncXML::propertyInfo_name_get(int _id, STLString* _name)
{
	STLMnString::iterator it;
	it = m_nameHash_m.find(_id);
	if (it == m_nameHash_m.end())
		return false;
	*_name = it->second;
	return true;
}

bool BaseSFuncXML::propertyInfo_remove(int _id, int _toid)
{
	for (int i = 0; i < m_propertyInfo_aa.size(); i++)
	{
		for (int j = 1; j < m_propertyInfo_aa[i].size(); j++)
		{
			if (m_propertyInfo_aa[i][j] == _id)
				m_propertyInfo_aa[i][j] = _toid;
		}

		if (m_propertyInfo_aa[i][0] == _id)
		{
			m_propertyInfo_aa.erase(m_propertyInfo_aa.begin() + i);
			i--;
		}
	}
	return true;
}

STLVInt BaseSFuncXML::propertyInfo_find(int _id)
{
	STLVInt ret;
	for (int i = 0; i < m_propertyInfo_aa.size(); i++)
	{
		for(int j=0; j<m_propertyInfo_aa[i].size(); j++)
		{
			if (m_propertyInfo_aa[i][j] == _id)
				return m_propertyInfo_aa[j];
		}
	}
	return ret;
}

bool BaseSFuncXML::propertyInfo_flip(int _id)
{
	for (int i = 0; i < m_propertyInfo_aa.size(); i++)
	{
		if (m_propertyInfo_aa[i][0] == _id)
		{
			STLVInt& info_a = m_propertyInfo_aa[i];
			int value = m_propertyInfo_aa[i][1];
			m_propertyInfo_aa[i][1] = m_propertyInfo_aa[i][2];
			m_propertyInfo_aa[i][2] = value;
			return true;
		}
	}

	return false;
}

bool BaseSFuncXML::propertyInfo_get(int _pipe_id, STLString* _from, STLString* _to, int* _fromid, int* _toid)
{
	for (int i = 0; i < m_propertyInfo_aa.size(); i++)
	{
		if (m_propertyInfo_aa[i][0] == _pipe_id)
		{
			STLVInt& info_a = m_propertyInfo_aa[i];
			bool find = false;
			*_fromid = info_a[1];
			*_toid = info_a[2];
			if (propertyInfo_name_get(info_a[1], _from))
				find = true;
			if (propertyInfo_name_get(info_a[2], _to))
				find = true;

			return find;
		}
	}
	return false;
}

int BaseSFuncXML::svgExport_varF()
{
	const int* hash = (const int*)m_param_value;
	const char* filename;
	if (!m_state_variable->get(*hash, (const void**)&filename))
		return 0;

	STLString filepath;
	filepath = filename;
	filepath = filepath.substr(0, filepath.find_last_of('.'));
	filepath += ".svg";

	BaseXMLNode &svg = *exportGet();
	svg.name = "svg";

	//int grid_size = (int)gap_af[0];
	//char grid_size_str[128];
	//sprintf_s(grid_size_str, "%d", grid_size);
	svg.block_b = true;
	//svg.set("width", grid_size_str);
	//svg.set("height", grid_size_str);
	svg.set("style", "background-color:white");
	svg.set("xmlns", "http://www.w3.org/2000/svg");
	svg.set("width", "23.375in");
	svg.set("height", "16.52778in");
	svg.set("pn_author", "cointcompany");
	svg.set("pn_version", 20210610);

	struct {
		bool operator()(const BaseXMLNode &_a , const BaseXMLNode &_b) const
		{
			return (_a.id < _b.id);
		}
	} compare;
	std::sort(svg.child_pa.begin(), svg.child_pa.end(), compare);

	BaseXML* arrow = (BaseXML*)m_resmanager_p->get_resource("pnid_arrow.svg", BaseXML::GetObjectId(), NULL, NULL, 0);
	svg.child_pa.insert(svg.child_pa.begin(), *arrow->root());

	STLString from, to;
	for (int i = 0; i < svg.child_pa.size(); i++)
	{
		BaseXMLNode* node = &svg.child_pa[i];
		int id = 0;
		if(node->int_get("pn_id", &id))
		{	
			int fromid, toid;
			if (propertyInfo_get(id, &from, &to, &fromid, &toid))
			{
				if (!from.empty())
				{
					node->set("pn_from", from);
					node->int_set("pn_fromid", fromid);
				}
				if (!to.empty())
				{
					node->set("pn_to", to);
					node->int_set("pn_toid", toid);
				}

			}
		}
	}

	STLString svgfile = svg.get(0);

	BaseFile file;
	if (file.OpenFile(filepath.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return 0;

	file.Write(svgfile.c_str(), (int)svgfile.size());
	file.CloseFile();
	return 1;
}

int BaseSFuncXML::paramEqual_astrIf()
{
	const char* str = (const char*)m_param_value;

	BaseFile parser;
	parser.set_asc_deletor(" ");
	if (parser.openParser((char*)str, (UINT32)strlen(str)))
		return 0;

	STLString name, param;
	parser.read_asc_line();
	name = parser.read_asc_str();
	param = parser.read_asc_str();

	if (name.empty() || param.empty())
		return 0;

	BaseXMLNode* root = m_xml_p->root()->node_get("svg");
	if(root->get(name.c_str()) != param)
		return 0;
	
	return 1;
}

int BaseSFuncXML::paramEqualBigger_astrIf()
{
	const char* str = (const char*)m_param_value;

	BaseFile parser;
	if (parser.OpenFile((void*)str, (UINT32)strlen(str)))
		return 0;

	STLString name;
	int param;
	parser.read_asc_line();
	name = parser.read_asc_str();
	if (!parser.read_asc_integer(&param))
		return 0;

	if (name.empty())
		return 0;

	BaseXMLNode* root = m_xml_p->root()->node_get("svg");
	int value = 0;
	if (!root->int_get(name.c_str(), &value))
		return 0;

	if (value > param)
		return 0;

	return 1;
}

int BaseSFuncXML::colorChange_nF()
{
	const char* set_col = (const char*)paramFallowGet(0); // set
	const char* from_col = (const char*)paramFallowGet(1); // from

	for (int i = 0; i < m_export->child_pa.size(); i++)
	{
		BaseXMLNode* node = &m_export->child_pa[i];
		
		STLString col_str;
		if (node->name == "text")
		{
			col_str = node->get("fill");
			if (!col_str.empty() && col_str == from_col)
				node->set("fill", set_col);
		}
		else {
			col_str = node->get("stroke");
			if (!col_str.empty() && col_str == from_col)
				node->set("stroke", set_col);
		}
	}

	return 1;
}
int BaseSFuncXML::svgExportSave_varF()
{
	const int* hash = (const int*)m_param_value;
	const char* filename;
	if (!m_state_variable->get(*hash, (const void**)&filename))
		return 0;

	if (!m_export)
		return 0;

	STLString filepath;
	filepath = filename;
	filepath = filepath.substr(0, filepath.find_last_of('.'));
	filepath += ".svg";

	BaseXMLNode& svg = *m_export;
	svg.name = "svg";

	svg.block_b = true;
	svg.set("style", "background-color:white");
	svg.set("xmlns", "http://www.w3.org/2000/svg");
	svg.set("width", "23.375in");
	svg.set("height", "16.52778in");
	svg.set("pn_author", "cointcompany");
	svg.set("pn_version", 20210610);

	struct {
		bool operator()(const BaseXMLNode& _a, const BaseXMLNode& _b) const
		{
			return (_a.id < _b.id);
		}
	} compare;
	std::sort(svg.child_pa.begin(), svg.child_pa.end(), compare);

	BaseXML* arrow = (BaseXML*)m_resmanager_p->get_resource("pnid_arrow.svg", BaseXML::GetObjectId(), NULL, NULL, 0);
	svg.child_pa.insert(svg.child_pa.begin(), *arrow->root());

	STLString svgfile = svg.get(0);

	BaseFile file;
	if (file.OpenFile(filepath.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return 0;

	file.Write(svgfile.c_str(), (int)svgfile.size());
	file.CloseFile();
	return 1;
}

STLVpSFunc& BaseSFuncXML::target_get()
{
	return m_target_a;
}

void BaseSFuncXML::target_add(BaseStateFunc * _node_p)
{
	m_target_a.push_back(_node_p);
}

//int BaseSFuncXML::dwgLoad_varF() // imprement in PnDwg::dwgLoad_varF
//{
//
//	return 1;
//}
int BaseSFuncXML::titleSave_varF()
{
	const char* path = (const char*)paramVariableGet();
	BaseFile file;
	file.set_asc_seperator("\t");
	file.OpenFile(path, BaseFile::OPEN_WRITE || BaseFile::OPEN_UTF8);
	
	file.write_asc_string("title", 0);
	file.write_asc_string("value", 0);
	for (int i = 0; i < 4; i++)
	{
		STLString title, value;
		title = "titleSub";
		title += ('0' + i);
		value = "valueSub";
		value += ('0' + i);
		file.write_asc_string(title.c_str(), 0);
		file.write_asc_string(value.c_str(), 0);
	}
	file.write_asc_line();
	for (int i = 0; i < m_title_aa.size(); i++)
	{
		for(int j = 0; j < m_title_aa[i].size(); j++)
		{
			BaseStringTable::trim(&m_title_aa[i][j]);
			file.write_asc_string(m_title_aa[i][j].c_str(), 0);
		}
		file.write_asc_line();
	}
	file.CloseFile();

	return 1;
}

bool node_option_c(const BaseXMLNode* _node, void* _data)
{
	STLVString& str_a = *(STLVString*)_data;

	if (str_a[1] == _node->get(str_a[0].c_str()))
		return true;

	return false;
}

bool node_option(BaseXMLNode* _node, void* _data)
{
	STLVString& str_a = *(STLVString*)_data;

	if (str_a[1] == _node->get(str_a[0].c_str()))
		return true;

	return false;
}

int BaseSFuncXML::layerExist_strIf()
{
	const char* layer = (const char*)m_param_value;
	BaseXMLNode* root;
	root = m_xml_p->root();

	STLVString str_a;
	str_a.push_back("pn_layer");
	str_a.push_back(layer);
	if (!root->find(&str_a, node_option_c))
		return 0;
	return 1;
}

int BaseSFuncXML::layerRemove_strF()
{
	const char* layer = (const char*)m_param_value;
	BaseXMLNode part;

	STLVString str_a;
	str_a.push_back("pn_layer");
	str_a.push_back(layer);

	BaseXMLNode* root_p = m_xml_p->root();
	do {
	} while (root_p->take(&str_a, node_option, &part));

	return 1;
}
int BaseSFuncXML::exportRemove_nF()
{
	exportRelease();
	return 1;
}
int BaseSFuncXML::svgFileMerge_varF()
{
	const char* _filepath = (const char*)paramVariableGet();
	if (!_filepath || strlen(_filepath) == 0)
		return 0;

	BaseXML *xml = (BaseXML*)m_resmanager_p->get_resource(_filepath, BaseXML::GetObjectId(), NULL, NULL, 0);

	if (!xml)
		return 0;

	BaseXMLNode* root_p = m_xml_p->root()->node_get("svg");

	BaseXMLNode* ext_p = xml->root()->node_get("svg");
	for (int i = 0; i < ext_p->child_pa.size(); i++)
	{
		BaseXMLNode* node = &ext_p->child_pa[i];
	
		if (node->name == "defs")
			continue;

		root_p->child_pa.push_back(*node);
	}

	return 1;
}

bool BaseSFuncXML::titleblockMakeTable(const char *_pathTitle, const char *_pathMatch, STLVString *_label_pa, STLVString *_value_pa)
{
	BaseResGrid* title = (BaseResGrid*)m_resmanager_p->get_resource(_pathTitle, BaseResGrid::GetObjectId(), NULL, NULL, 0);
	if (!title)
		return false;
	BaseResGrid* titleMatch = (BaseResGrid*)m_resmanager_p->get_resource(_pathMatch, BaseResGrid::GetObjectId(), NULL, NULL, 0);
	if (!titleMatch)
		return false;

	STLString label_first = titleMatch->label_get(0);
	for (int c = 0; c < titleMatch->column_count(); c++)
	{
		const char* title_name = titleMatch->column_get(0, c);
		int t = title->column_search(label_first.c_str(), title_name);
		if (t < 0)
			continue;

		for (int r = 0; r < title->label_count() / 2; r++)
		{
			title_name = titleMatch->column_get(r * 2 + 1, c);
			if (title_name && strlen(title_name) > 1) {
				const char* value = title->column_get(r * 2 + 1, t);
				_label_pa->push_back(title_name);
				_value_pa->push_back(value);
			}
		}
	}
	return true;
}

int BaseSFuncXML::title2dbtitle_nF()
{
	const char* pathTitle = (const char*)paramVariableGet();
	const char* pathMatch = (const char*)paramFallowGet(0);
	const char* ext = (const char*)paramFallowGet(1);

	if (!pathTitle || !pathMatch || !ext)
		return 0;
	
	STLVString stlVLabel_a, stlVValue_a;
	if (!titleblockMakeTable(pathTitle, pathMatch, &stlVLabel_a, &stlVValue_a))
		return 0;

	STLString filename, path;
	path = pathTitle;
	filename = path.substr(0, path.find('.'));
	filename += ext;
	BaseFile file;
	file.set_asc_seperator("\t");
	if (file.OpenFile((char*)filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return 0;

	for (int i = 0; i < stlVLabel_a.size(); i++)
		file.write_asc_string(stlVLabel_a[i].c_str(), 0);
	file.write_asc_line();
	for (int i = 0; i < stlVLabel_a.size(); i++)
		file.write_asc_string(stlVValue_a[i].c_str(), 0);;
	file.write_asc_line();
	file.CloseFile();

	return 1;
}

bool BaseSFuncXML::titleblockMakeGrid(const char* _pathTitle, const char* _pathMatch, BaseResGrid* _grid_p)
{
	BaseResGrid* titleDB = (BaseResGrid*)m_resmanager_p->reload_resource(_pathTitle, BaseResGrid::GetObjectId(), NULL, NULL, 0);
	if (!titleDB)
		return false;
	BaseResGrid* titleMatch = (BaseResGrid*)m_resmanager_p->get_resource(_pathMatch, BaseResGrid::GetObjectId(), NULL, NULL, 0);
	if (!titleMatch)
		return false;

	for (int i = 0; i < titleMatch->label_count(); i++)
		_grid_p->label_add(titleMatch->label_get(i).c_str());

	for (int c = 0; c < titleMatch->column_count(); c++)
	{
		STLVString value_a;
		for (int r = 0; r < titleMatch->label_count(); r++)
		{
			const char* value = titleMatch->column_get(r, c);
			if (!value || strlen(value) < 1)
			{
				value_a.push_back("");
				continue;
			}

			if (r % 2 == 0)
			{// it's title name
				value_a.push_back(value);
			}
			else {// it's value
				int label_n = titleDB->label_find(value);
				if (label_n >= 0)
					value_a.push_back(titleDB->column_get(label_n, 0));
				else
					value_a.push_back("");
			}
		}
		_grid_p->strings_add(value_a);
	}
	return true;
}

int BaseSFuncXML::dbtitle2title_nF()
{
	const char* pathDB = (const char*)paramVariableGet();
	const char* pathMatch = (const char*)paramFallowGet(0);
	const char* ext = (const char*)paramFallowGet(1);

	if (!pathDB || !pathMatch || !ext)
		return 0;

	BaseResGrid title;
	if (!titleblockMakeGrid(pathDB, pathMatch, &title))
		return 0;

	STLString path = pathDB;
	STLString filename = path.substr(0, path.find('.'));
	filename += ext;
	title.set_name(filename);
	if (!title.Save())
		return 0;

	m_resmanager_p->ClearResource(filename.c_str());
	return 1;
}

int BaseSFuncXML::title2json_nF()
{
	const char* pathTitle = (const char*)paramVariableGet();
	const char* pathMatch = (const char*)paramFallowGet(0);
	const char* ext = (const char*)paramFallowGet(1);

	if (!pathTitle || !pathMatch || !ext)
		return 0;

	STLVString stlVLabel_a, stlVValue_a;
	if (!titleblockMakeTable(pathTitle, pathMatch, &stlVLabel_a, &stlVValue_a))
		return 0;

	STLString filename, path;
	path = pathTitle;
	filename = path.substr(0, path.find('.'));
	filename += ext;
	BaseFile file;
	if (file.OpenFile((char*)filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return 0;

	STLString out = "{";
	for (int i = 0; i < stlVLabel_a.size(); i++)
	{
		if(i != 0)
			out += ",";
		out += "\"";
		out += stlVLabel_a[i];
		out += "\": ";
		out += "\"";
		out += stlVValue_a[i];
		out += "\"";
	}
	out += "}";
	file.write_asc_string(out.c_str(), 0);
	file.write_asc_line();
	file.CloseFile();

	return 1;
}
int BaseSFuncXML::json2title_nF()
{
	const char* pathDB = (const char*)paramVariableGet();
	const char* pathMatch = (const char*)paramFallowGet(0);
	const char* ext = (const char*)paramFallowGet(1);

	if (!pathDB || !pathMatch || !ext)
		return 0;

	BaseResGrid title;
	if (!titleblockMakeGrid(pathDB, pathMatch, &title))
		return 0;

	STLString path = pathDB;
	STLString filename = path.substr(0, path.find('.'));
	filename += ext;
	title.set_name(filename);
	if (!title.Save())
		return 0;

	m_resmanager_p->ClearResource(filename.c_str());
	return 1;
}
//#SF_functionInsert
