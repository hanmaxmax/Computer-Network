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

const int RouterPORT = 30000; //·�����˿ں�
const int ClientPORT = 20000; //client�˿ں�


int global_seq = 0;

int CWND = 1;//���ڴ�С
int ssthresh = 50;

int base = 0;
int nextseqnum = 0;
int msgStart;
bool over = 0;
bool sendAgain = 0;
int status = 0;

int congestionControl = 0;
int ResendNum = 0;

//ʵ��client�����Ͷˣ�����������
bool ConnectWithServer(SOCKET clientSocket, SOCKADDR_IN serverAddr)
{
	int AddrLen = sizeof(serverAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;

	//=============���͵�һ�����ֵ���Ϣ��SYN=1��seq=x��============
	buffer1.SrcPort = ClientPORT;
	buffer1.DestPort = RouterPORT;
	buffer1.flag += SYN;//����SYN
	buffer1.SeqNum = ++global_seq;//�������seq
	buffer1.setCheck();//����У���
	int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
	clock_t buffer1start = clock();
	if (sendByte == 0)
	{
		cout << "����ʧ��......�ر����ӣ�" << endl;
		return false;
	}
	cout << "client�ѷ��͵�һ�����ֵ���Ϣ��" << endl;

	//=============���յڶ������ֵ���Ϣ��SYN=1��ACK=1��ack=x��============
	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "����ʧ��......�ر����ӣ�" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//�ɹ��յ���Ϣ�����У��͡�ACK��SYN��ack
			if ((buffer2.flag && ACK) && (buffer2.flag && SYN) && buffer2.check() && (buffer2.AckNum == buffer1.SeqNum))
			{
				cout << "client���յ��ڶ������ֵ���Ϣ��" << endl;
				break;
			}
			else
			{
				cout << "���ӷ�������" << endl;
				return false;
			}
		}
		//buffer1��ʱ�����·��Ͳ����¼�ʱ
		if (clock() - buffer1start > MAX_WAIT_TIME)
		{
			cout << "��һ�����ֳ�ʱ�������ش�......" << endl;
			//============�ش�buffer1==============
			//buffer1.SrcPort = ClientPORT;
			//buffer1.DestPort = RouterPORT;
			//buffer1.flag += SYN;//����SYN
			//buffer1.SeqNum = global_seq;//�������seq
			//buffer1.setCheck();//����У���
			int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
			buffer1start = clock();
			if (sendByte == 0)
			{
				cout << "����ʧ��......�ر����ӣ�" << endl;
				return false;
			}
		}
	}

	//=============���͵��������ֵ���Ϣ��ACK=1��seq=x+1��============
	buffer3.SrcPort = ClientPORT;
	buffer3.DestPort = RouterPORT;
	buffer3.flag += ACK;//����ACK
	buffer3.SeqNum = ++global_seq;//�������seq=x+1
	buffer3.setCheck();//����У���
	sendByte = sendto(clientSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&serverAddr, AddrLen);
	if (sendByte == 0)
	{
		cout << "����ʧ��......�ر����ӣ�" << endl;
		return false;
	}
	cout << "client�ѷ��͵��������ֵ���Ϣ��" << endl;
	cout << "client���ӳɹ���" << endl;

}

