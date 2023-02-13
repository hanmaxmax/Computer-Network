#include <iostream>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <fstream>
#include <queue>
#pragma comment (lib, "ws2_32.lib")
#include "Message.h"
using namespace std;

const int RouterPORT = 30000; //路由器端口号
const int ClientPORT = 20000; //client端口号


int global_seq = 0;

int CWND = 1;//窗口大小
int ssthresh = 50;

int base = 0;
int nextseqnum = 0;
int msgStart;
bool over = 0;
bool sendAgain = 0;
int status = 0;

int congestionControl = 0;
int ResendNum = 0;

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

//实现client（发送端）的四次挥手
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
			if ((buffer3.flag && ACK) && (buffer3.flag && FIN) && buffer3.check())
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
			cout << "Seq = " << sendMsg.SeqNum << "的报文段 第" << ++timeOutTimes << "次超时，正在重传......" << endl;
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



struct parameters {
	SOCKADDR_IN serverAddr;
	SOCKET clientSocket;
	int msgSum;
};


//接收ack的线程
DWORD WINAPI recvThread(PVOID pParam)
{
	parameters* para = (parameters*)pParam;
	SOCKADDR_IN serverAddr = para->serverAddr;
	SOCKET clientSocket = para->clientSocket;
	int msgSum = para->msgSum;
	int AddrLen = sizeof(serverAddr);

	int wrongACK = -1;
	int wrongCount = 0;


	while (1)
	{
		//rdt_rcv
		Message recvMsg;
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			//cout << "================recv ack：" << recvMsg.AckNum << endl;
			//成功收到消息，且notcorrupt
			if (recvMsg.check())
			{
				if (recvMsg.AckNum >= base)
					base = recvMsg.AckNum + 1;

				if (base != nextseqnum)
					msgStart = clock();
				cout << "client已收到【Ack = " << recvMsg.AckNum << "】的确认报文" << endl;

				//打印窗口情况
				cout << "【当前窗口情况】 窗口总大小：" << CWND << "，已发送但未收到ACK：" << nextseqnum - base
					<< "，尚未发送：" << CWND - (nextseqnum - base) << "\n";

				//判断结束的情况
				if (recvMsg.AckNum == msgSum - 1)
				{
					cout << "\nover------------------" << endl;
					over = 1;
					return 0;
				}

				////快速重传
				//if (wrongACK != recvMsg.AckNum)
				//{
				//	wrongCount = 0;
				//	wrongACK = recvMsg.AckNum;
				//}
				//else
				//{
				//	wrongCount++;
				//}
				//if (wrongCount == 2)
				//{
				//	//重发
				//	sendAgain = 1;
				//}

				//===========================NEW RENO拥塞控制================================
				switch (status)
				{
				case 0://慢启动

					cout << "——————当前处于慢启动状态" << endl;

					//cout << "wrongACK = " << wrongACK << "，wrongCount = " << wrongCount << endl;

					//new ack
					if (wrongACK != recvMsg.AckNum)
					{
						CWND++;
						wrongCount = 0;
						wrongACK = recvMsg.AckNum;
					}
					else //duplicate ack 
					{
						wrongCount++;
					}


					if (wrongCount == 3)//进入快速恢复
					{
						//重发
						sendAgain = 1;//全局变量
						status = 2;
						ssthresh = (CWND + 1) / 2;//向上取整
						CWND = ssthresh + 3;

						cout << "\n======================================"
							<< "\n......进入快速恢复状态\n"
							<< "ssthresh 为 " << ssthresh
							<< "，CWND为 " << CWND << endl
							<< "======================================\n\n";
						ResendNum = nextseqnum - 1;
						break;
					}

					else if (CWND >= ssthresh)//进入拥塞避免
					{
						congestionControl = 0;
						status = 1;
						cout << "\n======================================"
							<< "\n......进入拥塞避免状态\n"
							<< "ssthresh 为 " << ssthresh
							<< "，CWND为 " << CWND << endl
							<< "======================================\n\n";

					}

					break;



				case 1://拥塞避免

					cout << "——————当前处于拥塞避免状态！" << endl;

					//cout << "wrongACK = " << wrongACK << "，wrongCount = " << wrongCount << endl;


					//new ack
					if (wrongACK != recvMsg.AckNum)
					{
						//cout << "congestionControl  " << congestionControl << endl;
						congestionControl++;
						if (congestionControl >= CWND)
						{
							CWND += 1;
							congestionControl = 0;
						}

						wrongCount = 0;
						wrongACK = recvMsg.AckNum;
					}
					else //duplicate ack 
					{
						wrongCount++;
					}

					if (wrongCount == 3)//进入快速恢复
					{
						//重发
						sendAgain = 1;//全局变量
						status = 2;
						ssthresh = (CWND + 1) / 2;
						CWND = ssthresh + 3;
						cout << "\n======================================"
							<< "\n......进入快速恢复状态\n"
							<< "ssthresh 为 " << ssthresh
							<< "，CWND为 " << CWND << endl
							<< "======================================\n\n";
						ResendNum = nextseqnum - 1;
					}

					break;


				case 2://快速恢复

					cout << "——————当前处于快速恢复状态！" << endl;

					//cout << "wrongACK = " << wrongACK << "，wrongCount = " << wrongCount << endl;


					//new ack
					if (wrongACK != recvMsg.AckNum)
					{
						CWND++;
						wrongCount = 0;
						wrongACK = recvMsg.AckNum;
					}
					else //duplicate ack 
					{
						//CWND++;
						wrongCount++;
					}
					if (wrongCount == 3)
					{
						sendAgain = 1;
					}
					//cout << "recvAck = " << recvMsg.AckNum << "Resend = " << ResendNum << endl;
					if (recvMsg.AckNum >= ResendNum)//进入拥塞避免
					{
						congestionControl = 0;
						CWND = ssthresh;
						status = 1;
						cout << "\n======================================"
							<< "\n......进入拥塞避免状态\n"
							<< "ssthresh 为 " << ssthresh
							<< "，CWND为 " << CWND << endl
							<< "======================================\n\n";
					}
					break;
				}


			}
			//若校验失败，则忽略，继续等待

		}

	}
	return 0;
}



