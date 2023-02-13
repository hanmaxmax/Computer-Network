#include <iostream>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

const int PORT = 8000; //�˿ں�
#define MaxClient 4 //�������ͻ���=MaxClient-1
#define MaxBufSize 1024 //��󻺳�����С
int conn_count = 0; //��ǰ���ӵĿͻ���
SOCKET clientSockets[MaxClient];//�ͻ���socket����
SOCKADDR_IN clientAddrs[MaxClient];//�ͻ��˵�ַ����
int flags[MaxClient];//flag����


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
	//=========================��ʼ��socket dll=========================
	WSAData wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata); //MAKEWORD(���汾��, ���汾��)
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		cout << "��ʼ��Socket DLL��fail��\n" << endl;
		return -1;
	}
	cout << "��ʼ��Socket DLL��success!\n" << endl;

	//=========================����socket=========================
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//����TCPЭ�飬��ʽ�׽���
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "����socket��fail��\n" << endl;
		return -1;
	}
	cout << "����socket��success��\n" << endl;


	//=====================��ʼ����������ַ && bind=====================
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET; //��ַ����
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //��ַ
	serverAddr.sin_port = htons(PORT); //�˿ں�
	int tem = bind(serverSocket, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (tem == SOCKET_ERROR)
	{
		cout << "bind��fail��\n" << endl;
		return -1;
	}
	else
	{
		cout << "bind to port " << PORT << "��success��\n" << endl;
	}

	//=========================listen=========================
	tem = listen(serverSocket, MaxClient); // ���������г���Ϊ MaxClient 
	if (tem != 0)
	{
		cout << "listen��fail��\n" << endl;
		return -1;
	}
	else
	{
		cout << "listen��success��\n" << endl;
	}

	cout << "Server ready��Waiting for client request..." << endl << endl;


	//=========================ѭ������client=========================
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
				cout << "wrong client��\n" << endl;
				closesocket(serverSocket);
				WSACleanup();
				return -1;
			}
			time_t ts = time(NULL);//ʱ���ǩ
			char mytime[32]{ 0 };
			strncpy(mytime, ctime(&ts), sizeof(mytime));
			cout << "client��" << clientSockets[kk] << "��connect��number of current clients��" << conn_count <<". TIME: " << mytime;
			HANDLE hThread = CreateThread(NULL, NULL, handlerRequest, (LPVOID)kk, 0, NULL);
			CloseHandle(hThread);
		}
		else
		{
			cout << "Request is full��" << endl;
			return -1;
		}
	}
	closesocket(serverSocket); //�ر�socket
	WSACleanup(); 
	return 0;
}





//�̺߳���
DWORD WINAPI handlerRequest(LPVOID lpParam)
{
	int n = (int)lpParam;
	int receByt = 0;
	char RecvBuf[MaxBufSize]; //������Ϣ�Ļ�����
	char SendBuf[MaxBufSize]; //������Ϣ�Ļ�����

	while (1)
	{
		//�������Ը��߳�����Ӧsocket����Ϣ
		receByt = recv(clientSockets[n], RecvBuf, sizeof(RecvBuf), 0);

		if (receByt > 0)
		{
			time_t ts = time(NULL);//ʱ���ǩ
			char mytime[32]{ 0 };
			strncpy(mytime, ctime(&ts), sizeof(mytime));
			cout << "client" << clientSockets[n] << "��" << RecvBuf << "     --" << mytime;

			sprintf(SendBuf, "[#%d]��%s\n		--%s", clientSockets[n], RecvBuf, mytime);

			//====================== ������socket��������Ϣ =====================
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
			//====================== �ر����ӣ��ͷ���Դ =====================
			if (WSAGetLastError() == 10054)//�ͻ��������ر�����
			{
				time_t ts = time(NULL);
				char mytime[32]{ 0 };
				strncpy(mytime, ctime(&ts), sizeof(mytime));
				cout << "client " << clientSockets[n] << " exit, number of current clients��" << conn_count - 1 << ". TIME: "<<mytime;
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