//ʵ��client�����Ͷˣ����Ĵλ���
bool CloseConnectWithServer(SOCKET clientSocket, SOCKADDR_IN serverAddr)
{
	int AddrLen = sizeof(serverAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;
	Message buffer4;

	//=============���͵�һ�λ��ֵ���Ϣ��FIN=1��seq=y��============
	buffer1.SrcPort = ClientPORT;
	buffer1.DestPort = RouterPORT;
	buffer1.flag += FIN;//����FIN
	buffer1.SeqNum = ++global_seq;//�������seq
	buffer1.setCheck();//����У���
	int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
	clock_t buffer1start = clock();
	if (sendByte == 0)
	{
		cout << "����ʧ��......�ر����ӣ�" << endl;
		return false;
	}
	cout << "client�ѷ��͵�һ�λ��ֵ���Ϣ��" << endl;

	//=============���յڶ��λ��ֵ���Ϣ��ACK=1��ack=y��============
	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "�ر�����error��" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//�ɹ��յ���Ϣ�����У��͡�ACK��ack
			if ((buffer2.flag && ACK) && buffer2.check() && (buffer2.AckNum == buffer1.SeqNum))
			{
				cout << "client���յ��ڶ��λ��ֵ���Ϣ��" << endl;
				break;
			}
			else
			{
				cout << "���ӷ�������" << endl;
				return false;
			}
		}
		//buffer1��ʱ�����·��Ͳ����¼�ʱ
		if (clock() - buffer1start > MAX_WAIT_TIME)
		{
			cout << "��һ�λ��ֳ�ʱ�������ش�......" << endl;
			//============�ش�buffer1==============
			//buffer1.SrcPort = ClientPORT;
			//buffer1.DestPort = RouterPORT;
			//buffer1.flag += FIN;//����FIN
			//buffer1.SeqNum = global_seq;//�������seq�����ﲻ��++�ˡ�
			//buffer1.setCheck();//����У���
			int sendByte = sendto(clientSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&serverAddr, AddrLen);
			buffer1start = clock();
			if (sendByte == 0)
			{
				cout << "�ر�����error��" << endl;
				return false;
			}
		}
	}


	//=============���յ����λ��ֵ���Ϣ��FIN=1��ACK=1��seq=z��============
	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "�ر�����error��" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//�ɹ��յ���Ϣ�����У��͡�ACK��ack
			if ((buffer3.flag && ACK) && (buffer3.flag && FIN) && buffer3.check())
			{
				cout << "client���յ������λ��ֵ���Ϣ��" << endl;
				break;
			}
			else
			{
				cout << "���ӷ�������" << endl;
				return false;
			}
		}
	}



	//=============���͵��Ĵλ��ֵ���Ϣ��ACK=1��ack=z��============
	buffer4.SrcPort = ClientPORT;
	buffer4.DestPort = RouterPORT;
	buffer4.flag += ACK;//����ACK
	buffer4.AckNum = buffer3.SeqNum;//�������seq
	buffer4.setCheck();//����У���
	sendByte = sendto(clientSocket, (char*)&buffer4, sizeof(buffer4), 0, (sockaddr*)&serverAddr, AddrLen);
	if (sendByte == 0)
	{
		cout << "�ر�����error��" << endl;
		return false;
	}
	cout << "client�ѷ��͵��Ĵλ��ֵ���Ϣ��" << endl;




	//============���Ĵλ���֮����ȴ�2MSL����ֹ���һ��ACK��ʧ�����ڰ�ر�=======
	int tempclock = clock();
	cout << "client��2MSL�ȴ�..." << endl;
	Message tmp;
	while (clock() - tempclock < 2 * MAX_WAIT_TIME)
	{
		int recvByte = recvfrom(clientSocket, (char*)&tmp, sizeof(tmp), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "�ر�����error��" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//�ظ���ʧ��ack
			//buffer4.SrcPort = ClientPORT;
			//buffer4.DestPort = RouterPORT;
			//buffer4.flag += ACK;//����ACK
			//buffer4.AckNum = buffer3.SeqNum;//�������seq
			//buffer4.setCheck();//����У���
			sendByte = sendto(clientSocket, (char*)&buffer4, sizeof(buffer4), 0, (sockaddr*)&serverAddr, AddrLen);
			cout << "�ظ�" << endl;
		}
	}


	cout << "\n�ر����ӳɹ���" << endl;
}


