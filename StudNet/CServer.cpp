#include "CServer.h"
//
unsigned __stdcall ListenThreadProc(LPVOID lParam);

// ������� ��� �������� ������;
void TCloseSocket(SOCKET Socket)
{
	try
	{
		shutdown(Socket, SD_BOTH);
		closesocket(Socket);
	}
	catch(...) {}
}

// ������� �������� ������;
void SendBufWSA(SOCKET Socket, LPBYTE Packet, int len)
{
	try
	{
		WSABUF DataBuf;
		// ----
		DataBuf.len = (ULONG)len;
		DataBuf.buf = (char*)Packet;
		// ----
		DWORD SendBytes;
		WSAOVERLAPPED SendOverlapped = {0};
		// ----
		WSASend(Socket, &DataBuf, 1, &SendBytes, 0, &SendOverlapped, NULL);
	}
	catch(...) {}
};

DWORD WINAPI ClientWorkerThread(LPVOID AcceptSocket)
{

	// ��� 1: ��������� ������;
	SOCKET ClientSocket = (SOCKET)AcceptSocket;
	// ��� 2: ������������� ��������������, ������ ����������;
	byte *InBuf = new byte[1024];
	//AnsiString sText;
	int nRes = 0;
	DWORD dwRet = 0;
	int ErrorCode = 0;
	WSABUF DataBuf;
	DataBuf.len = 1024;
	DataBuf.buf = (CHAR *)InBuf;
	DWORD RecvBytes = 0;
	unsigned long Flags = 0;
	WSAEVENT hEvent = WSA_INVALID_EVENT;
	WSANETWORKEVENTS events;
	// ��� 3: ��������� ������;
	ZeroMemory(InBuf, 1024);
	// ��� 4: �������� ������� �������;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		ErrorCode = WSAGetLastError();
		//	sText = "WSACreateEvent() failed with error: " + (String)ErrorCode;
		//WLog(g_Caption, sText);
		return 0;
	}
	// ��� 5: �������� �������/�, �������/�� ����� ����� ������������;
	nRes = WSAEventSelect(ClientSocket, hEvent, FD_READ | FD_CLOSE);
	if (nRes == SOCKET_ERROR)
	{
		ErrorCode = WSAGetLastError();
		//	sText = "WSAEventSelect() failed with error: " + (String)ErrorCode;
		//WLog(g_Caption, sText);
		return 0;
	}
	// ��� 6: ������ ����������� ����;
	while(true)
	{
		// ��� 7: ������ �������� �� ������������� ������� � ������ �� �����;
		// ���� ������� ���������, �� �� �������;
		if (WaitForSingleObject(0, 100) == WAIT_OBJECT_0) break;
		// ��� 8: ������� ������-���� �������;
		dwRet = WSAWaitForMultipleEvents(2, // ���-�� �������������� �������;
			&hEvent, // ��������� �� �������;
			FALSE, // ������� ���� ��������� ��� �������;
			100, // ���-�� ���� ��� �������������; (WSA_INFINITE - ����������� ��������)
			FALSE); // ������������ ��� ����������� �����-������;
		// ��� 9: �������� ������������� ��������;
		if (dwRet == WSA_WAIT_TIMEOUT) continue;
		// ��� 10: ��������� ������ �������, ������� ���������;
		nRes = WSAEnumNetworkEvents(ClientSocket, hEvent, &events);
		if (nRes == SOCKET_ERROR)
		{
			ErrorCode = WSAGetLastError();
			//	sText = "WSAEnumNetworkEvents() failed with error: " + (String)ErrorCode;
			//WLog(g_Caption, sText);
			break;
		}
		// ��� 11: ������������ ����� �������;
		// �������: �� ������� ������������ switch(), ��� ��� ���� � ��� ��������� �������, �� ��������� ��������� ������ ������� �������;
		if(events.lNetworkEvents & FD_CLOSE)
		{
			// ��� 12: �������� ������� �� ������� ������;
			if (events.iErrorCode[FD_CLOSE_BIT] == 0)
			{
				break;
			}
			else
			{
				ErrorCode = WSAGetLastError();
				//	sText = "events.iErrorCode[FD_CLOSE_BIT] failed with error: " + (String)ErrorCode;
				//WLog(g_Caption, sText);
				break;
			}
		}
		// ----
		if(events.lNetworkEvents & FD_READ)
		{
			// ��� 13: �������� ������� �� ������� ������;
			if (events.iErrorCode[FD_READ_BIT] == 0)
			{
				// ��� 14: ������ ������;
				nRes = WSARecv(ClientSocket, &DataBuf, 1, &RecvBytes, &Flags, NULL, NULL);
				if(nRes == SOCKET_ERROR)
				{
					ErrorCode = WSAGetLastError();
					if(ErrorCode != WSA_IO_PENDING)
					{
						//	sText = "WSARecv() failed with error: " + (String)ErrorCode;
						//WLog(g_Caption, sText);
						break;
					}
				}
				// ��� 15: �������� ����� �������� ������;
				if (RecvBytes > 0 && RecvBytes < 1024)
				{
					//PacketCore(ClientSocket, InBuf, RecvBytes);
				}
				else
				{
					//	sText = "WSARecv() out of range: " + (String)RecvBytes;
					//WLog(g_Caption, sText);
					break;
				}
				// ��� 16: ������� ������ ��� ����� ������;
				ZeroMemory(InBuf, RecvBytes);
			}
			else
			{
				ErrorCode = WSAGetLastError();
				//	sText = "events.iErrorCode[FD_READ_BIT] failed with error: " + (String)ErrorCode;
				//WLog(g_Caption, sText);
				break;
			}
		}
	}
	// ��� 17: �������� ������;
	delete[] InBuf;
	// ��� 18: �������� ������;
	TCloseSocket(ClientSocket);
	return 0;
};

