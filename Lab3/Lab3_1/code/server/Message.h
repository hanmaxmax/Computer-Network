#pragma once
#include <iostream>
using namespace std;


#define MAX_WAIT_TIME  5000 //��ʱʱ�� 
#define MAX_SEND_TIMES  1000 //��ʱʱ�� 
#define MaxFileSize 200000000 //����ļ���С
#define MaxMsgSize 10000 //����ļ���С

const unsigned short SYN = 0x1;
const unsigned short ACK = 0x2;
const unsigned short FIN = 0x4;
const unsigned short isName = 0x8;


#pragma pack(1)//�ñ��������ṹ������ǿ���������У���ֹ�Ż��洢�ṹ���ж���
struct Message
{
	//====================�ײ���28�ֽ� = 224bit = 14*16bit��=================
	//ԴIP��Ŀ��IP
	unsigned int SrcIP, DestIP;//4�ֽڡ�4�ֽ�
	//Դ�˿ںš�Ŀ�Ķ˿ں�
	unsigned short SrcPort, DestPort;//2�ֽڡ�2�ֽ�
	//��� Seq num
	unsigned int SeqNum;//4�ֽ�
	//ȷ�Ϻ� Ack num
	unsigned int AckNum;//4�ֽ�
	//���ݴ�С
	unsigned int size;//4�ֽ�
	//��־
	unsigned short flag;//2�ֽ�
	//У���
	unsigned short checkNum;//2�ֽ�

	//======================�������ݣ�MaxMsgSize�ֽڣ�===================
	BYTE msgData[MaxMsgSize];


	Message();
	bool check();
	void setCheck();

};

#pragma pack()

Message::Message()
{
	SrcIP = 0;
	DestIP = 0;//���ڲ���ʱ��һֱ�Ǳ��ػػ���ַ�����������ֶ�û���õ�
	SeqNum = 0;
	AckNum = 0;
	size = 0;
	flag = 0;
	memset(&msgData, 0, sizeof(msgData));
}

/*
 * У������ȫ0
 * ʣ�����ݲ������ȫ0
 * ���ײ�+���ݣ�ÿ16λ��͵õ�һ��32λ����
	������32λ��������16λ��Ϊ0�����16λ�ӵ�16λ�ٵõ�һ��32λ������
	�ظ�ֱ����16λΪ0
 * ����16λȡ�����õ�У���
*/
void Message::setCheck()
{
	this->checkNum = 0;
	int sum = 0;
	unsigned short* msgStream = (unsigned short*)this;


	for (int i = 0; i < sizeof(*this) / 2; i++)
	{
		sum += *msgStream++;
		if (sum & 0xFFFF0000)
		{
			sum &= 0xFFFF;
			sum++;
		}
	}
	this->checkNum = ~(sum & 0xFFFF);

}

/*
 * ���ײ�+���ݣ�ÿ16λ��͵õ�һ��32λ����
	������32λ��������16λ��Ϊ0�����16λ�ӵ�16λ�ٵõ�һ��32λ������
	�ظ�ֱ����16λΪ0
 * �����ս���ĵ�16λȫ1����У��ɹ�
*/
bool Message::check()
{

	unsigned int sum = 0;
	unsigned short* msgStream = (unsigned short*)this;

	for (int i = 0; i < sizeof(*this) / 2; i++)
	{
		sum += *msgStream++;
		if (sum & 0xFFFF0000)
		{
			sum &= 0xFFFF;
			sum++;
		}
	}
	if ((sum & 0xFFFF) == 0xFFFF)
	{
		return true;
	}
	return false;

}



