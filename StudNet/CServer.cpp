#include "CServer.h"
//
unsigned __stdcall ListenThreadProc(LPVOID lParam);

// Функция для закрытия сокета;
void TCloseSocket(SOCKET Socket)
{
	try
	{
		shutdown(Socket, SD_BOTH);
		closesocket(Socket);
	}
	catch(...) {}
}

// Функция отправки данных;
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

	// Шаг 1: Получение сокета;
	SOCKET ClientSocket = (SOCKET)AcceptSocket;
	// Шаг 2: Инициализация дополнительных, нужных переменных;
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
	// Шаг 3: Обнуление буфера;
	ZeroMemory(InBuf, 1024);
	// Шаг 4: Создание пустого события;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		ErrorCode = WSAGetLastError();
		//	sText = "WSACreateEvent() failed with error: " + (String)ErrorCode;
		//WLog(g_Caption, sText);
		return 0;
	}
	// Шаг 5: Задаемое событие/я, которое/ые нужно будет обрабатывать;
	nRes = WSAEventSelect(ClientSocket, hEvent, FD_READ | FD_CLOSE);
	if (nRes == SOCKET_ERROR)
	{
		ErrorCode = WSAGetLastError();
		//	sText = "WSAEventSelect() failed with error: " + (String)ErrorCode;
		//WLog(g_Caption, sText);
		return 0;
	}
	// Шаг 6: Создаём бесконечный цикл;
	while(true)
	{
		// Шаг 7: Делаем задержку на прослушивание события о выходе из цикла;
		// Если событие сработало, то мы выходим;
		if (WaitForSingleObject(0, 100) == WAIT_OBJECT_0) break;
		// Шаг 8: Ожидаем какого-либо события;
		dwRet = WSAWaitForMultipleEvents(2, // Кол-во прослушивающих событий;
			&hEvent, // Указатель на событие;
			FALSE, // Ожидать пока случиться все события;
			100, // Кол-во мсек для прослушивания; (WSA_INFINITE - бесконечное ожидание)
			FALSE); // Используется для перекрытого ввода-ввыода;
		// Шаг 9: Проверка возвращаемого значения;
		if (dwRet == WSA_WAIT_TIMEOUT) continue;
		// Шаг 10: Получения списка событий, которые произошли;
		nRes = WSAEnumNetworkEvents(ClientSocket, hEvent, &events);
		if (nRes == SOCKET_ERROR)
		{
			ErrorCode = WSAGetLastError();
			//	sText = "WSAEnumNetworkEvents() failed with error: " + (String)ErrorCode;
			//WLog(g_Caption, sText);
			break;
		}
		// Шаг 11: Осуществляем выбор события;
		// Заметка: Не советую использовать switch(), так как если у вас несколько событий, то произойдёт обработка только первого события;
		if(events.lNetworkEvents & FD_CLOSE)
		{
			// Шаг 12: проверка события на наличие ошибок;
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
			// Шаг 13: проверка события на наличие ошибок;
			if (events.iErrorCode[FD_READ_BIT] == 0)
			{
				// Шаг 14: Причём данных;
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
				// Шаг 15: Проверка длины входящих данных;
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
				// Шаг 16: Очистка буфера для приёма данных;
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
	// Шаг 17: Удаление буфера;
	delete[] InBuf;
	// Шаг 18: Закрытие сокета;
	TCloseSocket(ClientSocket);
	return 0;
};

void __fastcall CServer::OnAccept()
{

	//if (g_bTimeToKill || g_bDisconnectAll) return;
	// Шаг 2: Инициализация дополнительных, нужных переменных;
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
	// Шаг 3: Установка соединения;
	// Заметка: Советую использовать именно функцию Accept();
	// WSAAccept() возвращает управления потоку сразу, без проверки успошности;
	// Accept() возвращает настоящую проверку успешности;
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
	// Шаг 4: Получаем IP-Адрес клиента;
	memcpy(&SockInAddr, &SockAddr.sin_addr, sizeof(SockInAddr));
	cIP = inet_ntoa(SockInAddr);
	// Шаг 5: Создаёт событие-завершение для потока;
	hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	// Шаг 6: Создаём новый поток для обмена данными;
	// Заметка: Создавать новый поток для обмена данными приемлен только в том случае,
	// Если кол-во пользователей не будет превышать 100 пользователей;

	ThreadHandle = CreateThread(NULL, // Защита;
		0,
		ClientWorkerThread, // Имя функции;
		(void*)ClientSocket, // Передаём в поток сокет;
		0,
		&ThreadId); // Адрес потока;
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
	a.sin_port = htons(777); // Задаём прослушивающий порт;
	a.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // Для всех сетевых интерфейсов;


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

	Thread = (HANDLE)_beginthreadex(NULL, // Защита
		0, // Размер стэка - по умолчанию;
		ListenThreadProc, // Имя функции;
		(void*) this, // Передача указателя на данный класс;
		0, // Инициализация флага;
		&TherdId); // Адрес потока;

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

		dwRet = WSAWaitForMultipleEvents(1, // Кол-во прослушивающих событий;
			&pThis->Event, // Указатель на событие;
			FALSE, // Ожидать пока случиться все события;
			100, // Кол-во мсек для прослушивания; (WSA_INFINITE - бесконечное ожидание)
			FALSE); // Используется для перекрытого ввода-ввыода;


		if (dwRet == WSA_WAIT_TIMEOUT) continue;
		nRes = WSAEnumNetworkEvents(pThis->Listen, pThis->Event, &events);
		if (nRes == SOCKET_ERROR)
		{
			ErrorCode = WSAGetLastError();
			//sText = "WSAEnumNetworkEvents() failed with error: " + (String)ErrorCode;
			//WLog(g_Caption, sText);
			break;
		}
		// Шаг 8: Осуществляем выбор события;
		// Заметка: Не советую использовать switch(), так как если у вас несколько событий, то произойдёт обработка только первого события;
		if(events.lNetworkEvents & FD_ACCEPT)
		{
			// Шаг 9: проверка события на наличие ошибок;
			if (events.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				// Шаг 10: Вызов функции подключения;
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

	return 0; // Нормальный выход из потока;

}