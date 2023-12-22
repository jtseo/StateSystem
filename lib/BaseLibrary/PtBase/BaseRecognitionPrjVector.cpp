#include "stdafx.h"

#include "../PtBase/BaseStateFunc.h"
#include "../PtBase/BaseObject.h"
#include "../PtBase/BaseState.h"
#include "../PtBase/BaseFile.h"

#include "BaseRecognitionPrjVector.h"

#include "../PtBase/BaseStateMain.h"
#include "../PtBase/BaseStateManager.h"
#include "../PtBase/BaseStateSpace.h"

#include "../PtBase/BaseResFilterIP.h"
#include "../PtBase/BaseResManager.h"
#include "../PtBase/BaseTime.h"
#include "../PtBase/BaseStringTable.h"

#include "BaseXML.h"

PtObjectCpp(BaseRecognitionPrjVector);

// define  BaseStateFunc::FuncSample
// grouping by class 
// and call representator class FunctionProcessor with _processor hash

STLVInt	BaseRecognitionPrjVector::s_func_hash_a;
const char* BaseRecognitionPrjVector::s_class_name = "BaseRecognitionPrjVector";

int BaseRecognitionPrjVector::GetObjectId()
{
	static int	s_iId = 0;
	if (s_iId == 0) {
		s_iId = STRTOHASH(s_class_name);
	}

	return s_iId;
}

BaseRecognitionPrjVector::BaseRecognitionPrjVector()
{
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseRecognitionPrjVector::FunctionProcessor);
}

int BaseRecognitionPrjVector::StateFuncRegist(STLString _class_name, STLVInt* _func_hash, int _size, fnEventProcessor _func)
{
	if (BaseStateFunc::StateFuncRegist(_class_name, _func_hash, _size, _func) > 0)
	{
		STLString func_str;
		//#SF_FuncRegistStart
		//STDEF_SFREGIST(text_info_cast_nF);
		// 
		//#SF_FuncRegistInsert

		return _size;
	}
	else
		return 0;
}

BaseRecognitionPrjVector::~BaseRecognitionPrjVector()
{

}

void BaseRecognitionPrjVector::init()
{
	BaseStateFunc::init();
	m_nObjectId = GetObjectId();

	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseRecognitionPrjVector::FunctionProcessor);
}

void BaseRecognitionPrjVector::release()
{
}

BaseStateFuncEx* BaseRecognitionPrjVector::CreatorCallback(const void* _param)
{
	BaseRecognitionPrjVector* bs_func;
	PT_OAlloc(bs_func, BaseRecognitionPrjVector);

	bs_func->init();
	StateFuncRegist(s_class_name, &s_func_hash_a, EnumExtentionMax, BaseRecognitionPrjVector::FunctionProcessor);

	return bs_func;
}

