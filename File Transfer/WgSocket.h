#ifndef WGSOCKET_H
#define WGSOCKET_H
#include <winsock2.h>
using namespace std;

class WgSocket
{
private:
	SOCKET m_Socket;
public:
	static bool Initialize(void);
	static void Terminate(void);
	static bool IsLocalHost(const char* hostname);
	static bool GetHostIP(const char* hostname, int &ip1, int &ip2, int &ip3, int &ip4);
public:
	WgSocket(void);
	~WgSocket();
	bool IsOpened(void) const;
	void SetSocket(SOCKET socket);
	bool Open(const char* hostname, int port);
	void Close(void);
	bool WaitInputData(int seconds);
	bool Read(void* buffer, long len, long &ret_len);
	bool Write(const void* buffer, long len);
	bool Listen(int port);
	bool Accept(SOCKET &socket);
	bool SetNoDelay(void);
};

static bool m_Init_Flag = false;

bool WgSocket::Initialize(void)
//�����G��l��Socket����
//�Ǧ^�G���ѶǦ^false
{
	if (!m_Init_Flag)
	{
		WSAData wsa_data;
		try
		{
			if (WSAStartup(0x202, &wsa_data) != 0) return false; // ��l�ƥ��� //
		}
		catch (...)
		{
			return false;
		}
		m_Init_Flag = true;
	}
	return true;
}

void WgSocket::Terminate(void)
//�����G����Socket����
{
	if (m_Init_Flag)
	{
		try
		{
			WSACleanup();
		}
		catch (...)
		{
		}
		m_Init_Flag = false;
	}
}

WgSocket::WgSocket(void)
//�����G�غcSocket����
{
	m_Socket = INVALID_SOCKET;
}

WgSocket::~WgSocket()
//�����G�ѺcSocket����
{
	Close();
}

bool WgSocket::IsLocalHost(const char* hostname)
//�����G�ˬd�O�_��localhost�I�s
//��J�Ghostname = Server��}
//�Ǧ^�G�O�_��localhost�I�s
{
	if (hostname == NULL) return true;
	if (*hostname == 0) return true;
	if (stricmp(hostname, "localhost") == 0) return true;
	if (strcmp(hostname, "127.0.0.1") == 0) return true;
	return false;
}

bool WgSocket::IsOpened(void) const
//�����G�˴�Socket�O�_�w�}��
//�Ǧ^�G�˴����G
{
	if (m_Socket == INVALID_SOCKET) 
		return false;
	else
		return true;
}

bool WgSocket::Open(const char* hostname, int port)
//�����G�}�һPServer���s�u
//��J�Ghostname,port = Server��}�P�q�T��
//�Ǧ^�G���ѶǦ^false
{
	Close();
	if (!Initialize()) return false;
	struct sockaddr_in sock_addr;
	// �ѥXsocket address //
	if (IsLocalHost(hostname)) hostname = "127.0.0.1";
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	struct hostent *hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) return false;
	sock_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
	// �إ�socket //
	try
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	}
	catch (...)
	{
		m_Socket = INVALID_SOCKET;
		return false;
	}
	if (m_Socket == INVALID_SOCKET) return false;
	// �}�l�s�u //
	try
	{
		if (connect(m_Socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) >= 0) return true;
	}
	catch (...)
	{
	}
	// ���B�i�H�[�J�@�ǿ��~�B�z... //
	Close();
	return false;
}

void WgSocket::Close(void)
//�����G�����PServer���s�u
{
	if (!IsOpened()) return;
	try
	{
		shutdown(m_Socket, SD_SEND);
	}
	catch (...)
	{
	}
	try
	{
		closesocket(m_Socket);
	}
	catch (...)
	{
	}
	m_Socket = INVALID_SOCKET;
}

