#include <iostream>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <fstream>
#pragma comment (lib, "ws2_32.lib")
#include "Message.h"
using namespace std;

const int RouterPORT = 30000; //路由器端口号
const int ClientPORT = 20000; //client端口号


int global_seq = 0;



//实现client（发送端）的三次握手
bool ConnectWithServer(SOCKET clientSocket, SOCKADDR_IN serverAddr)
{
	int AddrLen = sizeof(serverAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;

	//=============发送第一次握手的消息（SYN=1，seq=x）============
	buffer1.SrcPort = ClientPORT;
	buffer1.DestPort = RouterPORT;
	buffer1.flag += SYN;//设置SYN
	buffer1.SeqNum = ++global_seq;//设置序号seq
	buffer1.setCheck();//设置校验和
	int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
	clock_t buffer1start = clock();
	if (sendByte == 0)
	{
		cout << "连接失败......关闭连接！" << endl;
		return false;
	}
	cout << "client已发送第一次握手的消息！" << endl;

	//=============接收第二次握手的消息（SYN=1，ACK=1，ack=x）============
	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "连接失败......关闭连接！" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//成功收到消息，检查校验和、ACK、SYN、ack
			if ((buffer2.flag && ACK) && (buffer2.flag && SYN) && buffer2.check() && (buffer2.AckNum == buffer1.SeqNum))
			{
				cout << "client已收到第二次握手的消息！" << endl;
				break;
			}
			else
			{
				cout << "连接发生错误！" << endl;
				return false;
			}
		}
		//buffer1超时，重新发送并重新计时
		if (clock() - buffer1start > MAX_WAIT_TIME)
		{
			cout << "第一次握手超时，正在重传......" << endl;
			//============重传buffer1==============
			//buffer1.SrcPort = ClientPORT;
			//buffer1.DestPort = RouterPORT;
			//buffer1.flag += SYN;//设置SYN
			//buffer1.SeqNum = global_seq;//设置序号seq
			//buffer1.setCheck();//设置校验和
			int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
			buffer1start = clock();
			if (sendByte == 0)
			{
				cout << "连接失败......关闭连接！" << endl;
				return false;
			}
		}
	}

	//=============发送第三次握手的消息（ACK=1，seq=x+1）============
	buffer3.SrcPort = ClientPORT;
	buffer3.DestPort = RouterPORT;
	buffer3.flag += ACK;//设置ACK
	buffer3.SeqNum = ++global_seq;//设置序号seq=x+1
	buffer3.setCheck();//设置校验和
	sendByte = sendto(clientSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&serverAddr, AddrLen);
	if (sendByte == 0)
	{
		cout << "连接失败......关闭连接！" << endl;
		return false;
	}
	cout << "client已发送第三次握手的消息！" << endl;
	cout << "client连接成功！" << endl;

}


