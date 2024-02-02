#pragma once
#include "../PtBase/BaseStateFunc.h"
#include "SerialPort.h"

class DevCashReader :
    public BaseStateFunc
{
    enum {
        //#SF_EnumStart
        Enum_ext_start = EnumFuncMax,
        //STDEF_SFENUM(Open_varF),
        STDEF_SFENUM(DataARead_nF),
        STDEF_SFENUM(DataAShift_nF),
        STDEF_SFENUM(DataComplete_nIf),
        STDEF_SFENUM(DataGet_varF),
        STDEF_SFENUM(DataWriteSet_varF),
        STDEF_SFENUM(ConfigWrite_nF),
        STDEF_SFENUM(CashReadStart_nF),
        STDEF_SFENUM(MoneyGet_varIf),
        STDEF_SFENUM(CashReadStop_nF),
        STDEF_SFENUM(DataFullCheck_nIf),
        STDEF_SFENUM(DataReset_nF),
        STDEF_SFENUM(PortInit_nIf),
        //#SF_EnumInsert
        EnumExtentionMax
    };

public:
    DevCashReader();
    ~DevCashReader();

    PtObjectHeaderInheritance(DevCashReader);
    STDEF_SC_HEADER(DevCashReader);

    virtual int Create();
    int DataARead_nF();
    int DataAShift_nF();
    int DataComplete_nIf();
    int DataGet_varF();
    int DataWriteSet_varF();
    int ConfigWrite_nF();
    int CashReadStart_nF();
    int MoneyGet_varIf();
    int CashReadStop_nF();
    int DataFullCheck_nIf();
    int DataReset_nF();
    int PortInit_nIf();
    //#SF_FuncHeaderInsert

    // User defin area from here
protected:
    STLVcChar m_datas;
    CSerialPort m_serial;
    char m_packet[5];
    bool PacketMake(const char* _str, char* _packet);
    // support for text analysis
public:
protected:
};

