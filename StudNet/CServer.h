#ifndef CServer_h__
#define CServer_h__


#include <WinSock2.h>
#include <iostream>
#include <string>
#include <process.h>

using namespace std;
#pragma comment(lib,"Ws2_32.lib")

const int default_port = 5150;
const int default_buffer = 4096;

class CServer{

public:
	CServer();
	virtual ~CServer();
	//unsigned &__stdcall ListenThreadProc(LPVOID lParam);
	void __fastcall CServer::OnAccept();
	void setupServer();
	bool serverListen();
	bool GetError();

	WSAEVENT Event;

	int port;
	int res;
	int errorCode;


	char Address[128];
	int AddrSize;

	HANDLE Thread;
	HANDLE KillEvent;
	UINT  TherdId;

	WSADATA wsadata;

	SOCKET Listen;
	SOCKET Client;


	bool Interface;// прослушивать указанный интерфейс
	bool RecvOnly; // только прием данных

	struct sockaddr_in local,client;
};





#endif // CServer_h__