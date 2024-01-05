#include "SerialPort.h"
#include <iostream>

#include "../PtBase/base.h"

using namespace std;

// *******************************************
// Class Constructor
// *******************************************
CSerialPort::CSerialPort()
{
    //
}


// *******************************************
// Class Destructor
// *******************************************
CSerialPort::~CSerialPort()
{
    //
}


// *******************************************
// Open Port
// *******************************************
bool CSerialPort::OpenPort(std::string portname)
{
    std::string port = "//./";
    port += portname;
    m_hComm = CreateFile((const char*)port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

    if (m_hComm == INVALID_HANDLE_VALUE)
        return false;
    else
        return true;
}


// *******************************************
// Configure Port
// *******************************************
bool CSerialPort::Configure_Port(unsigned int BaudRate, unsigned char ByteSize, unsigned int fParity, unsigned char Parity, unsigned char StopBits)
{
    if ((m_bPortReady = GetCommState(m_hComm, &m_dcb)) == 0)
    {
        ::OutputDebugString((const char*)"GetCommState Error");
        CloseHandle(m_hComm);

        return false;
    }

    m_dcb.BaudRate = BaudRate;
    m_dcb.ByteSize = ByteSize;
    m_dcb.Parity = Parity;
    m_dcb.StopBits = StopBits;
    m_dcb.fBinary = true;
    m_dcb.fDsrSensitivity = false;
    m_dcb.fParity = fParity;
    m_dcb.fOutX = false;
    m_dcb.fInX = false;
    m_dcb.fNull = false;
    m_dcb.fAbortOnError = true;
    m_dcb.fOutxCtsFlow = false;
    m_dcb.fOutxDsrFlow = false;
    m_dcb.fDtrControl = DTR_CONTROL_DISABLE;
    m_dcb.fDsrSensitivity = false;
    m_dcb.fRtsControl = RTS_CONTROL_DISABLE;
    m_dcb.fOutxCtsFlow = false;
    m_dcb.fOutxCtsFlow = false;

    m_bPortReady = SetCommState(m_hComm, &m_dcb);

    if (m_bPortReady == 0)
    {
        ::OutputDebugString("SetCommState Error");
        CloseHandle(m_hComm);

        return false;
    }

    return true;
}


// *******************************************
// Set Communication Timeouts
// *******************************************
bool CSerialPort::SetCommunicationTimeouts(DWORD ReadIntervalTimeout,
    DWORD ReadTotalTimeoutMultiplier,
    DWORD ReadTotalTimeoutConstant,
    DWORD WriteTotalTimeoutMultiplier,
    DWORD WriteTotalTimeoutConstant)
{
    if ((m_bPortReady = GetCommTimeouts(m_hComm, &m_CommTimeouts)) == 0)
        return false;

    m_CommTimeouts.ReadIntervalTimeout = ReadIntervalTimeout;
    m_CommTimeouts.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;
    m_CommTimeouts.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;
    m_CommTimeouts.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant;
    m_CommTimeouts.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier;

    m_bPortReady = SetCommTimeouts(m_hComm, &m_CommTimeouts);

    if (m_bPortReady == 0)
    {
        ::OutputDebugString("StCommTimeouts function failed : COM Port Error");
        CloseHandle(m_hComm);

        return false;
    }

    return true;
}

void CSerialPort::Dispay(const char *_pre, const char* _data)
{
    char buf[5];
    for (int i = 0; i < 4; i++)
    {
        buf[i] = *_data;
        _data++;
    }
    buf[4] = 0;

    g_SendMessage (LOG_MSG, "%s%s\n", _pre, buf);
}

bool CSerialPort::WriteBytes(const char* _data, int size)
{
    Dispay("w-", _data);
    bool ret = true;
    for (int i = 0; i < size; i++)
    {
        ret = WriteByte(_data[i]);
        if (!ret)
            return false;
    }
    return true;
}

// *******************************************
// Write Byte
// *******************************************
bool CSerialPort::WriteByte(BYTE bybyte)
{
    int iBytesWritten = 0;

    if (WriteFile(m_hComm, &bybyte, 1, &m_iBytesWritten, NULL) == 0)
        return false;
    else
        return true;
}


// *******************************************
// Read Byte
// *******************************************
bool CSerialPort::ReadByte(BYTE& resp)
{
    BYTE rx;
    resp = 0;

    DWORD dwBytesTransferred = 0;

    if (ReadFile(m_hComm, &rx, 1, &dwBytesTransferred, 0))
    {
        if (dwBytesTransferred == 1)
        {
            resp = rx;
            return true;
        }
    }

    return false;
}


// *******************************************
// Read Byte (Overriding)
// *******************************************
bool CSerialPort::ReadByte(char *resp, UINT size)
{
    DWORD dwBytesTransferred = 0;

    if (ReadFile(m_hComm, resp, size, &dwBytesTransferred, 0))
    {
        if (dwBytesTransferred == size)
            return true;
    }

    return false;
}


// *******************************************
// Close Port
// *******************************************
void CSerialPort::ClosePort()
{
    CloseHandle(m_hComm);
    return;
}