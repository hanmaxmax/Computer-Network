#include <iostream>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fstream>
#pragma comment (lib, "ws2_32.lib")
#include "Message.h"
using namespace std;

const int ServerPORT = 10000; //server端口号
const int RouterPORT = 30000; //路由器端口号




int global_seq = 0;


//实现server（接收端）的三次握手
bool ConnectWithClient(SOCKET serverSocket, SOCKADDR_IN clientAddr)
{

	int AddrLen = sizeof(clientAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;
	while (1)
	{
		//=============接收第一次握手的消息（SYN=1，seq=x）============
		int recvByte = recvfrom(serverSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "连接失败......关闭连接！" << endl;
			return false;
		}

		else if (recvByte > 0)
		{
			//判断SYN、检验和
			if (!(buffer1.flag && SYN) || !buffer1.check() || !(buffer1.SeqNum == global_seq + 1))
			{
				////`````````````````````````````````````````````````
				//if (buffer1.check() && (buffer1.SeqNum != global_seq + 1))
				//{
				//	//回复ACK
				//	Message replyMessage;
				//	replyMessage.SrcPort = ServerPORT;
				//	replyMessage.DestPort = RouterPORT;
				//	replyMessage.flag += ACK;
				//	replyMessage.AckNum = buffer1.SeqNum;
				//	replyMessage.setCheck();
				//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				//	cout << "【重复接收报文段】server收到 Seq = " << buffer1.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
				//}
				//else
				//{
				cout << "连接发生错误！" << endl;
				return false;
				//}

			}
			global_seq++;
			cout << "server已收到第一次握手的消息！" << endl;
			//=============发送第二次握手的消息（SYN=1，ACK=1，ack=x）============
			buffer2.SrcPort = ServerPORT;
			buffer2.DestPort = RouterPORT;
			buffer2.AckNum = buffer1.SeqNum;//服务器回复的ack=客户端发来的seq
			buffer2.flag += SYN;
			buffer2.flag += ACK;
			buffer2.setCheck();//设置校验和
			int sendByte = sendto(serverSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&clientAddr, AddrLen);
			clock_t buffer2start = clock();
			if (sendByte == 0)
			{
				cout << "连接失败......关闭连接！" << endl;
				return false;
			}
			cout << "server已发送第二次握手的消息！" << endl;

			//=============接收第三次握手的消息（ACK=1，seq=x+1）============
			while (1)
			{
				int recvByte = recvfrom(serverSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&clientAddr, &AddrLen);
				if (recvByte == 0)
				{
					cout << "连接失败......关闭连接！" << endl;
					return false;
				}
				else if (recvByte > 0)
				{
					//成功收到消息，检查校验和、seq
					if ((buffer3.flag && ACK) && buffer3.check() && (buffer3.SeqNum == global_seq + 1))
					{
						global_seq++;
						cout << "server已收到第三次握手的消息！" << endl;
						cout << "server连接成功！" << endl;
						return true;
					}
					else
					{
						////`````````````````````````````````````````````````
						//cout << "global_seq"<< global_seq << endl;
						//if (buffer3.check() && (buffer3.SeqNum != global_seq + 1))
						//{
						//	//回复ACK
						//	Message replyMessage;
						//	replyMessage.SrcPort = ServerPORT;
						//	replyMessage.DestPort = RouterPORT;
						//	replyMessage.flag += ACK;
						//	replyMessage.AckNum = buffer3.SeqNum;
						//	replyMessage.setCheck();
						//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
						//	cout << "【重复接收报文段】server收到 Seq = " << buffer3.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
						//}
						//else
						//{
						//	cout << "连接发生错误！" << endl;
						//	return false;
						//}
						cout << "连接发生错误！" << endl;
						return false;
					}
				}

				//buffer2超时，重新发送并重新计时
				if (clock() - buffer2start > MAX_WAIT_TIME)
				{
					cout << "第二次握手超时，正在重传......" << endl;
					//============重传buffer2==============
					//buffer2.SrcPort = ServerPORT;
					//buffer2.DestPort = RouterPORT;
					//buffer2.AckNum = buffer1.SeqNum;//服务器回复的ack=客户端发来的seq
					//buffer2.setCheck();//设置校验和
					int sendByte = sendto(serverSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&clientAddr, AddrLen);
					buffer2start = clock(); //重新设置buffer2的时间
					if (sendByte == 0)
					{
						cout << "连接失败......关闭连接！" << endl;
						return false;
					}
				}

			}
		}
	}
	return false;
}


//实现server（接收端）的四次挥手
bool CloseConnectWithClient(SOCKET serverSocket, SOCKADDR_IN clientAddr)
{
	int AddrLen = sizeof(clientAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;
	Message buffer4;
	while (1)
	{
		//=============接收第一次挥手的消息（FIN=1，seq=y）============
		int recvByte = recvfrom(serverSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "连接失败......关闭连接！" << endl;
			return false;
		}

		else if (recvByte > 0)
		{
			//判断SYN、检验和
			if (!(buffer1.flag && FIN) || !buffer1.check() || !(buffer1.SeqNum == global_seq + 1))
			{
				////`````````````````````````````````````````````````
				//if (buffer1.check() && (buffer1.SeqNum != global_seq + 1))
				//{
				//	//回复ACK
				//	Message replyMessage;
				//	replyMessage.SrcPort = ServerPORT;
				//	replyMessage.DestPort = RouterPORT;
				//	replyMessage.flag += ACK;
				//	replyMessage.AckNum = buffer1.SeqNum;
				//	replyMessage.setCheck();
				//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				//	cout << "【重复接收报文段】server收到 Seq = " << buffer1.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
				//}
				//else
				//{
				cout << "连接发生错误！" << endl;
				return false;
				//}

			}
			global_seq++;
			cout << "server已收到第一次挥手的消息！" << endl;

			//=============发送第二次挥手的消息（ACK=1，ack=y）============
			buffer2.SrcPort = ServerPORT;
			buffer2.DestPort = RouterPORT;
			buffer2.AckNum = buffer1.SeqNum;
			buffer2.flag += ACK;
			buffer2.setCheck();//设置校验和
			int sendByte = sendto(serverSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&clientAddr, AddrLen);
			clock_t buffer2start = clock();
			if (sendByte == 0)
			{
				cout << "连接失败......关闭连接！" << endl;
				return false;
			}
			cout << "server已发送第二次挥手的消息！" << endl;
			break;

		}
	}


	//=============发送第三次挥手的消息（FIN=1，ACK=1，seq=z）============
	buffer3.SrcPort = ServerPORT;
	buffer3.DestPort = RouterPORT;
	buffer3.flag += FIN;//设置FIN
	buffer3.flag += ACK;//设置ACK
	buffer3.SeqNum = global_seq++;//设置序号seq
	buffer3.setCheck();//设置校验和
	int sendByte = sendto(serverSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&clientAddr, AddrLen);
	clock_t buffer3start = clock();
	if (sendByte == 0)
	{
		cout << "连接失败......关闭连接！" << endl;
		return false;
	}
	cout << "server已发送第三次挥手的消息！" << endl;


	//=============接收第四次挥手的消息（ACK=1，ack=z）============
	while (1)
	{
		int recvByte = recvfrom(serverSocket, (char*)&buffer4, sizeof(buffer4), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "关闭连接error！" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//成功收到消息，检查校验和、ACK、ack
			if ((buffer4.flag && ACK) && buffer4.check() && (buffer4.AckNum == buffer3.SeqNum))
			{
				cout << "server已收到第四次挥手的消息！" << endl;
				break;
			}
			else
			{
				////`````````````````````````````````````````````````
				//if (buffer4.check() && (buffer4.AckNum != buffer3.SeqNum))
				//{
				//	//回复ACK
				//	Message replyMessage;
				//	replyMessage.SrcPort = ServerPORT;
				//	replyMessage.DestPort = RouterPORT;
				//	replyMessage.flag += ACK;
				//	replyMessage.AckNum = buffer1.SeqNum;
				//	replyMessage.setCheck();
				//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				//	cout << "【重复接收报文段】server收到 Seq = " << buffer1.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
				//}
				//else
				//{
				cout << "连接发生错误！" << endl;
				return false;
				//}
			}
		}
		//buffer3超时，重新发送并重新计时
		if (clock() - buffer3start > MAX_WAIT_TIME)
		{
			cout << "第三次挥手超时，正在重传......" << endl;
			//============重传buffer3==============
			//buffer3.SrcPort = ServerPORT;
			//buffer3.DestPort = RouterPORT;
			//buffer3.flag += FIN;//设置FIN
			//buffer3.flag += ACK;//设置ACK
			//buffer3.SeqNum = global_seq;//设置序号seq
			//buffer3.setCheck();//设置校验和
			int sendByte = sendto(serverSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&clientAddr, AddrLen);
			buffer3start = clock();
			if (sendByte == 0)
			{
				cout << "关闭连接error！" << endl;
				return false;
			}
		}
	}

	cout << "\n关闭连接成功！" << endl;
	return true;
}











bool recvMessage(Message& recvMsg, SOCKET serverSocket, SOCKADDR_IN clientAddr, int& expectedseqnum)
{

	int AddrLen = sizeof(clientAddr);
	while (1)
	{
		int recvByte = recvfrom(serverSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte > 0)
		{
			//成功收到消息
			if (recvMsg.check() && (recvMsg.SeqNum == expectedseqnum))
			{
				//回复ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = recvMsg.SeqNum;
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "server收到 Seq = " << recvMsg.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
				expectedseqnum++;
				return true;
			}

			//如果seq！= 期待值，则返回累计确认的ack（expectedseqnum-1）
			else if (recvMsg.check() && (recvMsg.SeqNum != expectedseqnum))
			{
				//回复ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = expectedseqnum - 1;
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "【累计确认（失序）】server收到 Seq = " << recvMsg.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
			}
		}
		else if (recvByte == 0)
		{
			return false;
		}

	}

}




void serverRecvFunction(SOCKET serverSocket, SOCKADDR_IN clientAddr)
{
	int expectedseqnum = 0;
	int AddrLen = sizeof(clientAddr);
	//======================接收文件名和文件大小=====================
	Message nameMessage;
	unsigned int fileSize;//文件大小
	char fileName[50] = { 0 };//文件名
	while (1)
	{
		int recvByte = recvfrom(serverSocket, (char*)&nameMessage, sizeof(nameMessage), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte > 0)
		{
			//成功收到消息且为expectedseqnum
			if (nameMessage.check() && (nameMessage.SeqNum == expectedseqnum))
			{
				fileSize = nameMessage.size;//获取文件大小
				for (int i = 0; nameMessage.msgData[i]; i++)//获取文件名
					fileName[i] = nameMessage.msgData[i];
				cout << "\n接收文件名为：" << fileName << "，大小为：" << fileSize << endl << endl;

				//回复ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = nameMessage.SeqNum;//确认expectedseqnum
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "server收到 Seq = " << nameMessage.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
				expectedseqnum++;
				break;
			}

			//如果seq！= 期待值，则返回累计确认的ack（expectedseqnum-1）
			else if (nameMessage.check() && (nameMessage.SeqNum != expectedseqnum))
			{
				//回复ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = expectedseqnum - 1;//累计确认
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "【累计确认（失序）】server收到 Seq = " << nameMessage.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
			}
		}
	}

	//==========================接收数据段==================================
	int batchNum = fileSize / MaxMsgSize;//全装满的报文个数
	int leftSize = fileSize % MaxMsgSize;//不能装满的剩余报文大小
	BYTE* fileBuffer = new BYTE[fileSize];

	cout << "开始接收数据段，共 " << batchNum << " 个最大装载报文段\n" << endl;

	for (int i = 0; i < batchNum; i++)
	{
		Message dataMsg;

		if (recvMessage(dataMsg, serverSocket, clientAddr, expectedseqnum))
		{
			cout << "数据报" << dataMsg.SeqNum << "接收成功" << endl;
		}
		else
		{
			cout << "数据接收失败！" << endl;
			return;
		}
		//读取数据部分
		for (int j = 0; j < MaxMsgSize; j++)
		{
			fileBuffer[i * MaxMsgSize + j] = dataMsg.msgData[j];
		}

	}

	//剩余部分
	if (leftSize > 0)
	{
		Message dataMsg;
		if (recvMessage(dataMsg, serverSocket, clientAddr, expectedseqnum))
		{
			cout << "数据报" << dataMsg.SeqNum << "接收成功" << endl;
		}
		else
		{
			cout << "数据接收失败！" << endl;
			return;
		}
		for (int j = 0; j < leftSize; j++)
		{
			fileBuffer[batchNum * MaxMsgSize + j] = dataMsg.msgData[j];
		}

	}

	cout << "\n\n文件传输成功，正在写入文件......" << endl;
	//写入文件
	FILE* outFile;
	outFile = fopen(fileName, "wb");
	if (fileBuffer != 0)
	{
		fwrite(fileBuffer, fileSize, 1, outFile);
		fclose(outFile);
		cout << "\n文件写入成功！" << endl;

	}
	return;

}












void test(SOCKET serverSocket, SOCKADDR_IN clientAddr)
{
	Message recvMsg;
	int AddrLen = sizeof(clientAddr);
	while (1)
	{
		int recvByte = recvfrom(serverSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte > 0)
		{
			//回复ACK
			Message replyMessage;
			replyMessage.AckNum = recvMsg.SeqNum;
			sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
			cout << "server收到 Seq = " << recvMsg.SeqNum << "的报文段，并发送 Ack = " << replyMessage.AckNum << " 的回复报文段" << endl;
		}
	}

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
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	unsigned long on = 1;
	ioctlsocket(serverSocket, FIONBIO, &on);//设置非阻塞
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
	serverAddr.sin_port = htons(ServerPORT); //端口号
	int tem = bind(serverSocket, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (tem == SOCKET_ERROR)
	{
		cout << "bind：fail！\n" << endl;
		return -1;
	}
	else
	{
		cout << "bind to port " << ServerPORT << "：success！\n" << endl;
	}
	cout << "Server ready！Waiting for client request..." << endl << endl;

	//=====================初始化客户端/路由器地址=====================
	SOCKADDR_IN clientAddr;
	clientAddr.sin_family = AF_INET; //地址类型
	clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //地址
	clientAddr.sin_port = htons(RouterPORT); //端口号

	//=====================建立连接=====================
	bool isConn = ConnectWithClient(serverSocket, clientAddr);
	if (isConn == 0)
		return -1;

	//=====================传输文件=====================
	serverRecvFunction(serverSocket, clientAddr);

	//======================关闭连接====================
	CloseConnectWithClient(serverSocket, clientAddr);





	//test(serverSocket, clientAddr);



	closesocket(serverSocket); //关闭socket
	WSACleanup();
	system("pause");

	return 0;
}

