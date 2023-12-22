#pragma once

struct sockaddr_in;

class BaseSocket
{
public:
	PtObjectHeader2(BaseSocket);
	PT_OPTHeader;
public:

	BaseSocket();
	BaseSocket(bool _bServer, const char *_strServer=NULL, unsigned short _nPort=5000 );
	void init(bool _bServer, const char *_strServer=NULL, unsigned short _nPort=5000 );
	virtual ~BaseSocket(void);

	void weakup();
	int bind();

	int send(char *_pData, int _nSize, char *_pRemote=NULL);
	int	receive(char *_pData, int _nSize, char *_pRemote=NULL, char *_anIP=NULL, short *_pnPort=NULL);

	bool is_connected();
	bool is_server()
	{
		return m_bServer;
	}
	bool socket_ready() {
		return m_bSocketInit;
	}
	void set_port(unsigned short _nPort);
	bool get_remote(char *_pRemote, char *_pnIP = 0, short *_pnPort = 0);
	
	static sockaddr_in *address_convert(sockaddr_in *_addr, const char *_server, unsigned short _port);
protected:
	void close_();
	bool	m_bConnected;
	unsigned short port_number;			/* Port number to use */
	int bytes_received;					/* Bytes received from client */
	char host_name[256];				/* Name of the server */
	time_t current_time;				/* Current time */

private:
	static int sm_nWinsockCount;
	bool	m_bSocketInit;
	bool	m_bServer;
#ifdef _WIN32
	UINT32	m_socket;
#else
    int             m_socket;
#endif
	char	m_sRemote[16];
	char	m_sLocal[16];
};