bool sendMessage(Message& sendMsg, SOCKET clientSocket, SOCKADDR_IN serverAddr)
{
	sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	cout << "client�ѷ��� Seq = " << sendMsg.SeqNum << " �ı��ĶΣ�" << endl;
	int msgStart = clock();
	Message recvMsg;
	int AddrLen = sizeof(serverAddr);
	int timeOutTimes = 0;

	while (1)
	{
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			//�ɹ��յ���Ϣ�����У��͡�ack
			if ((recvMsg.flag && ACK) && (recvMsg.AckNum == sendMsg.SeqNum))
			{
				cout << "client���յ� Ack = " << recvMsg.AckNum << "��ȷ�ϱ���" << endl;
				return true;
			}
			//��У��ʧ�ܻ�ack���ԣ�����ԣ������ȴ�
		}
		//��ʱ�����·��Ͳ����¼�ʱ
		if (clock() - msgStart > MAX_WAIT_TIME)
		{
			cout << "Seq = " << sendMsg.SeqNum << "�ı��Ķ� ��" << ++timeOutTimes << "�γ�ʱ�������ش�......" << endl;
			sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
			msgStart = clock();
		}
		if (timeOutTimes == MAX_SEND_TIMES)
		{
			cout << "��ʱ�ش�����" << MAX_SEND_TIMES << "�Σ�����ʧ�ܣ�" << endl;
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


//����ack���߳�
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
			//cout << "================recv ack��" << recvMsg.AckNum << endl;
			//�ɹ��յ���Ϣ����notcorrupt
			if (recvMsg.check())
			{
				if (recvMsg.AckNum >= base)
					base = recvMsg.AckNum + 1;

				if (base != nextseqnum)
					msgStart = clock();
				cout << "client���յ���Ack = " << recvMsg.AckNum << "����ȷ�ϱ���" << endl;

				//��ӡ�������
				cout << "����ǰ��������� �����ܴ�С��" << CWND << "���ѷ��͵�δ�յ�ACK��" << nextseqnum - base
					<< "����δ���ͣ�" << CWND - (nextseqnum - base) << "\n";

				//�жϽ��������
				if (recvMsg.AckNum == msgSum - 1)
				{
					cout << "\nover------------------" << endl;
					over = 1;
					return 0;
				}

				////�����ش�
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
				//	//�ط�
				//	sendAgain = 1;
				//}

				//===========================NEW RENOӵ������================================
				switch (status)
				{
				case 0://������

					cout << "��������������ǰ����������״̬" << endl;

					//cout << "wrongACK = " << wrongACK << "��wrongCount = " << wrongCount << endl;

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


					if (wrongCount == 3)//������ٻָ�
					{
						//�ط�
						sendAgain = 1;//ȫ�ֱ���
						status = 2;
						ssthresh = (CWND + 1) / 2;//����ȡ��
						CWND = ssthresh + 3;

						cout << "\n======================================"
							<< "\n......������ٻָ�״̬\n"
							<< "ssthresh Ϊ " << ssthresh
							<< "��CWNDΪ " << CWND << endl
							<< "======================================\n\n";
						ResendNum = nextseqnum - 1;
						break;
					}

					else if (CWND >= ssthresh)//����ӵ������
					{
						congestionControl = 0;
						status = 1;
						cout << "\n======================================"
							<< "\n......����ӵ������״̬\n"
							<< "ssthresh Ϊ " << ssthresh
							<< "��CWNDΪ " << CWND << endl
							<< "======================================\n\n";

					}

					break;



				case 1://ӵ������

					cout << "��������������ǰ����ӵ������״̬��" << endl;

					//cout << "wrongACK = " << wrongACK << "��wrongCount = " << wrongCount << endl;


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

					if (wrongCount == 3)//������ٻָ�
					{
						//�ط�
						sendAgain = 1;//ȫ�ֱ���
						status = 2;
						ssthresh = (CWND + 1) / 2;
						CWND = ssthresh + 3;
						cout << "\n======================================"
							<< "\n......������ٻָ�״̬\n"
							<< "ssthresh Ϊ " << ssthresh
							<< "��CWNDΪ " << CWND << endl
							<< "======================================\n\n";
						ResendNum = nextseqnum - 1;
					}

					break;


				case 2://���ٻָ�

					cout << "��������������ǰ���ڿ��ٻָ�״̬��" << endl;

					//cout << "wrongACK = " << wrongACK << "��wrongCount = " << wrongCount << endl;


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
					if (recvMsg.AckNum >= ResendNum)//����ӵ������
					{
						congestionControl = 0;
						CWND = ssthresh;
						status = 1;
						cout << "\n======================================"
							<< "\n......����ӵ������״̬\n"
							<< "ssthresh Ϊ " << ssthresh
							<< "��CWNDΪ " << CWND << endl
							<< "======================================\n\n";
					}
					break;
				}


			}
			//��У��ʧ�ܣ�����ԣ������ȴ�

		}

	}
	return 0;
}