bool WgSocket::Listen(int port)
//�����G��ť�Y��Port
//��J�Gport = ��ťPort
//�Ǧ^�G���ѶǦ^false
{
	Close();
	if (!Initialize()) return false;
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(port);
	// �إ�socket //
	try
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	}
	catch (...)
	{
		m_Socket = INVALID_SOCKET;
		return false;
	}
	if (m_Socket == INVALID_SOCKET) return false;
	// Bind socket //
	int on = 1;
	setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	int rc;
	try
	{
		rc = bind(m_Socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	}
	catch (...)
	{
		rc = SOCKET_ERROR;
	}
	if (rc == SOCKET_ERROR)
	{
		Close();
		return false;
	}
	// Listen socket //
	try
	{
		rc = listen(m_Socket, SOMAXCONN);
	}
	catch (...)
	{
		rc = SOCKET_ERROR;
	}
	if (rc == SOCKET_ERROR)
	{
		Close();
		return false;
	}
	return true;
}

bool WgSocket::Accept(SOCKET &socket)
//�����G���ݱ����s�u
//��X�G�s�usocket
//�Ǧ^�G���ѶǦ^false
{
	socket = INVALID_SOCKET;
	if (!IsOpened()) 
		return false;
	struct sockaddr_in from;
	int fromlen = sizeof(from);
	try
	{
		socket = accept(m_Socket, (struct sockaddr*)&from, &fromlen);
	}
	catch (...)
	{
		socket = INVALID_SOCKET;
		return false;
	}
	return true;
}

void WgSocket::SetSocket(SOCKET socket)
//�����G�]�w�s�u��socket
//��J�Gsocket = �s�u��socket
{
	Close();
	m_Socket = socket;
}

bool WgSocket::WaitInputData(int seconds)
//�����G���ݹ��e�Ӹ��
//��J�Gseconds = ���ݬ��
//�Ǧ^�G�S����ƶǦ^false
{
	if (!IsOpened()) return false;
	// �]�wdescriptor sets //
	fd_set socket_set;
	FD_ZERO(&socket_set);
	FD_SET((unsigned int)m_Socket, &socket_set);
	// �]�wtimeout�ɶ� //
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	// �����O�_����� //
	try
	{
		if (select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout) <= 0) return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool WgSocket::Read(void* data, long len, long &ret_len)
//�����GŪ�����
//��J�Gdata, len = ��ƽw�İϻP�j�p
//��X�Gdata = Ū�������, ret_len = ���Ū������Ƥj�p�A0����w�_�u
//�Ǧ^�G���ѶǦ^false
//�Ƶ��G����Ʒ|�@�����즳Ū����Ʃε����s�u�ɤ~�Ǧ^
{
	ret_len = 0;
	if (!IsOpened()) return true;
	try
	{
		ret_len = recv(m_Socket, (char*)data, len, 0);
	}
	catch (...)
	{
		ret_len = SOCKET_ERROR;
	}
	if (ret_len < 0)
	{
		ret_len = 0;
		return false;
	}
	return true;
}

bool WgSocket::Write(const void* data, long len)
//�����G�e�X���
//��J�Gdata, len = ��ƽw�İϻP�j�p
//�Ǧ^�G���ѶǦ^false
{
	if (!IsOpened()) return false;
	if (len <= 0) return true;
	int write_len;
	try
	{
		write_len = send(m_Socket, (const char*)data, len, 0);
	}
	catch (...)
	{
		write_len = SOCKET_ERROR;
	}
	if (write_len != len) return false;
	return true;
}

bool WgSocket::GetHostIP(const char* hostname, int &ip1, int &ip2, int &ip3, int &ip4)
//�����G���o���whost��ip
//��J�Ghostname = host��}
//��X�Gip1-4 = ip��}
//�Ǧ^�G���ѶǦ^false
{
	if (IsLocalHost(hostname))
	{
		// �����X��ڪ�hostname //
		struct hostent *hostinfo = gethostbyname("localhost");
		if (hostinfo == NULL) return false;
		hostname = hostinfo->h_name;
	}
	struct hostent* hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) return false;
	char* addr = hostinfo->h_addr_list[0];
	ip1 = (unsigned char)addr[0];
	ip2 = (unsigned char)addr[1];
	ip3 = (unsigned char)addr[2];
	ip4 = (unsigned char)addr[3];
	return true;
}

bool WgSocket::SetNoDelay(void)
//�����G�]�w������ǰe (����Nagle Algorithm)
//�Ǧ^�G�]�w���ѶǦ^false
{
	if (!IsOpened()) return false;
	int on = 1;
	if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on)) != 0) return false;
	return true;
}

#endif