void clientSendFunction_GBN(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
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
	int batchNum = fileSize / MaxMsgSize;//全装满的报文个数
	int leftSize = fileSize % MaxMsgSize;//不能装满的剩余报文大小



	////////////////////////////////////////////////////////////////
	//int base = 0;
	//int nextseqnum = 0;
	//int count = 0;

	//queue<Message> messageBuffer;
	//int msgStart;
	//int AddrLen = sizeof(serverAddr);

	//=================== 创建接受消息线程 =====================

	int msgSum = leftSize > 0 ? batchNum + 2 : batchNum + 1;

	parameters param;
	param.serverAddr = serverAddr;
	param.clientSocket = clientSocket;
	param.msgSum = msgSum;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvThread, &param, 0, 0);

	while (1)
	{
		//rdt_send(data)
		if (nextseqnum < base + CWND && nextseqnum < msgSum)
		{
			//make_pkt
			Message sendMsg;
			if (nextseqnum == 0)
			{
				sendMsg.SrcPort = ClientPORT;
				sendMsg.DestPort = RouterPORT;
				sendMsg.size = fileSize;
				sendMsg.flag += isName;
				sendMsg.SeqNum = nextseqnum;
				for (int i = 0; i < realname.size(); i++)//填充报文数据段
					sendMsg.msgData[i] = realname[i];
				sendMsg.msgData[realname.size()] = '\0';//字符串结尾补\0
				sendMsg.setCheck();
			}
			else if (nextseqnum == batchNum + 1 && leftSize > 0)
			{
				sendMsg.SrcPort = ClientPORT;
				sendMsg.DestPort = RouterPORT;
				sendMsg.SeqNum = nextseqnum;
				for (int j = 0; j < leftSize; j++)
				{
					sendMsg.msgData[j] = fileBuffer[batchNum * MaxMsgSize + j];
				}
				sendMsg.setCheck();

			}
			else
			{
				sendMsg.SrcPort = ClientPORT;
				sendMsg.DestPort = RouterPORT;
				sendMsg.SeqNum = nextseqnum;
				for (int j = 0; j < MaxMsgSize; j++)
				{
					sendMsg.msgData[j] = fileBuffer[(nextseqnum - 1) * MaxMsgSize + j];
				}
				sendMsg.setCheck();
			}
			//messageBuffer.push(sendMsg);//放入发送端缓冲区

			//send_pkt
			sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
			cout << "client已发送【Seq = " << sendMsg.SeqNum << "】的报文段！" << endl;

			if (base == nextseqnum)
			{
				msgStart = clock();
			}
			nextseqnum++;
			//打印窗口情况
			cout << "【当前窗口情况】 窗口总大小：" << CWND << "，已发送但未收到ACK：" << nextseqnum - base
				<< "，尚未发送：" << CWND - (nextseqnum - base) << "\n";
		}

		//timeout 或 快速重传
		if (clock() - msgStart > MAX_WAIT_TIME || sendAgain)
		{
			//超时：进入慢启动状态
			if (clock() - msgStart > MAX_WAIT_TIME)
			{
				ssthresh = (CWND + 1) / 2;//向上取整
				CWND = 1;
				status = 0;
				cout << "\n======================================"
					<< "\n......进入慢启动状态\n"
					<< "ssthresh 为 " << ssthresh
					<< "，CWND为 " << CWND << endl
					<< "======================================\n\n";
			}


			if (sendAgain)
				cout << "连续收到三次冗余ACK，快速重传......" << endl;
			//重发当前缓冲区的message
			Message sendMsg;
			for (int i = 0; i < nextseqnum - base; i++)
			{
				int sendnum = base + i;
				if (sendnum == 0)
				{
					sendMsg.SrcPort = ClientPORT;
					sendMsg.DestPort = RouterPORT;
					sendMsg.size = fileSize;
					sendMsg.flag += isName;
					sendMsg.SeqNum = sendnum;
					for (int i = 0; i < realname.size(); i++)//填充报文数据段
						sendMsg.msgData[i] = realname[i];
					sendMsg.msgData[realname.size()] = '\0';//字符串结尾补\0
					sendMsg.setCheck();
				}
				else if (sendnum == batchNum + 1 && leftSize > 0)
				{
					sendMsg.SrcPort = ClientPORT;
					sendMsg.DestPort = RouterPORT;
					sendMsg.SeqNum = sendnum;
					for (int j = 0; j < leftSize; j++)
					{
						sendMsg.msgData[j] = fileBuffer[batchNum * MaxMsgSize + j];
					}
					sendMsg.setCheck();

				}
				else
				{
					sendMsg.SrcPort = ClientPORT;
					sendMsg.DestPort = RouterPORT;
					sendMsg.SeqNum = sendnum;
					for (int j = 0; j < MaxMsgSize; j++)
					{
						sendMsg.msgData[j] = fileBuffer[(sendnum - 1) * MaxMsgSize + j];
					}
					sendMsg.setCheck();
				}

				sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
				cout << "Seq = " << sendMsg.SeqNum << "的报文段已超时，正在重传......" << endl;

			}
			msgStart = clock();
			sendAgain = 0;
		}

		if (over == 1)//已收到所有ack
		{
			break;
		}
	}

	CloseHandle(hThread);
	cout << "\n\n已发送并确认所有报文，文件传输成功！\n\n";

	//计算传输时间和吞吐率
	int endTime = clock();
	cout << "\n\n总体传输时间为:" << (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	cout << "吞吐率:" << ((float)fileSize) / ((endTime - startTime) / CLOCKS_PER_SEC) << "byte/s" << endl << endl;

}



