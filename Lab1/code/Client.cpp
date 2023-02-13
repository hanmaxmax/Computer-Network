
#include<iostream>
#include<Winsock2.h>//socket头文件
#include<cstring>
#pragma comment(lib,"ws2_32.lib")   //socket库
using namespace std;


const int PORT = 8000; //端口号
const int BufSize = 1024; //缓冲区大小
SOCKET clientSocket; //客户端socket

DWORD WINAPI recvThread();

int main() 
{

	//=========================初始化socket dll=========================
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		cout << "初始化Socket DLL：fail！\n" << endl;
		return -1;
	}
	cout << "初始化Socket DLL：success!\n" << endl;



	//=========================创建客户端的socket=========================
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//面向网路的流式套接字,第三个参数代表自动选择协议
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "创建socket：fail！\n" << endl;
		return -1;
	}
	cout << "创建socket：success！\n" << endl;


	//=========================服务器地址=========================
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;//地址类型
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//地址
	servAddr.sin_port = htons(PORT);//端口号


	//========================= connect =========================
	int r = connect(clientSocket, (SOCKADDR*)&servAddr, sizeof(SOCKADDR)) == SOCKET_ERROR;
	if (r==-1)
	{
		cout << "connect fail：" << WSAGetLastError() << endl;
	}
	else
	{
		cout << "connect：success！\n\n";
	}
	cout << "Now you can start chatting!\n\n";

	//=================== 创建接受消息线程 =====================
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvThread, NULL, 0, 0);
	
	//===================== 主线程用于键入消息并发送消息 =====================
	while (1)
	{
		char buf[BufSize] = { 0 };
		cin.getline(buf, sizeof(buf));
		if (strcmp(buf, "quit") == 0)//若输入“quit”，则退出
		{
			break;
		}
		send(clientSocket, buf, sizeof(buf), 0);
	}
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}


//接收消息的线程
DWORD WINAPI recvThread()
{
	while (1)
	{
		char buffer[BufSize] = { 0 };//接收消息的缓冲区
		int r = recv(clientSocket, buffer, sizeof(buffer), 0);//nrecv是接收到的字节数
		if (r > 0)//如果接收到的字符数大于0
		{
			cout << buffer << endl;
		}
		else if (r < 0)//如果接收到的字符数小于0就说明断开连接
		{
			cout << "loss connect!" << endl;
			break;
		}
	}
	return 0;
}