void __fastcall CServer::OnAccept()
{

	//if (g_bTimeToKill || g_bDisconnectAll) return;
	// ��� 2: ������������� ��������������, ������ ����������;
	SOCKET        ClientSocket;
	SOCKADDR_IN SockAddr;
	IN_ADDR        SockInAddr;
	int SockAddrLen = sizeof(SOCKADDR);
	int nLen = sizeof(SOCKADDR_IN);
	char  *cIP = "";
	int ErrorCode = 0;
	HANDLE hKillEvent;
	HANDLE ThreadHandle;
	DWORD ThreadId;
	// ��� 3: ��������� ����������;
	// �������: ������� ������������ ������ ������� Accept();
	// WSAAccept() ���������� ���������� ������ �����, ��� �������� ����������;
	// Accept() ���������� ��������� �������� ����������;
	ClientSocket = accept(Listen, (LPSOCKADDR)&SockAddr, &nLen);
	if (ClientSocket == SOCKET_ERROR)
	{
		ErrorCode = WSAGetLastError();
		if (ErrorCode != WSAEWOULDBLOCK)
		{
			//sText = "accept() failed with error: " + (String)ErrorCode;
			//WLog(g_Caption, sText);
			return;
		}
	}
	// ��� 4: �������� IP-����� �������;
	memcpy(&SockInAddr, &SockAddr.sin_addr, sizeof(SockInAddr));
	cIP = inet_ntoa(SockInAddr);
	// ��� 5: ������ �������-���������� ��� ������;
	hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	// ��� 6: ������ ����� ����� ��� ������ �������;
	// �������: ��������� ����� ����� ��� ������ ������� �������� ������ � ��� ������,
	// ���� ���-�� ������������� �� ����� ��������� 100 �������������;

	ThreadHandle = CreateThread(NULL, // ������;
		0,
		ClientWorkerThread, // ��� �������;
		(void*)ClientSocket, // ������� � ����� �����;
		0,
		&ThreadId); // ����� ������;
}
CServer::CServer()
{
	res = WSAStartup(MAKEWORD(2, 2),&wsadata);

	if(res!=0)
	{
		errorCode = WSAGetLastError();
		char buffer[255];
		sprintf(buffer,"%d",errorCode);

		string str = (string)"WSAStartup() failed with error: "  + buffer;
		MessageBoxA(0,(LPCSTR)str.c_str(),(LPCSTR)"Error",MB_ICONERROR);
	}

	port = default_port;
	Interface = false;
	RecvOnly  = false;
	AddrSize  = 0;
	Thread	  = 0;

}

CServer::~CServer()
{
	WSACleanup();
}

