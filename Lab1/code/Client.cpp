
#include<iostream>
#include<Winsock2.h>//socketͷ�ļ�
#include<cstring>
#pragma comment(lib,"ws2_32.lib")   //socket��
using namespace std;


const int PORT = 8000; //�˿ں�
const int BufSize = 1024; //��������С
SOCKET clientSocket; //�ͻ���socket

DWORD WINAPI recvThread();

int main() 
{

	//=========================��ʼ��socket dll=========================
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		cout << "��ʼ��Socket DLL��fail��\n" << endl;
		return -1;
	}
	cout << "��ʼ��Socket DLL��success!\n" << endl;



	//=========================�����ͻ��˵�socket=========================
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//������·����ʽ�׽���,���������������Զ�ѡ��Э��
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "����socket��fail��\n" << endl;
		return -1;
	}
	cout << "����socket��success��\n" << endl;


	//=========================��������ַ=========================
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;//��ַ����
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//��ַ
	servAddr.sin_port = htons(PORT);//�˿ں�


	//========================= connect =========================
	int r = connect(clientSocket, (SOCKADDR*)&servAddr, sizeof(SOCKADDR)) == SOCKET_ERROR;
	if (r==-1)
	{
		cout << "connect fail��" << WSAGetLastError() << endl;
	}
	else
	{
		cout << "connect��success��\n\n";
	}
	cout << "Now you can start chatting!\n\n";

	//=================== ����������Ϣ�߳� =====================
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvThread, NULL, 0, 0);
	
	//===================== ���߳����ڼ�����Ϣ��������Ϣ =====================
	while (1)
	{
		char buf[BufSize] = { 0 };
		cin.getline(buf, sizeof(buf));
		if (strcmp(buf, "quit") == 0)//�����롰quit�������˳�
		{
			break;
		}
		send(clientSocket, buf, sizeof(buf), 0);
	}
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}


//������Ϣ���߳�
DWORD WINAPI recvThread()
{
	while (1)
	{
		char buffer[BufSize] = { 0 };//������Ϣ�Ļ�����
		int r = recv(clientSocket, buffer, sizeof(buffer), 0);//nrecv�ǽ��յ����ֽ���
		if (r > 0)//������յ����ַ�������0
		{
			cout << buffer << endl;
		}
		else if (r < 0)//������յ����ַ���С��0��˵���Ͽ�����
		{
			cout << "loss connect!" << endl;
			break;
		}
	}
	return 0;
}