void clientSendFunction_GBN(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
{
	int startTime = clock();


	//=================�Ƚ�ȡ�ļ�����ɾ������·����
	string realname = "";
	for (int i = filename.size() - 1; i >= 0; i--)
	{
		if (filename[i] == '/' || filename[i] == '\\')
			break;
		realname += filename[i];
	}
	realname = string(realname.rbegin(), realname.rend());



	//=================���ļ��������ֽ���=================
	ifstream fin(filename.c_str(), ifstream::binary);
	if (!fin) {
		printf("�޷����ļ���\n");
		return;
	}
	//�ļ���ȡ��fileBuffer
	BYTE* fileBuffer = new BYTE[MaxFileSize];
	unsigned int fileSize = 0;
	BYTE byte = fin.get();
	while (fin) {
		fileBuffer[fileSize++] = byte;
		byte = fin.get();
	}
	fin.close();
	int batchNum = fileSize / MaxMsgSize;//ȫװ���ı��ĸ���
	int leftSize = fileSize % MaxMsgSize;//����װ����ʣ�౨�Ĵ�С



	////////////////////////////////////////////////////////////////
	//int base = 0;
	//int nextseqnum = 0;
	//int count = 0;

	//queue<Message> messageBuffer;
	//int msgStart;
	//int AddrLen = sizeof(serverAddr);

	//=================== ����������Ϣ�߳� =====================

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
				for (int i = 0; i < realname.size(); i++)//��䱨�����ݶ�
					sendMsg.msgData[i] = realname[i];
				sendMsg.msgData[realname.size()] = '\0';//�ַ�����β��\0
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
			//messageBuffer.push(sendMsg);//���뷢�Ͷ˻�����

			//send_pkt
			sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
			cout << "client�ѷ��͡�Seq = " << sendMsg.SeqNum << "���ı��ĶΣ�" << endl;

			if (base == nextseqnum)
			{
				msgStart = clock();
			}
			nextseqnum++;
			//��ӡ�������
			cout << "����ǰ��������� �����ܴ�С��" << CWND << "���ѷ��͵�δ�յ�ACK��" << nextseqnum - base
				<< "����δ���ͣ�" << CWND - (nextseqnum - base) << "\n";
		}

		//timeout �� �����ش�
		if (clock() - msgStart > MAX_WAIT_TIME || sendAgain)
		{
			//��ʱ������������״̬
			if (clock() - msgStart > MAX_WAIT_TIME)
			{
				ssthresh = (CWND + 1) / 2;//����ȡ��
				CWND = 1;
				status = 0;
				cout << "\n======================================"
					<< "\n......����������״̬\n"
					<< "ssthresh Ϊ " << ssthresh
					<< "��CWNDΪ " << CWND << endl
					<< "======================================\n\n";
			}


			if (sendAgain)
				cout << "�����յ���������ACK�������ش�......" << endl;
			//�ط���ǰ��������message
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
					for (int i = 0; i < realname.size(); i++)//��䱨�����ݶ�
						sendMsg.msgData[i] = realname[i];
					sendMsg.msgData[realname.size()] = '\0';//�ַ�����β��\0
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
				cout << "Seq = " << sendMsg.SeqNum << "�ı��Ķ��ѳ�ʱ�������ش�......" << endl;

			}
			msgStart = clock();
			sendAgain = 0;
		}

		if (over == 1)//���յ�����ack
		{
			break;
		}
	}

	CloseHandle(hThread);
	cout << "\n\n�ѷ��Ͳ�ȷ�����б��ģ��ļ�����ɹ���\n\n";

	//���㴫��ʱ���������
	int endTime = clock();
	cout << "\n\n���崫��ʱ��Ϊ:" << (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	cout << "������:" << ((float)fileSize) / ((endTime - startTime) / CLOCKS_PER_SEC) << "byte/s" << endl << endl;

}



void test(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
{
	int AddrLen = sizeof(serverAddr);
	int num1 = 0;
	int num2 = 1000;

	unsigned long on = 0;
	unsigned long off = 1;
	ioctlsocket(clientSocket, FIONBIO, &on);//��������

	while (1)
	{
		Message sendMsg;
		sendMsg.SeqNum = num1++;
		sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
		cout << "��1��client�ѷ��� Seq = " << sendMsg.SeqNum << " �ı��ĶΣ�" << endl;


		ioctlsocket(clientSocket, FIONBIO, &off);//���÷�����
		//rdt_rcv
		Message recvMsg;
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			cout << "client���յ� Ack = ��" << recvMsg.AckNum << "����ȷ�ϱ���" << endl;
		}

		ioctlsocket(clientSocket, FIONBIO, &on);//��������


		if (num1 % 50 == 10)
		{
			for (int i = 0; i < 100; i++)
			{
				Message sendMsg;
				sendMsg.SeqNum = num2++;
				sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
				cout << "��2��client�ѷ��� Seq = " << sendMsg.SeqNum << " �ı��ĶΣ�" << endl;
			}

		}

	}



}