int BaseRecognitionPrjVector::FunctionCall(const char* _class_name, STLVInt& _func_hash)
{
	int ret = BaseStateFunc::FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseRecognitionPrjVector::FunctionProcessor(const BaseDStructureValue* _base, BaseDStructureValue* _event, BaseDStructureValue* _context, int _status)
{
	int _processor = _base->function_hash_get();

	BaseRecognitionPrjVector* bs_func = (BaseRecognitionPrjVector*)BaseStateFunc::FunctionProcessorHeader(_processor, s_class_name, sm_sample.GetObjectId(), _base, _event, _context, &s_func_hash_a, BaseRecognitionPrjVector::CreatorCallback, 0);

	int ret = bs_func->FunctionCall(s_class_name, s_func_hash_a);

	return ret;
}

int BaseRecognitionPrjVector::Create()
{
	if (!BaseStateFunc::Create())
		return 0;

	//GroupLevelSet(2); // 1 should be BaseSFuncXML

	//GroupIdSet(m_node->id);

	return 1;
}

void BaseProjectedVector::textSet(const STLString& _text)
{
	m_text = _text;
}

void BaseProjectedVector::nameSet(STLString _name)
{
	m_name = _name;
}

float BaseProjectedVector::differ(const BaseProjectedVector* _prjVector) const
{
	float differ = 0;
	PtVector3 vec;

	const STLVVec3& vec_a = _prjVector->vectorGet();
	float cnt = 0;
	for (int i = 0; i < m_prjVec_av3.size(); i++)
	{	
		float dot = m_prjVec_av3[i].Dot(vec_a[i]);

		if (m_prjVec_av3[i].DoubleLength() == 0
			&& vec_a[i].DoubleLength() == 0)
		{
			cnt++;
		}else if (dot > -0.98f)
		{
			dot -= 1;
			dot *= -1;
			differ += dot/2.f;
			cnt++;
		}
	}

	if (!m_text.empty())
	{
		if (BaseStringTable::find(_prjVector->m_text, m_text) == -1)
			differ += (float)cnt/10.f;
		else
			differ -= differ / 10.f;
	}

	float lean1 = leaningGet(), lean2 = _prjVector->leaningGet();

	lean1 -= lean2;
	if (lean1 < 0)
		lean1 *= -1;

	differ += lean1*(float)cnt / 10.f; // rate difference will increase different percent.

	differ /= cnt;
	differ *= 100;
	return differ;
}

float BaseProjectedVector::leaningGet() const
{
	if (m_wscale > m_hscale)
		return m_wscale / m_hscale;
	return m_hscale / m_wscale;
}

bool BaseProjectedVector::rotate(float _degree)
{
	STLVVec3 rot_av3;
	rot_av3.resize(m_height * m_width);
	
	PtMatrix4 mat;
	PtVector3 axi(0, 0, -1);
	mat.MakeIdentity();
	mat.SetRotate(axi, PM_HALF);

	for(int h=0; h < m_height; h++)
		for (int w = 0; w < m_width; w++)
		{
			int idx = h * m_width + w;
			PtVector3 v3 = m_prjVec_av3[idx];
			v3 *= mat;
			int tidx = (m_height - w - 1) * m_width + h;
			rot_av3[tidx] = v3;
		}

	m_prjVec_av3 = rot_av3;
	return true;
}

bool BaseProjectedVector::set(int _w, int _h, int _gap, BaseXMLNode* _root)
{
	m_width = _w;
	m_height = _h;
	m_gap = _gap;

	PtBound bnd;
	PtMatrix4 m4;
	m4.MakeIdentity();

	for (int i = 0; i < _root->child_pa.size(); i++)
	{ // check white rect has no back polygon, remove it from the projection vector target.
		BaseXMLNode* node = &_root->child_pa[i];
		if (node->get("fill") != "#FFFFFF")
			continue;

		bool col = false;
		if (!node->bound_get(&m4, &bnd))
			continue;

		bnd.extend(-0.1f);

		for (int j = 0; j < _root->child_pa.size(); j++)
		{
			if (i == j)
				continue;

			node = &_root->child_pa[j];
			PtBound bnd2;
			if (!node->bound_get(&m4, &bnd2))
				continue;

			if (bnd.collision(bnd2))
			{
				col = true;
				break;
			}
		}

		if (!col)
		{
			_root->child_pa.erase(_root->child_pa.begin() + i); // remove object what has no back polygon.
			i--;
		}
	}

	if (!_root->bound_get(&m4, &bnd))
		return false;

	if (bnd.size() == 0)
		return false;

	PtVector3 org_v3;
	org_v3 = bnd[0] * -1;

	_root->move(org_v3);
	org_v3.x = (float)(_w - 2) / (bnd[1].x - bnd[0].x);
	org_v3.y = (float)(_h - 2) / (bnd[1].y - bnd[0].y);
	org_v3.z = 0;
	m_wscale = org_v3.x;
	m_hscale = org_v3.y;
	_root->scale(org_v3);
	org_v3.SetValue(1, 1, 0);
	_root->move(org_v3);

	int len = _w * _h;
	m_prjVec_av3.resize(len);
	for (int x = 0; x < _w; x++)
		for (int y = 0; y < _h; y++)
		{
			int index = x + _w * y;
			m_prjVec_av3[index].SetInit();
			m_prjVec_av3[index].z = -1; // mean first is -1
		}

	m_wstep = 1;
	m_hstep = 1;

	float gap = _gap * (m_wstep + m_hstep) / 2.f;
	PtVector3 col(0, 0, 0);

	for (int j = 0; j < _root->child_pa.size(); j++)
	{
		const BaseXMLNode* node = &_root->child_pa[j];
		STLVVec3 vec_a = node->vertex_get(true);

		if (node->name == "text")
		{
			m_text = node->text_get();
			char buf[255];
			strcpy_s(buf, 255, m_text.c_str());
			BaseStringTable::cut_str(' ', buf);
			m_text = buf;
		}

		for (int i = 0; i + 1 < vec_a.size(); i++)
			for (int x = 0; x < _w; x++)
				for (int y = 0; y < _h; y++)
				{
					int index = x + _w * y;
					PtVector3 pos((float)x * m_wstep, (float)y * m_hstep, 0), v3;
					float dist;
					v3 = m_prjVec_av3[index];

					dist = pos.line_distance(&vec_a[i], &col);
					if (//dist <= gap &&
						(v3.z == -1 || // mean first collision
							dist < v3.z))
					{
						m_prjVec_av3[index] = col - pos;
						m_prjVec_av3[index].z = 0;
						m_prjVec_av3[index].MakeNormal();
						m_prjVec_av3[index].z = dist;
					}
				}
	}

	for (int j = 0; j < _root->child_pa.size(); j++)
	{
		const BaseXMLNode* node = &_root->child_pa[j];
		STLVVec3 vec_a = node->vertex_get(true);

		if (vec_a.empty() || vec_a[0] != vec_a.back())
			continue;

		if (node->get("fill") != "#000000")
			continue;
		// vertex what are filled by black, reset value.
		
		for (int x = 0; x < _w; x++)
			for (int y = 0; y < _h; y++)
			{
				int index = x + _w * y;
				PtVector3 pos((float)x * m_wstep, (float)y * m_hstep, 0), v3;
				v3 = m_prjVec_av3[index];

				if (x == 2 && y == 6)
				{
					int k = 0;
					k++;
				}
				if (PtPolygon::inCheck(pos, vec_a))
					m_prjVec_av3[index].SetInit();
			}
	}

	for (int x = 0; x < _w; x++)
		for (int y = 0; y < _h; y++)
		{
			int index = x + _w * y;
			m_prjVec_av3[index].z = 0;
		}

	return true;
}

const char *latestVer_str = "ProjectVector Ver: 2021052602";

bool BaseProjectedVector::prjvecLoad(STLString _filename)
{
	BaseFile file;
	if (file.OpenFile(_filename.c_str(), BaseFile::OPEN_READ | BaseFile::OPEN_UTF8))
		return false;

	file.read_asc_line();
	STLString ver = file.read_asc_str(); //  ("ProjectVector Ver: 20210526", 0);
	if(ver != latestVer_str)
		g_SendMessage(LOG_MSG_POPUP, "prjvec Version conflict '%s' : %s", _filename.c_str(), ver.c_str());

	file.read_asc_line();
	m_name = file.read_asc_str();
	m_text = file.read_asc_str();
	file.read_asc_line();
	file.read_asc_integer(&m_width);
	file.read_asc_integer(&m_height);
	file.read_asc_integer(&m_gap);
	file.read_asc_float(&m_wstep);
	file.read_asc_float(&m_hstep);
	file.read_asc_float(&m_wscale);
	file.read_asc_float(&m_hscale);
	file.read_asc_integer(&m_index);

	PtVector3 vec;
	vec.SetInit();
	m_prjVec_av3.clear();
	while(file.read_asc_line()){
		file.read_asc_float(&vec.x);
		file.read_asc_float(&vec.y);
		m_prjVec_av3.push_back(vec);
	}

	file.CloseFile();

	return true;
}

bool BaseProjectedVector::prjvecSave(STLString _filename) const
{
	STLString filename = _filename.substr(0, _filename.find_last_of('.'));

	filename += ".prjvec";
	BaseFile file;
	if (file.OpenFile(filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return false;

	file.write_asc_string(latestVer_str, 0);
	file.write_asc_line();

	file.write_asc_string(m_name.c_str(), 0);
	file.write_asc_string(m_text.c_str(), 0);
	file.write_asc_line();

	file.write_asc_integer(m_width);
	file.write_asc_integer(m_height);
	file.write_asc_integer(m_gap);
	file.write_asc_float(m_wstep);
	file.write_asc_float(m_hstep);
	file.write_asc_float(m_wscale);
	file.write_asc_float(m_hscale);
	file.write_asc_integer(m_index);

	file.write_asc_line();

	for (int i = 0; i < m_prjVec_av3.size(); i++)
	{
		file.write_asc_float(m_prjVec_av3[i].x);
		file.write_asc_float(m_prjVec_av3[i].y);
		file.write_asc_line();
	}

	file.CloseFile();

	return true;
}

bool BaseProjectedVector::svgSave(STLString _filename) const
{
	STLString filename = _filename.substr(0, _filename.find_last_of('.'));

	BaseXMLNode svg;

	char grid_size_str[128];
	sprintf_s(grid_size_str, 128, "%d", m_width * 10);

	svg.name = "svg";
	svg.block_b = true;
	svg.set("width", grid_size_str);
	svg.set("height", grid_size_str);
	svg.set("viewBox", "");
	svg.set("style", "background-color:white");
	svg.set("xmlns", "http://www.w3.org/2000/svg");

	BaseXMLNode node;
	node.name = "path";
	node.set("stroke", "black");

	char buf[255];
	svg.child_pa.clear();

	for (int x = 0; x < m_width; x++)
		for (int y = 0; y < m_height; y++)
		{
			int index = x + m_width * y;
			PtVector3 pos((float)x * m_wstep * 10.f, (float)y * m_hstep * 10.f, 0.f), pos2;
			pos2 = pos + m_prjVec_av3[index] * 10;
			sprintf_s(buf, 255, "M%.3f %.3f %.3f %.3f", pos.x, pos.y, pos2.x, pos2.y);
			node.set("d", buf);

			svg.child_pa.push_back(node);
		}

	STLString svgfile = svg.get(0);
	filename += "PRJ.svg";
	BaseFile file;
	if (file.OpenFile(filename.c_str(), BaseFile::OPEN_WRITE | BaseFile::OPEN_UTF8))
		return false;

	file.Write(svgfile.c_str(), (int)svgfile.size());
	file.CloseFile();

	return true;
}

void BaseProjectedVector::vectorSet(const STLVVec3& _vec_a)
{
	m_prjVec_av3 = _vec_a;
}

BaseProjectedVector& BaseProjectedVector::operator=(const BaseProjectedVector& _prjVec)
{
	m_index = _prjVec.m_index;
	m_width = _prjVec.m_width;
	m_height = _prjVec.m_height;
	m_gap = _prjVec.m_gap;
	m_wscale = _prjVec.m_wscale;
	m_hscale = _prjVec.m_hscale;
	m_name = _prjVec.m_name;
	m_prjVec_av3 = _prjVec.m_prjVec_av3;
	m_hstep = _prjVec.m_hstep;
	m_wstep = _prjVec.m_wstep;
	return *this;
}

float BaseProjectedVector::operator-(const BaseProjectedVector& _prjVec) const
{
	return differ(&_prjVec);
}
