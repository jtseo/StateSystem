#pragma once

#include "BaseStateFunc.h"

class BaseSFuncXML;
class BaseProjectedVector;

typedef std::vector<BaseProjectedVector*, PT_allocator<BaseProjectedVector*> > STLVpProjectedVector;

class BasePrjVectorGroup
{
public:
    BasePrjVectorGroup();
    void release();

    int load(STLString _name, int _index);
    int add(BaseProjectedVector* _prjVec_p, int _index);
    float differ(const BaseProjectedVector* _prjVec_p) const;
    float differ_min(const BaseProjectedVector* _prjVec_p, int *_index_p = NULL) const;
    bool predic(STLVInt* _index2_a, STLVfloat* _diff_a, const BaseProjectedVector* _prjVec_p);

    float &margin() { return m_groupMargin;  }
    int averageRemake();
    bool averageGet(BaseProjectedVector* _prjVec_p) const;
    int indexGet(int _seq);
protected:
    float               m_groupMargin;
    STLVVec3    m_avarge_a;
    STLVpProjectedVector    m_prjVec_ap;
};

typedef std::vector<BasePrjVectorGroup, PT_allocator<BasePrjVectorGroup> > STLVPrjVectorGroup;

class BasePrjVectorName
{
public:
    int load(STLString name);
    int add(BaseProjectedVector* _prjVec_p);
    int _add(BaseProjectedVector* _prjVec_p);
    bool predic(STLVInt *_index_a, STLVInt *_index2_a, STLVfloat *_diff_a, const BaseProjectedVector *_prjVec_p, float _margin);

    STLString& name() { return m_name; }
    STLString& layer() { return m_layer; }
    STLString& filename() { return m_filename; }
    const STLVPrjVectorGroup &groupGet() { return m_prjVecGroup_a;  }
    float& groupMargin() { return m_groupMargin;  }
protected:
    float       m_groupMargin;
    STLString   m_name;
    STLString   m_layer;
    STLString   m_filename;
    STLVPrjVectorGroup  m_prjVecGroup_a;
};

typedef std::vector<BasePrjVectorName, PT_allocator<BasePrjVectorName> > STLVPrjVecName;

class BaseRecognitionManager :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(typeCheck_nIf),
        STDEF_SFENUM(add_nF),
        STDEF_SFENUM(predict_nF),
        STDEF_SFENUM(crossCheck_nF),
        STDEF_SFENUM(load_nF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseRecognitionManager();
    ~BaseRecognitionManager();

    PtObjectHeaderInheritance(BaseRecognitionManager);
    STDEF_SC_HEADER(BaseRecognitionManager);

    virtual int Create();
    int add_nF();
    int predict_nF();
    int crossCheck_nF();
    int load_nF();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    
public:
    
protected:
    STLVPrjVecName  m_prjVecName_a;
 };
