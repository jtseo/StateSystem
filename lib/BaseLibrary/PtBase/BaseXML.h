#pragma once

#pragma once
#include "BaseResResource.h"
#include "BaseFile.h"

#define XMLREAD(x)	x = _file->read_asc_str(); if(x.empty()) return false
#define XMLREADQ(x)	x = _file->read_asc_quotation(); if(x.empty()) return false
#define XMLCheck(x)	if (_file->get_seperator_last() != x) return false
#define XMLWTAB(x) for(int k=0; k<(x); k++) format += "\t"

class PtVector3;
class PtMatrix4;
class PtBound;

class BaseXMLOption
{
public:
	STLString name;
	STLString value;

	bool set(BaseFile* _file);
	STLString get();
};

typedef std::vector<BaseXMLOption, PT_allocator<BaseXMLOption> >	STLVxmlOption;

class BaseXMLParam
{
public:
	STLString name;
	STLString value;
	STLVxmlOption	option_a;
	typedef enum en_style {
		Single,
		Multi
	} EnStyle;
	EnStyle style;

	static STLSInt s_param_multi_s;

	bool set(BaseFile* _file);
	void set(STLString _name, STLString _option);
	void float_set(STLString _name, float _value);
	void color_set(STLString _name, const PtVector3 &_color);
	STLString get();
};

typedef std::vector<BaseXMLParam, PT_allocator<BaseXMLParam> >	STLVxmlParam;

class BaseXMLNode
{
public:
	unsigned id;
	STLString name;
	STLString text;
	STLString text_tos;
	STLVxmlParam	param_a;
	bool	block_b; // it has end tag, if not it should be ended by '/'
	std::vector<BaseXMLNode, PT_allocator<BaseXMLNode> >	child_pa;
	bool m_end;

	BaseXMLNode& operator=(const BaseXMLNode& _node);
	BaseXMLNode* node_get(const char* _name);
	bool text_set(const STLString &_text);
	STLString text_get() const;
	bool int_get(const char* _name, int* _value) const;
	void int_set(const char* _name, int _value);
	bool vector_get(const char* _name, PtVector3* _pv3);
	void vector_set(const char* _name, const PtVector3& _v3);
	float get_float(const char* _name, const char* _option = NULL);
	STLString get(const char* _name, const char* _option = NULL) const;
	bool set(STLString  _name, STLString  _option, STLString  _value);
	bool set(const char* _name, const char* _value);
	bool set(const char* _name, int _value);
	bool set(STLString _name, STLString _value);
	bool set(BaseFile* _file);
	bool fontsize_get(const char* _name, float* _value);
	void fontsize_set(const char* _name, float _value);
	bool float_get(const char* _name, float* _value);
	void float_set(const char* _name, float _value);
	bool color_get(const char* _name, PtVector3 *_color);
	void color_set(const char* _name, const PtVector3& _color);
	STLString read_tag(BaseFile* _file);
	STLString get(int _tab);

	typedef bool (*fnCallBack)(BaseXMLNode *_node, void *_data);
	typedef bool (*fnConstCallBack)(const BaseXMLNode* _node, void* _data);

	static STLVVec3 vertex_get(STLString& _str, STLVInt* _seperator = NULL);
	STLVVec3 vertex_get(bool _include_z = false) const;
	bool vertex_set(STLVVec3& _vec_a, STLVInt& _seperator);
	bool vertex_set(STLVVec3& _vec_a, char _type, bool _closed = false);
	bool vertexPoly_set(STLVVec3& _ver_a);
	bool vertexCurve_set(STLVVec3& _ver_a);
	bool transform_remove(PtMatrix4 _m4);
	bool move(PtVector3 _v3);
	bool scale(PtVector3 _scale);
	bool transform(const PtMatrix4& _m4);
	bool bound_get(PtMatrix4* _m4, PtBound* _bnd) const;
	bool bound_child_get(PtMatrix4* _m4, PtBound* _bnd, STLString _name, int _value) const;
	const BaseXMLNode *find(void* _data, fnConstCallBack _func) const;
	bool take(void* _data, fnCallBack _func, BaseXMLNode *_node);

	bool polygonCollision(BaseXMLNode* _node, float _minimumAngle);
	bool lineCollision(BaseXMLNode* _node, float _maxDist);
	bool lineCollision(const STLVVec3& _vec_a, float _maxDist);
	bool limitGet(const PtVector3& _dir, PtVector3* _limit) const;
	float lineCoverRate(PtBound& _bnd, float *_total_p);
protected:
	STLVVec3	m_vertex_a;
};

typedef std::vector<BaseXMLNode, PT_allocator<BaseXMLNode> > STLVXMLNode;

class BaseXML :
	public BaseResResource
{
public:
	BaseXML();
	~BaseXML();

	PtObjectHeader(BaseXML);
	static int GetObjectId();

	virtual bool Load(); // Load In Thread
	virtual void Clear();
	bool Save();

	int read(int _nPos, bbyte* _data, int _nSize);
	BaseXMLNode* root() {
		if (m_root_a.size() > 0)
			return &m_root_a[0];
		return NULL;
	}

	bool set(STLString& _stream);
	void root_set(BaseXMLNode* _root);
	STLString text_get();
	STLString last;

	static bool fnMaxID(BaseXMLNode* _node, void* _data);
protected:

	bool set(BaseFile* _file);
	STLVXMLNode	m_root_a;
	BaseFile* m_pFile;
};

