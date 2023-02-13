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

const int ServerPORT = 10000; //server�˿ں�
const int RouterPORT = 30000; //·�����˿ں�




int global_seq = 0;


//ʵ��server�����նˣ�����������
bool ConnectWithClient(SOCKET serverSocket, SOCKADDR_IN clientAddr)
{

	int AddrLen = sizeof(clientAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;
	while (1)
	{
		//=============���յ�һ�����ֵ���Ϣ��SYN=1��seq=x��============
		int recvByte = recvfrom(serverSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "����ʧ��......�ر����ӣ�" << endl;
			return false;
		}

		else if (recvByte > 0)
		{
			//�ж�SYN�������
			if (!(buffer1.flag && SYN) || !buffer1.check() || !(buffer1.SeqNum == global_seq + 1))
			{
				////`````````````````````````````````````````````````
				//if (buffer1.check() && (buffer1.SeqNum != global_seq + 1))
				//{
				//	//�ظ�ACK
				//	Message replyMessage;
				//	replyMessage.SrcPort = ServerPORT;
				//	replyMessage.DestPort = RouterPORT;
				//	replyMessage.flag += ACK;
				//	replyMessage.AckNum = buffer1.SeqNum;
				//	replyMessage.setCheck();
				//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				//	cout << "���ظ����ձ��ĶΡ�server�յ� Seq = " << buffer1.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
				//}
				//else
				//{
				cout << "���ӷ�������" << endl;
				return false;
				//}

			}
			global_seq++;
			cout << "server���յ���һ�����ֵ���Ϣ��" << endl;
			//=============���͵ڶ������ֵ���Ϣ��SYN=1��ACK=1��ack=x��============
			buffer2.SrcPort = ServerPORT;
			buffer2.DestPort = RouterPORT;
			buffer2.AckNum = buffer1.SeqNum;//�������ظ���ack=�ͻ��˷�����seq
			buffer2.flag += SYN;
			buffer2.flag += ACK;
			buffer2.setCheck();//����У���
			int sendByte = sendto(serverSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&clientAddr, AddrLen);
			clock_t buffer2start = clock();
			if (sendByte == 0)
			{
				cout << "����ʧ��......�ر����ӣ�" << endl;
				return false;
			}
			cout << "server�ѷ��͵ڶ������ֵ���Ϣ��" << endl;

			//=============���յ��������ֵ���Ϣ��ACK=1��seq=x+1��============
			while (1)
			{
				int recvByte = recvfrom(serverSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&clientAddr, &AddrLen);
				if (recvByte == 0)
				{
					cout << "����ʧ��......�ر����ӣ�" << endl;
					return false;
				}
				else if (recvByte > 0)
				{
					//�ɹ��յ���Ϣ�����У��͡�seq
					if ((buffer3.flag && ACK) && buffer3.check() && (buffer3.SeqNum == global_seq + 1))
					{
						global_seq++;
						cout << "server���յ����������ֵ���Ϣ��" << endl;
						cout << "server���ӳɹ���" << endl;
						return true;
					}
					else
					{
						////`````````````````````````````````````````````````
						//cout << "global_seq"<< global_seq << endl;
						//if (buffer3.check() && (buffer3.SeqNum != global_seq + 1))
						//{
						//	//�ظ�ACK
						//	Message replyMessage;
						//	replyMessage.SrcPort = ServerPORT;
						//	replyMessage.DestPort = RouterPORT;
						//	replyMessage.flag += ACK;
						//	replyMessage.AckNum = buffer3.SeqNum;
						//	replyMessage.setCheck();
						//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
						//	cout << "���ظ����ձ��ĶΡ�server�յ� Seq = " << buffer3.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
						//}
						//else
						//{
						//	cout << "���ӷ�������" << endl;
						//	return false;
						//}
						cout << "���ӷ�������" << endl;
						return false;
					}
				}

				//buffer2��ʱ�����·��Ͳ����¼�ʱ
				if (clock() - buffer2start > MAX_WAIT_TIME)
				{
					cout << "�ڶ������ֳ�ʱ�������ش�......" << endl;
					//============�ش�buffer2==============
					//buffer2.SrcPort = ServerPORT;
					//buffer2.DestPort = RouterPORT;
					//buffer2.AckNum = buffer1.SeqNum;//�������ظ���ack=�ͻ��˷�����seq
					//buffer2.setCheck();//����У���
					int sendByte = sendto(serverSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&clientAddr, AddrLen);
					buffer2start = clock(); //��������buffer2��ʱ��
					if (sendByte == 0)
					{
						cout << "����ʧ��......�ر����ӣ�" << endl;
						return false;
					}
				}

			}
		}
	}
	return false;
}


//ʵ��server�����նˣ����Ĵλ���
bool CloseConnectWithClient(SOCKET serverSocket, SOCKADDR_IN clientAddr)
{
	int AddrLen = sizeof(clientAddr);
	Message buffer1;
	Message buffer2;
	Message buffer3;
	Message buffer4;
	while (1)
	{
		//=============���յ�һ�λ��ֵ���Ϣ��FIN=1��seq=y��============
		int recvByte = recvfrom(serverSocket, (char*)&buffer1, sizeof(buffer1), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "����ʧ��......�ر����ӣ�" << endl;
			return false;
		}

		else if (recvByte > 0)
		{
			//�ж�SYN�������
			if (!(buffer1.flag && FIN) || !buffer1.check() || !(buffer1.SeqNum == global_seq + 1))
			{
				////`````````````````````````````````````````````````
				//if (buffer1.check() && (buffer1.SeqNum != global_seq + 1))
				//{
				//	//�ظ�ACK
				//	Message replyMessage;
				//	replyMessage.SrcPort = ServerPORT;
				//	replyMessage.DestPort = RouterPORT;
				//	replyMessage.flag += ACK;
				//	replyMessage.AckNum = buffer1.SeqNum;
				//	replyMessage.setCheck();
				//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				//	cout << "���ظ����ձ��ĶΡ�server�յ� Seq = " << buffer1.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
				//}
				//else
				//{
				cout << "���ӷ�������" << endl;
				return false;
				//}

			}
			global_seq++;
			cout << "server���յ���һ�λ��ֵ���Ϣ��" << endl;

			//=============���͵ڶ��λ��ֵ���Ϣ��ACK=1��ack=y��============
			buffer2.SrcPort = ServerPORT;
			buffer2.DestPort = RouterPORT;
			buffer2.AckNum = buffer1.SeqNum;
			buffer2.flag += ACK;
			buffer2.setCheck();//����У���
			int sendByte = sendto(serverSocket, (char*)&buffer2, sizeof(buffer2), 0, (sockaddr*)&clientAddr, AddrLen);
			clock_t buffer2start = clock();
			if (sendByte == 0)
			{
				cout << "����ʧ��......�ر����ӣ�" << endl;
				return false;
			}
			cout << "server�ѷ��͵ڶ��λ��ֵ���Ϣ��" << endl;
			break;

		}
	}


	//=============���͵����λ��ֵ���Ϣ��FIN=1��ACK=1��seq=z��============
	buffer3.SrcPort = ServerPORT;
	buffer3.DestPort = RouterPORT;
	buffer3.flag += FIN;//����FIN
	buffer3.flag += ACK;//����ACK
	buffer3.SeqNum = global_seq++;//�������seq
	buffer3.setCheck();//����У���
	int sendByte = sendto(serverSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&clientAddr, AddrLen);
	clock_t buffer3start = clock();
	if (sendByte == 0)
	{
		cout << "����ʧ��......�ر����ӣ�" << endl;
		return false;
	}
	cout << "server�ѷ��͵����λ��ֵ���Ϣ��" << endl;


	//=============���յ��Ĵλ��ֵ���Ϣ��ACK=1��ack=z��============
	while (1)
	{
		int recvByte = recvfrom(serverSocket, (char*)&buffer4, sizeof(buffer4), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte == 0)
		{
			cout << "�ر�����error��" << endl;
			return false;
		}
		else if (recvByte > 0)
		{
			//�ɹ��յ���Ϣ�����У��͡�ACK��ack
			if ((buffer4.flag && ACK) && buffer4.check() && (buffer4.AckNum == buffer3.SeqNum))
			{
				cout << "server���յ����Ĵλ��ֵ���Ϣ��" << endl;
				break;
			}
			else
			{
				////`````````````````````````````````````````````````
				//if (buffer4.check() && (buffer4.AckNum != buffer3.SeqNum))
				//{
				//	//�ظ�ACK
				//	Message replyMessage;
				//	replyMessage.SrcPort = ServerPORT;
				//	replyMessage.DestPort = RouterPORT;
				//	replyMessage.flag += ACK;
				//	replyMessage.AckNum = buffer1.SeqNum;
				//	replyMessage.setCheck();
				//	sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				//	cout << "���ظ����ձ��ĶΡ�server�յ� Seq = " << buffer1.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
				//}
				//else
				//{
				cout << "���ӷ�������" << endl;
				return false;
				//}
			}
		}
		//buffer3��ʱ�����·��Ͳ����¼�ʱ
		if (clock() - buffer3start > MAX_WAIT_TIME)
		{
			cout << "�����λ��ֳ�ʱ�������ش�......" << endl;
			//============�ش�buffer3==============
			//buffer3.SrcPort = ServerPORT;
			//buffer3.DestPort = RouterPORT;
			//buffer3.flag += FIN;//����FIN
			//buffer3.flag += ACK;//����ACK
			//buffer3.SeqNum = global_seq;//�������seq
			//buffer3.setCheck();//����У���
			int sendByte = sendto(serverSocket, (char*)&buffer3, sizeof(buffer3), 0, (sockaddr*)&clientAddr, AddrLen);
			buffer3start = clock();
			if (sendByte == 0)
			{
				cout << "�ر�����error��" << endl;
				return false;
			}
		}
	}

	cout << "\n�ر����ӳɹ���" << endl;
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
			//�ɹ��յ���Ϣ
			if (recvMsg.check() && (recvMsg.SeqNum == expectedseqnum))
			{
				//�ظ�ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = recvMsg.SeqNum;
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "server�յ� Seq = " << recvMsg.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
				expectedseqnum++;
				return true;
			}

			//���seq��= �ڴ�ֵ���򷵻��ۼ�ȷ�ϵ�ack��expectedseqnum-1��
			else if (recvMsg.check() && (recvMsg.SeqNum != expectedseqnum))
			{
				//�ظ�ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = expectedseqnum - 1;
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "���ۼ�ȷ�ϣ�ʧ�򣩡�server�յ� Seq = " << recvMsg.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
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
	//======================�����ļ������ļ���С=====================
	Message nameMessage;
	unsigned int fileSize;//�ļ���С
	char fileName[50] = { 0 };//�ļ���
	while (1)
	{
		int recvByte = recvfrom(serverSocket, (char*)&nameMessage, sizeof(nameMessage), 0, (sockaddr*)&clientAddr, &AddrLen);
		if (recvByte > 0)
		{
			//�ɹ��յ���Ϣ��Ϊexpectedseqnum
			if (nameMessage.check() && (nameMessage.SeqNum == expectedseqnum))
			{
				fileSize = nameMessage.size;//��ȡ�ļ���С
				for (int i = 0; nameMessage.msgData[i]; i++)//��ȡ�ļ���
					fileName[i] = nameMessage.msgData[i];
				cout << "\n�����ļ���Ϊ��" << fileName << "����СΪ��" << fileSize << endl << endl;

				//�ظ�ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = nameMessage.SeqNum;//ȷ��expectedseqnum
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "server�յ� Seq = " << nameMessage.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
				expectedseqnum++;
				break;
			}

			//���seq��= �ڴ�ֵ���򷵻��ۼ�ȷ�ϵ�ack��expectedseqnum-1��
			else if (nameMessage.check() && (nameMessage.SeqNum != expectedseqnum))
			{
				//�ظ�ACK
				Message replyMessage;
				replyMessage.SrcPort = ServerPORT;
				replyMessage.DestPort = RouterPORT;
				replyMessage.flag += ACK;
				replyMessage.AckNum = expectedseqnum - 1;//�ۼ�ȷ��
				replyMessage.setCheck();
				sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
				cout << "���ۼ�ȷ�ϣ�ʧ�򣩡�server�յ� Seq = " << nameMessage.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
			}
		}
	}

	//==========================�������ݶ�==================================
	int batchNum = fileSize / MaxMsgSize;//ȫװ���ı��ĸ���
	int leftSize = fileSize % MaxMsgSize;//����װ����ʣ�౨�Ĵ�С
	BYTE* fileBuffer = new BYTE[fileSize];

	cout << "��ʼ�������ݶΣ��� " << batchNum << " �����װ�ر��Ķ�\n" << endl;

	for (int i = 0; i < batchNum; i++)
	{
		Message dataMsg;

		if (recvMessage(dataMsg, serverSocket, clientAddr, expectedseqnum))
		{
			cout << "���ݱ�" << dataMsg.SeqNum << "���ճɹ�" << endl;
		}
		else
		{
			cout << "���ݽ���ʧ�ܣ�" << endl;
			return;
		}
		//��ȡ���ݲ���
		for (int j = 0; j < MaxMsgSize; j++)
		{
			fileBuffer[i * MaxMsgSize + j] = dataMsg.msgData[j];
		}

	}

	//ʣ�ಿ��
	if (leftSize > 0)
	{
		Message dataMsg;
		if (recvMessage(dataMsg, serverSocket, clientAddr, expectedseqnum))
		{
			cout << "���ݱ�" << dataMsg.SeqNum << "���ճɹ�" << endl;
		}
		else
		{
			cout << "���ݽ���ʧ�ܣ�" << endl;
			return;
		}
		for (int j = 0; j < leftSize; j++)
		{
			fileBuffer[batchNum * MaxMsgSize + j] = dataMsg.msgData[j];
		}

	}

	cout << "\n\n�ļ�����ɹ�������д���ļ�......" << endl;
	//д���ļ�
	FILE* outFile;
	outFile = fopen(fileName, "wb");
	if (fileBuffer != 0)
	{
		fwrite(fileBuffer, fileSize, 1, outFile);
		fclose(outFile);
		cout << "\n�ļ�д��ɹ���" << endl;

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
			//�ظ�ACK
			Message replyMessage;
			replyMessage.AckNum = recvMsg.SeqNum;
			sendto(serverSocket, (char*)&replyMessage, sizeof(replyMessage), 0, (sockaddr*)&clientAddr, sizeof(SOCKADDR_IN));
			cout << "server�յ� Seq = " << recvMsg.SeqNum << "�ı��ĶΣ������� Ack = " << replyMessage.AckNum << " �Ļظ����Ķ�" << endl;
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
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	unsigned long on = 1;
	ioctlsocket(serverSocket, FIONBIO, &on);//���÷�����
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
	serverAddr.sin_port = htons(ServerPORT); //�˿ں�
	int tem = bind(serverSocket, (LPSOCKADDR)&serverAddr, sizeof(serverAddr));
	if (tem == SOCKET_ERROR)
	{
		cout << "bind��fail��\n" << endl;
		return -1;
	}
	else
	{
		cout << "bind to port " << ServerPORT << "��success��\n" << endl;
	}
	cout << "Server ready��Waiting for client request..." << endl << endl;

	//=====================��ʼ���ͻ���/·������ַ=====================
	SOCKADDR_IN clientAddr;
	clientAddr.sin_family = AF_INET; //��ַ����
	clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //��ַ
	clientAddr.sin_port = htons(RouterPORT); //�˿ں�

	//=====================��������=====================
	bool isConn = ConnectWithClient(serverSocket, clientAddr);
	if (isConn == 0)
		return -1;

	//=====================�����ļ�=====================
	serverRecvFunction(serverSocket, clientAddr);

	//======================�ر�����====================
	CloseConnectWithClient(serverSocket, clientAddr);





	//test(serverSocket, clientAddr);



	closesocket(serverSocket); //�ر�socket
	WSACleanup();
	system("pause");

	return 0;
}