void test(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
{
	int AddrLen = sizeof(serverAddr);
	int num1 = 0;
	int num2 = 1000;

	unsigned long on = 0;
	unsigned long off = 1;
	ioctlsocket(clientSocket, FIONBIO, &on);//设置阻塞

	while (1)
	{
		Message sendMsg;
		sendMsg.SeqNum = num1++;
		sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
		cout << "【1】client已发送 Seq = " << sendMsg.SeqNum << " 的报文段！" << endl;


		ioctlsocket(clientSocket, FIONBIO, &off);//设置非阻塞
		//rdt_rcv
		Message recvMsg;
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			cout << "client已收到 Ack = 【" << recvMsg.AckNum << "】的确认报文" << endl;
		}

		ioctlsocket(clientSocket, FIONBIO, &on);//设置阻塞


		if (num1 % 50 == 10)
		{
			for (int i = 0; i < 100; i++)
			{
				Message sendMsg;
				sendMsg.SeqNum = num2++;
				sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
				cout << "【2】client已发送 Seq = " << sendMsg.SeqNum << " 的报文段！" << endl;
			}

		}

	}



}



void clientSendFunction_2(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
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
	int batchNum = fileSize / MaxMsgSize;//全装满的报文个数
	int leftSize = fileSize % MaxMsgSize;//不能装满的剩余报文大小



	////////////////////////////////////////////////////////////////
	int base = 0;
	int nextseqnum = 0;
	//int count = 0;

	queue<Message> messageBuffer;
	int msgStart;
	int AddrLen = sizeof(serverAddr);


	while (1)
	{
		//rdt_send(data)
		if (nextseqnum < base + CWND && base < batchNum + 2)
		{
			//make_pkt
			Message sendMsg;
			if (nextseqnum == 0)
			{
				sendMsg.SrcPort = ClientPORT;
				sendMsg.DestPort = RouterPORT;
				sendMsg.size = fileSize;
				sendMsg.flag += isName;
				sendMsg.SeqNum = nextseqnum;
				for (int i = 0; i < realname.size(); i++)//填充报文数据段
					sendMsg.msgData[i] = realname[i];
				sendMsg.msgData[realname.size()] = '\0';//字符串结尾补\0
				sendMsg.setCheck();
			}
			else if (nextseqnum == batchNum + 1)
			{
				sendMsg.SrcPort = ClientPORT;
				sendMsg.DestPort = RouterPORT;
				sendMsg.SeqNum = nextseqnum;
				for (int j = 0; j < leftSize; j++)
				{
					sendMsg.msgData[j] = fileBuffer[batchNum * MaxMsgSize + j];
				}
				sendMsg.setCheck();

			}
			else
			{
				sendMsg.SrcPort = ClientPORT;
				sendMsg.DestPort = RouterPORT;
				sendMsg.SeqNum = nextseqnum;
				for (int j = 0; j < MaxMsgSize; j++)
				{
					sendMsg.msgData[j] = fileBuffer[(nextseqnum - 1) * MaxMsgSize + j];
				}
				sendMsg.setCheck();
			}
			messageBuffer.push(sendMsg);//放入发送端缓冲区

			//send_pkt
			sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
			cout << "client已发送 Seq = " << sendMsg.SeqNum << " 的报文段！" << endl;

			if (base == nextseqnum)
			{
				msgStart = clock();
			}
			nextseqnum++;
			//打印窗口情况
			cout << "【当前窗口情况】 窗口总大小：" << CWND << "，已发送但未收到ACK：" << messageBuffer.size()
				<< "，尚未发送：" << CWND - messageBuffer.size() << "\n";

		}

		//rdt_rcv
		Message recvMsg;
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			cout << "================recv ack：" << recvMsg.AckNum << endl;
			//成功收到消息，且notcorrupt
			if (recvMsg.check())
			{
				base = recvMsg.AckNum + 1;
				if (base != nextseqnum)
					msgStart = clock();
				for (int k = 0; k < base - messageBuffer.front().SeqNum; k++)
				{
					messageBuffer.pop();//从缓冲区中pop一个出来，相当于滑动窗口右移
				}
				cout << "client已收到 Ack = " << recvMsg.AckNum << "的确认报文" << endl;

				//打印窗口情况
				cout << "【当前窗口情况】 窗口总大小：" << CWND << "，已发送但未收到ACK：" << messageBuffer.size()
					<< "，尚未发送：" << CWND - messageBuffer.size() << "\n";
			}
			//若校验失败或ack不对，则忽略，继续等待
		}

		//Sleep(1500);

		//timeout
		if (clock() - msgStart > MAX_WAIT_TIME)
		{
			int qSize = messageBuffer.size();
			//重发当前缓冲区的message
			for (int i = 0; i < qSize; i++)
			{
				Message sendMsg = messageBuffer.front();
				messageBuffer.pop();
				messageBuffer.push(sendMsg);
				sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
				cout << "Seq = " << sendMsg.SeqNum << "的报文段已超时，正在重传......" << endl;
			}
			msgStart = clock();
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

			clientSendFunction_GBN(filename, serverAddr, clientSocket);
			//test(filename, serverAddr, clientSocket);
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