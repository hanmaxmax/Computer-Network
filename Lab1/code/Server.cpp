#include <iostream>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

const int PORT = 8000; //端口号
#define MaxClient 4 //最大允许客户数=MaxClient-1
#define MaxBufSize 1024 //最大缓冲区大小
int conn_count = 0; //当前连接的客户数
SOCKET clientSockets[MaxClient];//客户端socket数组
SOCKADDR_IN clientAddrs[MaxClient];//客户端地址数组
int flags[MaxClient];//flag数组


DWORD WINAPI handlerRequest(LPVOID lpParameter);



int find_flag()
{
	for (int i = 0; i < MaxClient; i++)
	{
		if (flags[i] == 0)
		{
			return i;
		}
	}
	return -1;
}




int main()
{
	//=========================初始化socket dll=========================
	WSAData wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata); //MAKEWORD(主版本号, 副版本号)
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		cout << "初始化Socket DLL：fail！\n" << endl;
		return -1;
	}
	cout << "初始化Socket DLL：success!\n" << endl;

	//=========================创建socket=========================
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//采用TCP协议，流式套接字
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "创建socket：fail！\n" << endl;
		return -1;
	}
	cout << "创建socket：success！\n" << endl;


	//=====================初始化服务器地址 && bind=====================
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET; //地址类型
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //地址
	serverAddr.sin_port = htons(PORT); //端口号
	int tem = bind(serverSocket, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (tem == SOCKET_ERROR)
	{
		cout << "bind：fail！\n" << endl;
		return -1;
	}
	else
	{
		cout << "bind to port " << PORT << "：success！\n" << endl;
	}

	//=========================listen=========================
	tem = listen(serverSocket, MaxClient); // 最大监听队列长度为 MaxClient 
	if (tem != 0)
	{
		cout << "listen：fail！\n" << endl;
		return -1;
	}
	else
	{
		cout << "listen：success！\n" << endl;
	}

	cout << "Server ready！Waiting for client request..." << endl << endl;


	//=========================循环接收client=========================
	while (1)
	{
		if (conn_count < MaxClient)
		{
			int len = sizeof(SOCKADDR);
			int kk = find_flag();
			clientSockets[kk] = accept(serverSocket, (sockaddr*)&clientAddrs[kk], &len);
			flags[kk] = 1;
			conn_count++;
			if (clientSockets[kk] == SOCKET_ERROR)
			{
				cout << "wrong client！\n" << endl;
				closesocket(serverSocket);
				WSACleanup();
				return -1;
			}
			time_t ts = time(NULL);//时间标签
			char mytime[32]{ 0 };
			strncpy(mytime, ctime(&ts), sizeof(mytime));
			cout << "client（" << clientSockets[kk] << "）connect，number of current clients：" << conn_count <<". TIME: " << mytime;
			HANDLE hThread = CreateThread(NULL, NULL, handlerRequest, (LPVOID)kk, 0, NULL);
			CloseHandle(hThread);
		}
		else
		{
			cout << "Request is full！" << endl;
			return -1;
		}
	}
	closesocket(serverSocket); //关闭socket
	WSACleanup(); 
	return 0;
}





//线程函数
DWORD WINAPI handlerRequest(LPVOID lpParam)
{
	int n = (int)lpParam;
	int receByt = 0;
	char RecvBuf[MaxBufSize]; //接收消息的缓冲区
	char SendBuf[MaxBufSize]; //发送消息的缓冲区

	while (1)
	{
		//接收来自该线程所对应socket的消息
		receByt = recv(clientSockets[n], RecvBuf, sizeof(RecvBuf), 0);

		if (receByt > 0)
		{
			time_t ts = time(NULL);//时间标签
			char mytime[32]{ 0 };
			strncpy(mytime, ctime(&ts), sizeof(mytime));
			cout << "client" << clientSockets[n] << "：" << RecvBuf << "     --" << mytime;

			sprintf(SendBuf, "[#%d]：%s\n		--%s", clientSockets[n], RecvBuf, mytime);

			//====================== 向其他socket都发送消息 =====================
			for (int i = 0; i < MaxClient; i++)
			{
				if (flags[i] == 1)
				{
					send(clientSockets[i], SendBuf, sizeof(SendBuf), 0);
				}
			}
		}

		else
		{
			//====================== 关闭连接，释放资源 =====================
			if (WSAGetLastError() == 10054)//客户端主动关闭连接
			{
				time_t ts = time(NULL);
				char mytime[32]{ 0 };
				strncpy(mytime, ctime(&ts), sizeof(mytime));
				cout << "client " << clientSockets[n] << " exit, number of current clients：" << conn_count - 1 << ". TIME: "<<mytime;
				closesocket(clientSockets[n]);
				flags[n] = 0;
				conn_count--;
				return 0;
			}
			cout << "failed to receive,Error:" << WSAGetLastError() << endl;
			return 0;
		}
	}

}