void CServer::setupServer()
{
	Listen  = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(Listen==INVALID_SOCKET)
	{
		errorCode = WSAGetLastError();
		closesocket(Listen);
		char buffer[255];
		sprintf(buffer,"%d",errorCode);

		string str = (string)"WSASocket() failed with error: "  + buffer;
		MessageBoxA(0,(LPCSTR)str.c_str(),(LPCSTR)"Error",MB_ICONERROR);
	}

	Event = WSACreateEvent();
	if (Event == WSA_INVALID_EVENT)
	{
		errorCode = WSAGetLastError();
		closesocket(Listen);
		char buffer[255];
		sprintf(buffer,"%d",errorCode);

		string str = (string)"WSAEventSelect() failed with error: "  + buffer;
		MessageBoxA(0,(LPCSTR)str.c_str(),(LPCSTR)"Error",MB_ICONERROR);

	}

	sockaddr_in a;
	a.sin_family = AF_INET;
	a.sin_port = htons(777); // ����� �������������� ����;
	a.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // ��� ���� ������� �����������;


	res = bind(Listen, (sockaddr*)&a, sizeof(sockaddr));

	if(res == SOCKET_ERROR)
	{
		errorCode = WSAGetLastError();
		closesocket(Listen);
		char buffer[255];
		sprintf(buffer,"%d",errorCode);

		string str = (string)"bind() failed with error: "  + buffer;
		MessageBoxA(0,(LPCSTR)str.c_str(),(LPCSTR)"Error",MB_ICONERROR);
	};

	res = listen(Listen, 15);

	Thread = (HANDLE)_beginthreadex(NULL, // ������
		0, // ������ ����� - �� ���������;
		ListenThreadProc, // ��� �������;
		(void*) this, // �������� ��������� �� ������ �����;
		0, // ������������� �����;
		&TherdId); // ����� ������;

}

bool CServer::GetError()
{
	int  error;
	//if((error = WSAGetLastError())!=0)
	return true;

	return false;
}

bool CServer::serverListen()
{
	AddrSize = sizeof(Client);
	Client   = accept(Listen,(struct sockaddr *)&client,&AddrSize);
	if(Client == INVALID_SOCKET)
	{
		return false;
	}
	printf("Aceepted client %s %d",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
	return true;
}

unsigned _stdcall ListenThreadProc(LPVOID lParam)
{

	CServer* pThis = reinterpret_cast<CServer*>(lParam);

	DWORD dwRet = 0;
	int nRes = 0;
	int ErrorCode = 0;
	string sText = "";
	WSANETWORKEVENTS events;

	while(true)
	{

		if (WaitForSingleObject(pThis->KillEvent, 100) == WAIT_OBJECT_0) break;

		dwRet = WSAWaitForMultipleEvents(1, // ���-�� �������������� �������;
			&pThis->Event, // ��������� �� �������;
			FALSE, // ������� ���� ��������� ��� �������;
			100, // ���-�� ���� ��� �������������; (WSA_INFINITE - ����������� ��������)
			FALSE); // ������������ ��� ����������� �����-������;


		if (dwRet == WSA_WAIT_TIMEOUT) continue;
		nRes = WSAEnumNetworkEvents(pThis->Listen, pThis->Event, &events);
		if (nRes == SOCKET_ERROR)
		{
			ErrorCode = WSAGetLastError();
			//sText = "WSAEnumNetworkEvents() failed with error: " + (String)ErrorCode;
			//WLog(g_Caption, sText);
			break;
		}
		// ��� 8: ������������ ����� �������;
		// �������: �� ������� ������������ switch(), ��� ��� ���� � ��� ��������� �������, �� ��������� ��������� ������ ������� �������;
		if(events.lNetworkEvents & FD_ACCEPT)
		{
			// ��� 9: �������� ������� �� ������� ������;
			if (events.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				// ��� 10: ����� ������� �����������;
				pThis->OnAccept();
			}
			else
			{
				ErrorCode = WSAGetLastError();
				// sText = "events.lNetworkEvents & FD_ACCEPT failed with error: " + (String)ErrorCode;
				//WLog(g_Caption, sText);
			}


		}


	}

	return 0; // ���������� ����� �� ������;

}