#pragma once


#include "BaseStateFunc.h"
#include "BaseRecognitionPrjVector.h"

class BaseState;
class BaseXML;
class BaseXMLNode;
//class PnDwg;
class BaseResGrid;
typedef std::vector<BaseXMLNode*, PT_allocator<BaseXMLNode*> > STLVpXMLNode;

class BaseSFuncXML :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        STDEF_SFENUM(textRemove_strF),
        STDEF_SFENUM(svgSave_varF),
        STDEF_SFENUM(fileIs_nIf),
        STDEF_SFENUM(svgAIMakeTrainDataCast_nF),
        STDEF_SFENUM(objectAllCast_nF),
        //STDEF_SFENUM(dwgSave_varF),
        STDEF_SFENUM(propertyInfoExport_varF),
        STDEF_SFENUM(svgExport_varF),
        STDEF_SFENUM(paramEqual_astrIf),
        STDEF_SFENUM(paramEqualBigger_astrIf),
        STDEF_SFENUM(colorChange_nF),
        STDEF_SFENUM(svgExportSave_varF),
        //STDEF_SFENUM(dwgConvert_varF),
        //STDEF_SFENUM(dwgLoad_varF),
        STDEF_SFENUM(titleSave_varF),
        STDEF_SFENUM(layerExist_strIf),
        STDEF_SFENUM(layerRemove_strF),
        STDEF_SFENUM(exportRemove_nF),
        STDEF_SFENUM(svgFileMerge_varF),
        STDEF_SFENUM(title2dbtitle_nF),
        STDEF_SFENUM(dbtitle2title_nF),
        STDEF_SFENUM(title2json_nF),
        STDEF_SFENUM(json2title_nF),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    enum NodeType {
        TYPE_UNKNOW,
        TYPE_LINE,
        TYPE_TEXT,
        TYPE_POLYGON,
        TYPE_MAX
    };

public:
    BaseSFuncXML();
    ~BaseSFuncXML();

    PtObjectHeaderInheritance(BaseSFuncXML);
    STDEF_SC_HEADER(BaseSFuncXML);

    virtual int Create();
    int textRemove_strF();
    int svgSave_varF();
    int fileIs_nIf();
    int svgAIMakeTrainDataCast_nF();
    int objectAllCast_nF();
    int prjectedVectorChartSave(STLString filename);
    //int dwgSave_varF();
    int propertyInfoExport_varF();
    int svgExport_varF();
    int paramEqual_astrIf();
    int paramEqualBigger_astrIf();
    int colorChange_nF();
    int svgExportSave_varF();
    //int dwgConvert_varF();
    //int dwgLoad_varF();
    int titleSave_varF();
    int layerExist_strIf();
    int layerRemove_strF();
    int exportRemove_nF();
    int svgFileMerge_varF();
    int title2dbtitle_nF();
    int dbtitle2title_nF();
    int title2json_nF();
    int json2title_nF();
    //#SF_FuncHeaderInsert

    // User defin area from here
    bool titleblockMakeTable(const char* _pathTitle, const char* _pathMatch, STLVString* _label_pa, STLVString* _value_pa);
    bool titleblockMakeGrid(const char* _pathTitle, const char* _pathMatch, BaseResGrid *_grid_p);
    static NodeType nodeType(BaseXMLNode& _node);
    void nodeAdd(NodeType _type, BaseStateFunc* _node);
    STLVpSFunc& nodeGet(NodeType _type);
    BaseStateFunc* nodeGet(NodeType _type, int _id);
    bool nodeErase(NodeType _type, BaseStateFunc* _node, int _holder);
    bool nodeHiden(NodeType _type, BaseStateFunc* _node, int _holder);
    bool nodeShowing(int _id);
    const STLVpVoid& nodeHidenGet();
    BaseStateFunc* nodeHidenGet(int _id);
    BaseXMLNode* exportGet();
    void exportRelease();
    BaseXMLNode* rootGet();
    bool svgSave(BaseXMLNode* _svg, const STLString &_filename);
    bool boundRemove(PtBound _bnd);
protected:
    BaseXML *m_xml_p;
    BaseXMLNode *m_export;
    STLVpVoid m_hidenList_a;

    STLVpSFunc m_node[TYPE_MAX];
    STLVProjectedVector m_prjVec_a;

    // support for analysis
public:
    STLVpSFunc& target_get();
    void target_add(BaseStateFunc* _node_p);
protected:
    STLVpSFunc    m_target_a;

    // support for property information
public:
    void propertyInfo_add(STLVInt   _info_a);
    void propertyInfo_name_set(int _id, STLString _name);
    bool propertyInfo_get(int _pipe_id, STLString *_from, STLString *_to, int *_fromid, int *_toid);
    bool propertyInfo_name_get(int _id, STLString *_name);
    bool propertyInfo_remove(int _id, int _toid);
    bool propertyInfo_flip(int _id);
    STLVInt propertyInfo_find(int _id);
    const STLVStlVInt& propertyInfo_get() { return m_propertyInfo_aa;  }

protected:
    STLVStlVInt m_propertyInfo_aa;
    STLMnString m_nameHash_m;

    // support DWG file export
    //PnDwg* m_dwg_p;

    // support titleblock
public:
    void titleAdd(const STLVString& _title_a);
protected:
    STLVstlVString  m_title_aa;
};

