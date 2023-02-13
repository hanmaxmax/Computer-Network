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

const int RouterPORT = 30000; //·�����˿ں�
const int ClientPORT = 20000; //client�˿ں�


int global_seq = 0;



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
			if ((buffer3.flag && ACK)&& (buffer3.flag && FIN) && buffer3.check())
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
			cout << "Seq = "<<sendMsg.SeqNum << "�ı��Ķ� ��" << ++timeOutTimes << "�γ�ʱ�������ش�......" << endl;
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



void clientSendFunction(string filename, SOCKADDR_IN serverAddr, SOCKET clientSocket)
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


	//=========================�����ļ������ļ���С=================
	Message nameMessage;
	nameMessage.SrcPort = ClientPORT;
	nameMessage.DestPort = RouterPORT;
	nameMessage.size = fileSize;
	nameMessage.flag += isName;
	nameMessage.SeqNum = ++global_seq;
	for (int i = 0; i < realname.size(); i++)//��䱨�����ݶ�
		nameMessage.msgData[i] = realname[i];
	nameMessage.msgData[realname.size()] = '\0';//�ַ�����β��\0
	nameMessage.setCheck();
	if (!sendMessage(nameMessage, clientSocket, serverAddr))
	{
		cout << "����ʧ�ܣ�" << endl;
		return;
	}
	cout << "�ɹ������ļ������ļ���С��" << endl;

	//=============================�������ݲ���=================
	int batchNum = fileSize / MaxMsgSize;//ȫװ���ı��ĸ���
	int leftSize = fileSize % MaxMsgSize;//����װ����ʣ�౨�Ĵ�С
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
			cout << "����ʧ�ܣ�" << endl;
			return;
		}
		cout << "�ɹ����͵� " << i << " �����װ�ر��Ķ�" << endl;
	}
	//ʣ�ಿ��
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
			cout << "����ʧ�ܣ�" << endl;
			return;
		}
		cout << "�ɹ�����ʣ�ಿ�ֵı��Ķ�" << endl;
	}

	//���㴫��ʱ���������
	int endTime = clock();
	cout << "\n\n���崫��ʱ��Ϊ:" << (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	cout << "������:" << ((float)fileSize) / ((endTime - startTime) / CLOCKS_PER_SEC) << "byte/s" << endl << endl;

	return;
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

			clientSendFunction(filename, serverAddr, clientSocket);
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