void clientSendFunction_2(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
{
	int startTime = clock();


	//=================�Ƚ�ȡ�ļ�����ɾ������·����
	string realname = "";
	for (int i = filename.size() - 1; i >= 0; i--)
	{
		if (filename[i] == '/' || filename[i] == '\\')
			break;
		realname += filename[i];
	}
	realname = string(realname.rbegin(), realname.rend());



	//=================���ļ��������ֽ���=================
	ifstream fin(filename.c_str(), ifstream::binary);
	if (!fin) {
		printf("�޷����ļ���\n");
		return;
	}
	//�ļ���ȡ��fileBuffer
	BYTE* fileBuffer = new BYTE[MaxFileSize];
	unsigned int fileSize = 0;
	BYTE byte = fin.get();
	while (fin) {
		fileBuffer[fileSize++] = byte;
		byte = fin.get();
	}
	fin.close();
	int batchNum = fileSize / MaxMsgSize;//ȫװ���ı��ĸ���
	int leftSize = fileSize % MaxMsgSize;//����װ����ʣ�౨�Ĵ�С



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
				for (int i = 0; i < realname.size(); i++)//��䱨�����ݶ�
					sendMsg.msgData[i] = realname[i];
				sendMsg.msgData[realname.size()] = '\0';//�ַ�����β��\0
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
			messageBuffer.push(sendMsg);//���뷢�Ͷ˻�����

			//send_pkt
			sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
			cout << "client�ѷ��� Seq = " << sendMsg.SeqNum << " �ı��ĶΣ�" << endl;

			if (base == nextseqnum)
			{
				msgStart = clock();
			}
			nextseqnum++;
			//��ӡ�������
			cout << "����ǰ��������� �����ܴ�С��" << CWND << "���ѷ��͵�δ�յ�ACK��" << messageBuffer.size()
				<< "����δ���ͣ�" << CWND - messageBuffer.size() << "\n";

		}

		//rdt_rcv
		Message recvMsg;
		int recvByte = recvfrom(clientSocket, (char*)&recvMsg, sizeof(recvMsg), 0, (sockaddr*)&serverAddr, &AddrLen);
		if (recvByte > 0)
		{
			cout << "================recv ack��" << recvMsg.AckNum << endl;
			//�ɹ��յ���Ϣ����notcorrupt
			if (recvMsg.check())
			{
				base = recvMsg.AckNum + 1;
				if (base != nextseqnum)
					msgStart = clock();
				for (int k = 0; k < base - messageBuffer.front().SeqNum; k++)
				{
					messageBuffer.pop();//�ӻ�������popһ���������൱�ڻ�����������
				}
				cout << "client���յ� Ack = " << recvMsg.AckNum << "��ȷ�ϱ���" << endl;

				//��ӡ�������
				cout << "����ǰ��������� �����ܴ�С��" << CWND << "���ѷ��͵�δ�յ�ACK��" << messageBuffer.size()
					<< "����δ���ͣ�" << CWND - messageBuffer.size() << "\n";
			}
			//��У��ʧ�ܻ�ack���ԣ�����ԣ������ȴ�
		}

		//Sleep(1500);

		//timeout
		if (clock() - msgStart > MAX_WAIT_TIME)
		{
			int qSize = messageBuffer.size();
			//�ط���ǰ��������message
			for (int i = 0; i < qSize; i++)
			{
				Message sendMsg = messageBuffer.front();
				messageBuffer.pop();
				messageBuffer.push(sendMsg);
				sendto(clientSocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
				cout << "Seq = " << sendMsg.SeqNum << "�ı��Ķ��ѳ�ʱ�������ش�......" << endl;
			}
			msgStart = clock();
		}

	}
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
	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
	unsigned long on = 1;
	ioctlsocket(clientSocket, FIONBIO, &on);//���÷�����
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "����socket��fail��\n" << endl;
		return -1;
	}
	cout << "����socket��success��\n" << endl;


	//=====================��ʼ��������/·������ַ=====================
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET; //��ַ����
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //��ַ
	serverAddr.sin_port = htons(RouterPORT); //�˿ں�
	//=====================��ʼ���ͻ��˵�ַ=====================
	SOCKADDR_IN clientAddr;
	clientAddr.sin_family = AF_INET; //��ַ����
	clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //��ַ
	clientAddr.sin_port = htons(ClientPORT); //�˿ں�
	bind(clientSocket, (LPSOCKADDR)&clientAddr, sizeof(clientAddr));
	//=====================��������=====================
	bool isConn = ConnectWithServer(clientSocket, serverAddr);
	if (isConn == 0)
		return -1;
	while (isConn)
	{
		int choice;
		cout << "����������ѡ��\n"
			<< "����ֹ���ӡ���0		�����ļ�����1��" << endl;
		cin >> choice;
		if (choice == 1)
		{
			string filename;
			cout << "�������ļ�·����" << endl;
			cin >> filename;

			clientSendFunction_GBN(filename, serverAddr, clientSocket);
			//test(filename, serverAddr, clientSocket);
		}
		else
		{
			isConn = false;//�˳�ѭ��
		}
	}

	cout << "�ر�����..." << endl;
	CloseConnectWithServer(clientSocket, serverAddr);

	system("pause");

}