#ifndef _STRUCT_H
#define _STRUCT_H

#include <stdio.h>

/*
**********************************************************************************************************
*									Status Code - 状态码
**********************************************************************************************************
*/

//进程运行状态
#define READY 1
#define RUN 2
#define BLOCK 3

//资源状态
#define FREE 1
#define ALLOCATE 2

//优先级
#define INIT 0
#define USER 1
#define SYSTEM 2

#define PCBLIST_SIZE 10 //最大容纳PCB数量
#define PRIO_NUM 3		//优先级数量
#define RESTYPE_NUM 4	//资源类型数量

/*
*********************************************************************************************************
*										Struct - 结构体
*********************************************************************************************************
*/

//进程 - PCB
typedef struct pcb
{
	int ID;	   //进程ID
	char *PID; //进程名称
	int Prio;  //优先级
	int Type;  //运行状态

	struct pcb *Pre;  //指向同优先级就绪队列前一个进程
	struct pcb *Next; //指向同优先级就绪队列后一个进程

	struct pcb *Parent;	 //指向父进程
	struct pcb **Childs; //指向多个子进程

	struct res *OcpResList[RESTYPE_NUM]; //已占用资源队列
	int OcpListCount[RESTYPE_NUM];		 //已占用资源计数
	int RequestCount[RESTYPE_NUM];		 //用于进程阻塞时,已请求资源计数
} PCB;

//资源 - RES
typedef struct res
{
	int ID;		//资源ID
	int Status; //资源状态

	struct res *Next; //用于占用资源队列,获取下个被占用资源
} RES;

//资源管理器 - RCB
typedef struct rcb
{
	int ID;	   //资源ID,与对应的单个资源ID相同
	char *RID; //资源名称

	int FreeCount; //空闲计数

	struct res **ResList;	//资源队列,存放对应的多个资源
	struct pcb **BlockList; //阻塞队列,记录阻塞与该资源的PCB
} RCB;

#endif