bool CloseConnectWithServer(SOCKET clientSocket, SOCKADDR_IN serverAddr)
{
	int AddrLen = sizeof(serverAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;
	Message buffer4;

	//=============发送第一次挥手的消息（FIN=1，seq=y）============
	buffer1.SrcPort = ClientPORT;
	buffer1.DestPort = RouterPORT;
	buffer1.flag += FIN;//设置FIN
	buffer1.SeqNum = ++global_seq;//设置序号seq
	buffer1.setCheck();//设置校验和
	int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
	clock_t buffer1start = clock();
	if (sendByte == 0)
	{
		cout << "连接失败......关闭连接！" << endl;
		return false;
	}
	cout << "client已发送第一次挥手的消息！" << endl;

	//=============接收第二次挥手的消息（ACK=1，ack=y）============
	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "关闭连接error！" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//成功收到消息，检查校验和、ACK、ack
			if ((buffer2.flag && ACK) && buffer2.check() && (buffer2.AckNum == buffer1.SeqNum))
			{
				cout << "client已收到第二次挥手的消息！" << endl;
				break;
			}
			else
			{
				cout << "连接发生错误！" << endl;
				return false;
			}
		}
		//buffer1超时，重新发送并重新计时
		if (clock() - buffer1start > MAX_WAIT_TIME)
		{
			cout << "第一次挥手超时，正在重传......" << endl;
			//============重传buffer1==============
			//buffer1.SrcPort = ClientPORT;
			//buffer1.DestPort = RouterPORT;
			//buffer1.flag += FIN;//设置FIN
			//buffer1.SeqNum = global_seq;//设置序号seq【这里不能++了】
			//buffer1.setCheck();//设置校验和
			int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
			buffer1start = clock();
			if (sendByte == 0)
			{
				cout << "关闭连接error！" << endl;
				return false;
			}
		}
	}


	//=============接收第三次挥手的消息（FIN=1，ACK=1，seq=z）============
	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "关闭连接error！" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//成功收到消息，检查校验和、ACK、ack
			if ((buffer3.flag && ACK)&& (buffer3.flag && FIN) && buffer3.check())
			{
				cout << "client已收到第三次挥手的消息！" << endl;
				break;
			}
			else
			{
				cout << "连接发生错误！" << endl;
				return false;
			}
		}
	}
	

	
	//=============发送第四次挥手的消息（ACK=1，ack=z）============
	buffer4.SrcPort = ClientPORT;
	buffer4.DestPort = RouterPORT;
	buffer4.flag += ACK;//设置ACK
	buffer4.AckNum = buffer3.SeqNum;//设置序号seq
	buffer4.setCheck();//设置校验和
	sendByte = sendto(clientSocket, (char*)&buffer4, sizeof(buffer4), 0, (sockaddr*)&serverAddr, AddrLen);
	if (sendByte == 0)
	{
		cout << "关闭连接error！" << endl;
		return false;
	}
	cout << "client已发送第四次挥手的消息！" << endl;

	


	//============第四次挥手之后还需等待2MSL，防止最后一个ACK丢失，处于半关闭=======
	int tempclock = clock();
	cout << "client端2MSL等待..." << endl;
	Message tmp;
	while (clock() - tempclock < 2 * MAX_WAIT_TIME)
	{
		int recvByte = recvfrom(clientSocket, (char*)&tmp, sizeof(tmp), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "关闭连接error！" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//回复丢失的ack
			//buffer4.SrcPort = ClientPORT;
			//buffer4.DestPort = RouterPORT;
			//buffer4.flag += ACK;//设置ACK
			//buffer4.AckNum = buffer3.SeqNum;//设置序号seq
			//buffer4.setCheck();//设置校验和
			sendByte = sendto(clientSocket, (char*)&buffer4, sizeof(buffer4), 0, (sockaddr*)&serverAddr, AddrLen);
			cout << "回复" << endl;
		}
	}

	
	cout << "\n关闭连接成功！" << endl;
}








bool sendMessage(Message& sendMsg, SOCKET clientSocket, SOCKADDR_IN serverAddr)
{
	sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	cout << "client已发送 Seq = " << sendMsg.SeqNum << " 的报文段！" << endl;
	int msgStart = clock();
	Message recvMsg;
	int AddrLen = sizeof(serverAddr);
	int timeOutTimes = 0;

	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			//成功收到消息，检查校验和、ack
			if ((recvMsg.flag && ACK) && (recvMsg.AckNum == sendMsg.SeqNum))
			{
				cout << "client已收到 Ack = " << recvMsg.AckNum << "的确认报文" << endl;
				return true;
			}
			//若校验失败或ack不对，则忽略，继续等待
		}
		//超时，重新发送并重新计时
		if (clock() - msgStart > MAX_WAIT_TIME)
		{
			cout << "Seq = "<<sendMsg.SeqNum << "的报文段 第" << ++timeOutTimes << "次超时，正在重传......" << endl;
			sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
			msgStart = clock();
		}
		if (timeOutTimes == MAX_SEND_TIMES)
		{
			cout << "超时重传超过" << MAX_SEND_TIMES << "次，传输失败！" << endl;
			break;
		}
	}
	return false;

}



