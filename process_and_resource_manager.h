#ifndef _PROCESS_AND_RESOURCE_MANAGER_H
#define _PROCESS_AND_RESOURCE_MANAGER_H

#include "struct.h"

/*
*********************************************************************************************************
*                                   Global Variable -  全局变量
*********************************************************************************************************
*/

extern int current_id;             //当前运行进程id
extern char *PID[PCBLIST_SIZE];    //进程名称,与PCBList中顺序一一对应
extern PCB *PCBList[PCBLIST_SIZE]; //PCB进程队列
extern PCB *RL[PRIO_NUM];          //存储就绪队列对应优先级的指针
extern RCB *RCBList[RESTYPE_NUM];  //资源进程管理器队列

/*
*********************************************************************************************************
*                                       Function -  函数
*********************************************************************************************************
*/

//初始化
extern void Init(void);
extern void InitPCBList(void);
extern void InitReadyList(void);
extern void InitRCBList(void);

//进程操作
extern void CreatePCB(char *pid, int prio);
extern void DestoryPCB(char *pid);
extern void KillPCBTree(void);
extern void Timeout(void);
extern void Insert(PCB *rl, PCB *pcb);
extern void Request(char *rid, int n);
extern void Release(char *rid, int n);
extern void BlockPCB(PCB *pcb, RCB *rcb, int n);
extern void AllocateRes(PCB *pcb, RCB *rcb, int n);
extern void ReleaseRes(PCB *pcb, RCB *rcb, int n);
extern void UpdateBlockList(PCB **blocks);
extern void Scheduler(void);

//进程数据显示
extern void ListAllPCB(void);
extern void ListAllRES(void);
extern void ShowInform(char *pid);

//卸载函数
extern void Uninstaller(void);

#endif