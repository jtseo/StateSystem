#pragma once

#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

class CSerialPort
{
public:
    CSerialPort(void);
    virtual ~CSerialPort(void);

private:
    HANDLE          m_hComm;
    DCB             m_dcb;
    COMMTIMEOUTS    m_CommTimeouts;
    BOOL            m_bPortReady;
    BOOL            m_bWriteRC;
    BOOL            m_bReadRC;
    DWORD           m_iBytesWritten;
    DWORD           m_iBytesRead;
    DWORD           m_dwBytesRead;

public:
    bool OpenPort(std::string portname);
    void ClosePort();
    bool ReadByte(BYTE& resp);
    bool ReadByte(char *resp, UINT size);
    bool WriteByte(BYTE bybyte);
    bool WriteBytes(const char *_data, int size);
    bool SetCommunicationTimeouts(DWORD ReadIntervalTimeout, DWORD ReadTotalTimeoutMultiplier, DWORD ReadTotalTimeoutConstant, DWORD WriteTotalTimeoutMultiplier, DWORD WriteTotalTimeoutConstant);
    bool Configure_Port(unsigned int BaudRate, unsigned char ByteSize, unsigned int fParity, unsigned char Parity, unsigned char StopBits);

    void Dispay(const char* _pre, const char* _data);
};