void clientSendFunction(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
{
	int startTime = clock();


	//=================先截取文件名（删除无用路径）
	string realname = "";
	for (int i = filename.size() - 1; i >= 0; i--)
	{
		if (filename[i] == '/' || filename[i] == '\\')
			break;
		realname += filename[i];
	}
	realname = string(realname.rbegin(), realname.rend());



	//=================打开文件，读成字节流=================
	ifstream fin(filename.c_str(), ifstream::binary);
	if (!fin) {
		printf("无法打开文件！\n");
		return;
	}
	//文件读取到fileBuffer
	BYTE* fileBuffer = new BYTE[MaxFileSize];
	unsigned int fileSize = 0;
	BYTE byte = fin.get();
	while (fin) {
		fileBuffer[fileSize++] = byte;
		byte = fin.get();
	}
	fin.close();


	//=========================发送文件名和文件大小=================
	Message nameMessage;
	nameMessage.SrcPort = ClientPORT;
	nameMessage.DestPort = RouterPORT;
	nameMessage.size = fileSize;
	nameMessage.flag += isName;
	nameMessage.SeqNum = ++global_seq;
	for (int i = 0; i < realname.size(); i++)//填充报文数据段
		nameMessage.msgData[i] = realname[i];
	nameMessage.msgData[realname.size()] = '\0';//字符串结尾补\0
	nameMessage.setCheck();
	if (!sendMessage(nameMessage, clientSocket, serverAddr))
	{
		cout << "发送失败！" << endl;
		return;
	}
	cout << "成功发送文件名和文件大小！" << endl;

	//=============================发送数据部分=================
	int batchNum = fileSize / MaxMsgSize;//全装满的报文个数
	int leftSize = fileSize % MaxMsgSize;//不能装满的剩余报文大小
	for (int i = 0; i < batchNum; i++)
	{
		Message dataMsg;
		dataMsg.SrcPort = ClientPORT;
		dataMsg.DestPort = RouterPORT;
		dataMsg.SeqNum = ++global_seq;
		for (int j = 0; j < MaxMsgSize; j++)
		{
			dataMsg.msgData[j] = fileBuffer[i * MaxMsgSize + j];
		}
		dataMsg.setCheck();
		if (!sendMessage(dataMsg, clientSocket, serverAddr))
		{
			cout << "发送失败！" << endl;
			return;
		}
		cout << "成功发送第 " << i << " 个最大装载报文段" << endl;
	}
	//剩余部分
	if (leftSize > 0)
	{
		Message dataMsg;
		dataMsg.SrcPort = ClientPORT;
		dataMsg.DestPort = RouterPORT;
		dataMsg.SeqNum = ++global_seq;
		for (int j = 0; j < leftSize; j++)
		{
			dataMsg.msgData[j] = fileBuffer[batchNum * MaxMsgSize + j];
		}
		dataMsg.setCheck();
		if (!sendMessage(dataMsg, clientSocket, serverAddr))
		{
			cout << "发送失败！" << endl;
			return;
		}
		cout << "成功发送剩余部分的报文段" << endl;
	}

	//计算传输时间和吞吐率
	int endTime = clock();
	cout << "\n\n总体传输时间为:" << (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	cout << "吞吐率:" << ((float)fileSize) / ((endTime - startTime) / CLOCKS_PER_SEC) << "byte/s" << endl << endl;

	return;
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
	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
	unsigned long on = 1;
	ioctlsocket(clientSocket, FIONBIO, &on);//设置非阻塞
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "创建socket：fail！\n" << endl;
		return -1;
	}
	cout << "创建socket：success！\n" << endl;


	//=====================初始化服务器/路由器地址=====================
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET; //地址类型
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //地址
	serverAddr.sin_port = htons(RouterPORT); //端口号
	//=====================初始化客户端地址=====================
	SOCKADDR_IN clientAddr;
	clientAddr.sin_family = AF_INET; //地址类型
	clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //地址
	clientAddr.sin_port = htons(ClientPORT); //端口号
	bind(clientSocket, (LPSOCKADDR)&clientAddr, sizeof(clientAddr));
	//=====================建立连接=====================
	bool isConn = ConnectWithServer(clientSocket, serverAddr);
	if (isConn == 0)
		return -1;
	while (isConn)
	{
		int choice;
		cout << "请输入您的选择：\n"
			<< "（终止连接——0		传输文件——1）" << endl;
		cin >> choice;
		if (choice == 1)
		{
			string filename;
			cout << "请输入文件路径：" << endl;
			cin >> filename;

			clientSendFunction(filename, serverAddr, clientSocket);
		}
		else
		{
			isConn = false;//退出循环
		}
	}

	cout << "关闭连接..." << endl;
	CloseConnectWithServer(clientSocket, serverAddr);

	system("pause");

}