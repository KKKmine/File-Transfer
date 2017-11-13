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
//說明：初始化Socket物件
//傳回：失敗傳回false
{
	if (!m_Init_Flag)
	{
		WSAData wsa_data;
		try
		{
			if (WSAStartup(0x202, &wsa_data) != 0) return false; // 初始化失敗 //
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
//說明：結束Socket物件
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
//說明：建構Socket物件
{
	m_Socket = INVALID_SOCKET;
}

WgSocket::~WgSocket()
//說明：解構Socket物件
{
	Close();
}

bool WgSocket::IsLocalHost(const char* hostname)
//說明：檢查是否為localhost呼叫
//輸入：hostname = Server位址
//傳回：是否為localhost呼叫
{
	if (hostname == NULL) return true;
	if (*hostname == 0) return true;
	if (stricmp(hostname, "localhost") == 0) return true;
	if (strcmp(hostname, "127.0.0.1") == 0) return true;
	return false;
}

bool WgSocket::IsOpened(void) const
//說明：檢測Socket是否已開啟
//傳回：檢測結果
{
	if (m_Socket == INVALID_SOCKET) 
		return false;
	else
		return true;
}

bool WgSocket::Open(const char* hostname, int port)
//說明：開啟與Server的連線
//輸入：hostname,port = Server位址與通訊埠
//傳回：失敗傳回false
{
	Close();
	if (!Initialize()) return false;
	struct sockaddr_in sock_addr;
	// 解出socket address //
	if (IsLocalHost(hostname)) hostname = "127.0.0.1";
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	struct hostent *hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) return false;
	sock_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
	// 建立socket //
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
	// 開始連線 //
	try
	{
		if (connect(m_Socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) >= 0) return true;
	}
	catch (...)
	{
	}
	// 此處可以加入一些錯誤處理... //
	Close();
	return false;
}

void WgSocket::Close(void)
//說明：關閉與Server的連線
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
//說明：接聽某個Port
//輸入：port = 接聽Port
//傳回：失敗傳回false
{
	Close();
	if (!Initialize()) return false;
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = htons(port);
	// 建立socket //
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
//說明：等待接收連線
//輸出：連線socket
//傳回：失敗傳回false
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
//說明：設定連線的socket
//輸入：socket = 連線的socket
{
	Close();
	m_Socket = socket;
}

bool WgSocket::WaitInputData(int seconds)
//說明：等待對方送來資料
//輸入：seconds = 等待秒數
//傳回：沒有資料傳回false
{
	if (!IsOpened()) return false;
	// 設定descriptor sets //
	fd_set socket_set;
	FD_ZERO(&socket_set);
	FD_SET((unsigned int)m_Socket, &socket_set);
	// 設定timeout時間 //
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	// 偵測是否有資料 //
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
//說明：讀取資料
//輸入：data, len = 資料緩衝區與大小
//輸出：data = 讀取的資料, ret_len = 實際讀取的資料大小，0表對方已斷線
//傳回：失敗傳回false
//備註：本函數會一直等到有讀取資料或結束連線時才傳回
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
//說明：送出資料
//輸入：data, len = 資料緩衝區與大小
//傳回：失敗傳回false
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
//說明：取得指定host的ip
//輸入：hostname = host位址
//輸出：ip1-4 = ip位址
//傳回：失敗傳回false
{
	if (IsLocalHost(hostname))
	{
		// 先取出實際的hostname //
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
//說明：設定不延遲傳送 (停掉Nagle Algorithm)
//傳回：設定失敗傳回false
{
	if (!IsOpened()) return false;
	int on = 1;
	if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on)) != 0) return false;
	return true;
}

#endif