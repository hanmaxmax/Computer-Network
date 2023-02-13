#pragma once
#include <iostream>
using namespace std;


#define MAX_WAIT_TIME  5000 //超时时限 
#define MAX_SEND_TIMES  1000 //超时时限 
#define MaxFileSize 200000000 //最大文件大小
#define MaxMsgSize 10000 //最大文件大小

const unsigned short SYN = 0x1;
const unsigned short ACK = 0x2;
const unsigned short FIN = 0x4;
const unsigned short isName = 0x8;


#pragma pack(1)//让编译器将结构体数据强制连续排列，禁止优化存储结构进行对齐
struct Message
{
	//====================首部（28字节 = 224bit = 14*16bit）=================
	//源IP、目的IP
	unsigned int SrcIP, DestIP;//4字节、4字节
	//源端口号、目的端口号
	unsigned short SrcPort, DestPort;//2字节、2字节
	//序号 Seq num
	unsigned int SeqNum;//4字节
	//确认号 Ack num
	unsigned int AckNum;//4字节
	//数据大小
	unsigned int size;//4字节
	//标志
	unsigned short flag;//2字节
	//校验和
	unsigned short checkNum;//2字节

	//======================报文数据（MaxMsgSize字节）===================
	BYTE msgData[MaxMsgSize];


	Message();
	bool check();
	void setCheck();

};

#pragma pack()

Message::Message()
{
	SrcIP = 0;
	DestIP = 0;//由于测试时，一直是本地回环地址，故这两个字段没有用到
	SeqNum = 0;
	AckNum = 0;
	size = 0;
	flag = 0;
	memset(&msgData, 0, sizeof(msgData));
}

/*
 * 校验和填充全0
 * 剩余数据部分填充全0
 * （首部+数据）每16位求和得到一个32位的数
	如果这个32位的数，高16位不为0，则高16位加低16位再得到一个32位的数；
	重复直到高16位为0
 * 将低16位取反，得到校验和
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
 * （首部+数据）每16位求和得到一个32位的数
	如果这个32位的数，高16位不为0，则高16位加低16位再得到一个32位的数；
	重复直到高16位为0
 * 若最终结果的低16位全1，则校验成功
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



