#pragma once

#include "BaseStateFunc.h"

class BaseXMLNode;

class BaseProjectedVector
{
public:
    bool set(int _w, int _h, int _gap, BaseXMLNode* _root);
    bool svgSave(STLString _filename) const;
    bool prjvecSave(STLString _filename) const;
    bool prjvecLoad(STLString _filename);
    float differ(const BaseProjectedVector* _prjVector) const;
    bool rotate(float _degree);
    void nameSet(STLString _name);
    void textSet(const STLString &_text);
    STLString nameGet() const { return m_name;  }

    BaseProjectedVector& operator=(const BaseProjectedVector& _prjVec);
    float operator-(const BaseProjectedVector& _prjVec) const;
    float scaleWidth() const { return m_wscale; }
    float scaleHeight() const { return m_hscale; }
    const STLVVec3& vectorGet() const { return m_prjVec_av3; }
    void vectorSet(const STLVVec3& _vec_a);

    friend BaseProjectedVector;
    int& index() { return m_index;  }
    float leaningGet() const;
protected:
    int m_index;
    int m_width, m_height, m_gap;
    float m_wstep, m_hstep;
    float m_wscale, m_hscale;
    STLString m_text; // text in the file
    STLVVec3 m_prjVec_av3;
    STLString m_name;
};

typedef std::vector<BaseProjectedVector, PT_allocator<BaseProjectedVector> > STLVProjectedVector;

class BaseRecognitionPrjVector :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(typeCheck_nIf),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    BaseRecognitionPrjVector();
    ~BaseRecognitionPrjVector();

    PtObjectHeaderInheritance(BaseRecognitionPrjVector);
    STDEF_SC_HEADER(BaseRecognitionPrjVector);

    virtual int Create();
    //#SF_FuncHeaderInsert
    // User defin area from here
protected:

public:

protected:
};
