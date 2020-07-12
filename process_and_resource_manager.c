#include "process_and_resource_manager.h"
#include "struct.h"

//全局变量定义
int current_id;
char *PID[PCBLIST_SIZE];
PCB *PCBList[PCBLIST_SIZE];
PCB *RL[PRIO_NUM];
RCB *RCBList[RESTYPE_NUM];

/*
*********************************************************************************************************
 *										Init - 初始化
 *
 * 	描述：
 * 		调用InitPCBList(),InitReadyList(),InitRCBList()函数,完成进程,资源,队列等的初始化操作,
 * 		并创建初始化init进程.
 * 
 * 	参数：
 * 		无.
*********************************************************************************************************
*/

void Init(void)
{
	current_id = 0; //指明初始化运行init进程

	InitPCBList(); //初始化PCBList

	InitReadyList(); //初始化就绪队列

	InitRCBList(); //初始化RCB队列

	Insert(RL[0], PCBList[0]); //将init进程插入对应的就绪优先级队列

	Scheduler(); //调度init运行

	printf("init \n");
}

//PCB队列初始化函数
void InitPCBList(void)
{
	PCB *ppcb;

	//为PCBList中的每个PCB申请空间并初始化
	for (int i = 0; i < PCBLIST_SIZE; i++)
	{
		//申请空间并显式初始化
		ppcb = (PCB *)calloc(1, sizeof(PCB));
		ppcb->Childs = (PCB **)calloc(1, sizeof(PCB));
		ppcb->PID = (char *)"?";
		ppcb->ID = i;
		ppcb->Parent = NULL;
		ppcb->Pre = NULL;
		ppcb->Next = NULL;
		ppcb->Type = -1;
		ppcb->Prio = -1;
		for (int j = 0; j < RESTYPE_NUM; j++)
		{
			ppcb->OcpResList[j] = (RES *)calloc(1, sizeof(RES));
			ppcb->OcpListCount[j] = 0;
			ppcb->RequestCount[j] = 0;
		}
		PCBList[i] = ppcb;

		//初始化用来保存PCB名字的PID队列
		PID[i] = (char *)calloc(1, sizeof(char));
		strcpy(PID[i], "?");
	}

	//创建init进程
	strcpy(PID[0], "init");
	PCBList[0]->PID = PID[0];
	PCBList[0]->Prio = 0;
	PCBList[0]->Type = READY;
}

//就绪队列初始化函数
void InitReadyList(void)
{
	PCB *prl;

	//初始化各个就绪优先级队列
	for (int i = 0; i < PRIO_NUM; i++)
	{
		prl = (PCB *)calloc(1, sizeof(PCB));
		RL[i] = prl;
		RL[i]->ID = -1;
	}
}

//RCB队列初始化函数
void InitRCBList(void)
{
	RES *pres, **preses;
	RCB *prcb;

	//初始化各个RCB
	for (int i = 0; i < RESTYPE_NUM; i++)
	{
		prcb = (RCB *)calloc(1, sizeof(RCB));
		prcb->ResList = (RES *)calloc(1, sizeof(RCB));
		prcb->BlockList = (PCB **)calloc(1, sizeof(PCB));
		prcb->RID = "?";
		prcb->ID = i;
		prcb->FreeCount = i + 1; //初始化空闲资源数
		preses = prcb->ResList;	 //获取各个RCB的资源队列

		//将各类资源逐个放入对应的资源队列中
		for (int j = 0; j < prcb->FreeCount; j++)
		{
			//为每个资源申请空间
			pres = (RES *)calloc(1, sizeof(RES));
			pres->ID = prcb->ID;
			pres->Status = FREE;
			*preses = pres;
			preses++;
		}

		RCBList[i] = prcb; //将RCB地址保存到RCB队列中
	}

	//初始化各个RCB名称
	RCBList[0]->RID = "R1";
	RCBList[1]->RID = "R2";
	RCBList[2]->RID = "R3";
	RCBList[3]->RID = "R4";
}

/*
*********************************************************************************************************
 *									CreatePCB - 创建进程
 * 
 * 	描述: 
 * 		创建进程,完成进程初始化,然后将进程插入对应的就绪优先级队列中.
 * 
 * 	参数:
 * 		pid	  被创建进程的进程名称,保存在进程的PID和PID数列中.
 * 
 *      prio  被创建进程的优先级：
 * 			  0			INIT优先级,优先级最低.
 * 			  1			USER优先级.
 * 			  2			SYSTEM优先级,优先级最高.			  
*********************************************************************************************************
*/

void CreatePCB(char *pid, int prio)
{
	PCB *ppcb, **pchilds;
	int k;

	//检查传入PID是否已经存在
	for (int i = 1; i < PCBLIST_SIZE; i++)
	{
		if (strcmp(PID[i], pid) == 0)
		{
			//存在重复PID,报错并退出函数
			printf("PID name already exsited\n");
			return;
		}
	}

	//寻找PCB队列中的空闲下标,创建PCB
	for (k = 0; k < PCBLIST_SIZE; k++)
	{
		if (strcmp(PID[k], "?") == 0)
		{
			ppcb = PCBList[k];
			pchilds = PCBList[current_id]->Childs;
			strcpy(PID[k], pid); //保存PID到PID数组中
			ppcb->PID = PID[k];
			ppcb->Prio = prio;
			ppcb->Parent = PCBList[current_id]; //存储创建PCB的父进程
			while (*pchilds != NULL)
			{
				pchilds++;
			}
			*pchilds = ppcb; //将PCB放入当前运行进程空闲子节点
			ppcb->Type = READY;

			//将节点插入对应的就绪优先级队列
			Insert(RL[prio], ppcb);
			break;
		}
	}

	//进程容量达到最大值，报错并返回
	if(k == PCBLIST_SIZE)
	{
		printf("Can't create more process!\n");
		return;
	}
	
	printf("%s Create: %s , Prio: %d\n",PCBList[current_id]->PID,pid,prio);
	Scheduler();
}

/*
*********************************************************************************************************
 *									Insert - 插入函数
 * 
 * 	描述: 
 * 		将进程插入对应的就绪优先级队列.
 * 
 * 	参数:
 * 		rl   进程对应优先级的就绪优先级队列的头指针.
 * 
 * 		pcb  要插入就绪优先级队列的进程指针.
*********************************************************************************************************
*/

void Insert(PCB *rl, PCB *pcb)
{
	PCB *ppcb; //获取头指针
	ppcb = rl;

	//将进程插入队列尾部
	while (ppcb->Next != NULL)
	{
		ppcb = ppcb->Next;
	}
	ppcb->Next = pcb;
	pcb->Pre = ppcb; //在进程中保存其在队列中的前继节点
}

/*
*********************************************************************************************************
 *									Scheduler - 进度调度
 * 
 * 	描述: 
 * 		将最高优先级的进程从就绪态转换为运行态.
 * 
 * 	参数:
 * 		无.
*********************************************************************************************************
*/
// 		当遇到以下情况时，当前运行进程会被调度：
// 			(1) 就绪队列中存在优先级较高的进程
// 	    	(2) 当前进程被销毁或被阻塞，即不处于运行态
// 			(3) 进程时间片到达
void Scheduler(void)
{
	PCB *ppcb;
	int prio;

	//按优先级高到低遍历优先级队列
	for (int i = PRIO_NUM - 1; i >= 0; i--)
	{
		ppcb = RL[i]; //获取对应优先级头指针

		//当前优先级存在进程
		if (ppcb->Next != NULL)
		{
			ppcb = ppcb->Next; //获取就绪进程
			prio = ppcb->Prio; //获取该进程的优先级

			//就绪进程满足调度条件,进行调度的相关操作
			if (prio > PCBList[current_id]->Prio || PCBList[current_id]->Type != RUN || strcmp(PID[current_id], "?") == 0)
			{

				//防止被销毁被放入就绪队列
				if (prio > PCBList[current_id]->Prio && strcmp(PID[current_id], "?") != 0)
				{
					PCBList[current_id]->Type = READY;
					Insert(RL[PCBList[current_id]->Prio], PCBList[current_id]);
				}

				current_id = ppcb->ID;

				//将进程从就绪优先级队列移除
				if (ppcb->Next != NULL)
				{
					ppcb->Next->Pre = RL[i];
				}
				RL[i]->Next = ppcb->Next;

				//清空自身的前后指针
				ppcb->Pre = NULL;
				ppcb->Next = NULL;

				//修改状态为运行态
				PCBList[current_id]->Type = RUN;
				break;
			}
		}
	};
}

/*
*********************************************************************************************************
 *									DestoryPCB - 进程销毁
 * 
 * 	描述: 
 * 		通过KillPCBTree()函数,将当前进程和所有子进程进行销毁,并移除所占用的资源,然后执行进程调度.
 * 	
 * 	参数:
 * 		pid   要进行销毁的进程名称.
*********************************************************************************************************
*/

void DestoryPCB(char *pid)
{
	PCB *ppcb;

	//遍历PID队列,通过名称找到要销毁的进程ID
	for (int i = 0; i < PCBLIST_SIZE; i++)
	{
		if (strcmp(PID[i], pid) == 0)
		{
			ppcb = PCBList[i];

			//防止子进程杀死父进程
			if (ppcb == PCBList[current_id]->Parent)
			{
				printf("Child can't Kill Parent!\n");
				return;
			}

			//进程树销毁
			KillPCBTree(ppcb);
			break;
		}
	}

	Scheduler();
}

/*
*********************************************************************************************************
 *									KillPCBTree - 进程树销毁
 * 
 * 	描述: 
 * 		通过递归,销毁当前进程和子进程,清除当前进程和子进程的阻塞状态,或者释放所占用的资源,并检查被释放
 * 		资源的阻塞队列的头节点进程是否满足条件,将资源条件满足的阻塞进程移入就绪队列.
 * 
 * 	参数:
 * 		pcb   要进行销毁的进程指针.
*********************************************************************************************************
*/

void KillPCBTree(PCB *pcb)
{
	PCB *pchild, **pchilds, **childstemp;
	PCB *pblock, **pblocks;
	RCB *prcb;

	printf("Kill: %s\n", pcb->PID);
	//检查当前进程是否有子进程
	pchilds = pcb->Childs;
	while (*pchilds != NULL)
	{
		pchild = *pchilds;
		KillPCBTree(pchild); //递归子进程指针
		
	}

	

	//检查当前进程的资源占用队列和请求资源队列
	for (int i = 0; i < RESTYPE_NUM; i++)
	{
		prcb = RCBList[i];

		//当前进程存在资源占用情况
		if (pcb->OcpListCount[i] != 0)
		{
			ReleaseRes(pcb, prcb, pcb->OcpListCount[i]); //释放资源
		}

		//当前进程存在请求资源情况
		if (pcb->RequestCount[i] != 0)
		{
			//在阻塞队列中清除进程
			pblocks = prcb->BlockList;
			pblock = *pblocks;
			while (pblock != NULL && pblock->PID != pcb->PID)
			{
				pblocks++;
				pblock = *pblocks;
			}

			//更新阻塞队列
			UpdateBlockList(pblocks);
			pcb->RequestCount[i] = 0;
		}

		pblocks = prcb->BlockList;
		pblock = *pblocks;

		//检查当前资源阻塞队列是否有满足条件的阻塞进程
		while (pblock != NULL && prcb->FreeCount >= pblock->RequestCount[prcb->ID])
		{
			//分配资源
			AllocateRes(pblock, prcb, pblock->RequestCount[prcb->ID]);
			pblock->RequestCount[prcb->ID] = 0;

			//将获得资源的阻塞进程插入就绪队列
			pblock->Type = READY;
			Insert(RL[pblock->Prio], pblock);

			//更新阻塞队列
			pblocks = prcb->BlockList;
			UpdateBlockList(pblocks);
			pblock = *pblocks;
		}
	}

	//若处于就绪队列
	if (pcb->Type != RUN && pcb->Next != NULL)
	{
		pcb->Next->Pre = pcb->Pre;
		pcb->Next = NULL;
	}
	if (pcb->Type != RUN && pcb->Pre != NULL)
	{
		pcb->Pre->Next = pcb->Next;
		pcb->Pre = NULL;
	}

	//将节点从父节点的子进程队列中删除
	if (pcb->Parent != NULL)
	{
		pchilds = pcb->Parent->Childs;
		while (*pchilds != pcb)
		{
			pchilds++;
		}
		childstemp = pchilds;
		while (*(++pchilds) != NULL)
		{
			*childstemp = *pchilds;
			childstemp = pchilds;
			*childstemp = NULL;
		}
		*childstemp = NULL;
	}

	printf("Kill Succeed: %s\n", pcb->PID);


	//销毁其他信息
	strcpy(PID[pcb->ID], "?");
	pcb->PID = PID[pcb->ID];
	pcb->Parent = NULL;
	pcb->Prio = -1;
	pcb->Type = -1;

}

/*
*********************************************************************************************************
 *									Timeout - 时间片轮转
 * 
 * 	描述: 
 * 		当前运行进程的运行时间结束,将当前运行进程插入就绪优先级队列,并调度优先级最高的就绪进程运行.
 * 
 * 	参数:
 * 		pid   要进行销毁的进程名称.
*********************************************************************************************************
*/

void Timeout(void)
{
	PCB *pcb;

	//将运行进程插入就绪优先级队列
	pcb = PCBList[current_id];
	pcb->Type = READY;
	Insert(RL[pcb->Prio], pcb);

	printf("Time out: %s\n", pcb->PID);
	Scheduler();
}

/*
*********************************************************************************************************
 *									Request - 请求资源
 * 
 * 	描述: 
 * 		根据当前进程请求的资源类型和个数,决定执行阻塞进程操作BlockPCB(),或者是资源分配操作AllocateRes().
 * 
 * 	参数:
 * 		rid   要请求的资源名称.
 * 
 * 		n     请求的资源个数；
*********************************************************************************************************
*/

void Request(char *rid, int n)
{
	RCB *prcb;
	PCB *ppcb;

	//遍历RCB队列,通过资源名称找到资源对应的RCB
	for (int i = 0; i < RESTYPE_NUM; i++)
	{
		if (strcmp(RCBList[i]->RID, rid) == 0)
		{
			prcb = RCBList[i];
			break;
		}
	}

	ppcb = PCBList[current_id];
	if (prcb->FreeCount < n)
	{
		BlockPCB(ppcb, prcb, n); //资源不足,阻塞进程
		Scheduler();
	}
	else
	{
		AllocateRes(ppcb, prcb, n); //资源充足,分配资源
	}
}

/*
*********************************************************************************************************
 *									Release - 释放资源
 * 
 * 	描述: 
 * 		根据当前进程释放的资源名称和个数,决定是否释放资源,并检查当前资源的阻塞队列是否有满足条件的阻塞
 *		进程,将其移出阻塞队列并放入对应的就绪优先级队列中.
 * 
 * 	参数:
 * 		rid   要释放的资源名称.
 * 
 * 		n     释放的资源个数.
*********************************************************************************************************
*/

void Release(char *rid, int n)
{
	PCB *ppcb, *pblock, **pblocks;
	RCB *prcb;

	//遍历RCB队列,通过资源名称找到资源对应的RCB
	for (int i = 0; i < RESTYPE_NUM; i++)
	{
		if (strcmp(RCBList[i]->RID, rid) == 0)
		{
			prcb = RCBList[i];
			break;
		}
	}

	ppcb = PCBList[current_id];
	if (ppcb->OcpListCount[prcb->ID] < n)
	{
		//拥有资源小于n,指令无效,出错返回
		printf("%s Release error: no full resource\n", ppcb->PID);
		return;
	}
	else
	{
		ReleaseRes(ppcb, prcb, n); //条件满足,释放资源
	}

	pblocks = prcb->BlockList;
	pblock = *pblocks;

	//检查当前资源阻塞队列是否有满足条件的阻塞进程
	while (pblock != NULL && prcb->FreeCount >= pblock->RequestCount[prcb->ID])
	{
		//分配资源
		AllocateRes(pblock, prcb, pblock->RequestCount[prcb->ID]);
		pblock->RequestCount[prcb->ID] = 0;

		//将获得资源的阻塞进程插入就绪队列
		pblock->Type = READY;
		Insert(RL[pblock->Prio], pblock);

		//更新阻塞队列
		UpdateBlockList(pblocks);
		pblock = *pblocks;
	}

	Scheduler();
}

/*
*********************************************************************************************************
 *									BlockPCB - 阻塞进程
 * 
 * 	描述: 
 * 		将进程放入对应的阻塞队列,并在进程对应的资源请求对列中增加所请求的资源个数.
 * 
 * 	参数:
 * 		pcb   要被阻塞的进程指针.
 * 
 * 		rcb   被请求资源的资源管理器RCB.
 * 
 * 		n 	  请求的资源个数
*********************************************************************************************************
*/

void BlockPCB(PCB *pcb, RCB *rcb, int n)
{
	PCB **pblocks;

	pcb->Type = BLOCK;

	//将进程放入资源阻塞队列的尾部
	pblocks = rcb->BlockList;
	while (*pblocks != NULL)
	{
		pblocks++;
	}
	*pblocks = pcb;

	pcb->RequestCount[rcb->ID] += n;
	
	printf("Block %s on %s-%d\n", pcb->PID, rcb->RID, n);
}

/*
*********************************************************************************************************
 *									AllocateRes - 资源分配
 * 
 * 	描述: 
 * 		将对应的资源分配给进程,根据请求的资源个数,将空闲的资源放入进程的资源占用队列中.
 * 
 * 	参数:
 * 		pcb   请求资源的进程指针.
 * 
 * 		rcb   被请求资源的资源管理器RCB.
 * 
 * 		n 	  请求的资源个数.
*********************************************************************************************************
*/

void AllocateRes(PCB *pcb, RCB *rcb, int n)
{
	RES *pocp, *pres, **preses;

	//将n个资源放入进程的占用资源队列中
	pocp = pcb->OcpResList[rcb->ID]; //获取对应的资源占用队列头指针
	preses = rcb->ResList;			 //获取RCB中存储的资源队列
	for (int i = 0; i < n; i++)
	{
		//到达占用资源队列的尾部
		while (pocp->Next != NULL)
		{
			pocp = pocp->Next;
		}
		pres = *preses;

		//将空闲资源放入占用资源队列
		while (pres->Status != FREE)
		{
			preses++;
			pres = *preses;
		}
		pocp->Next = pres;
		pres->Status = ALLOCATE;
	}

	//增加占用计数,减少空闲资源计数
	pcb->OcpListCount[rcb->ID] += n;
	rcb->FreeCount -= n;

	printf("Allocate %s-%d for %s\n", rcb->RID, n, pcb->PID);
}

/*
*********************************************************************************************************
 *									AllocateRes - 资源释放
 * 
 * 	描述: 
 * 		释放进程所占用的资源.
 * 
 * 	参数:
 * 		pcb   释放资源的进程指针.
 * 
 * 		rcb   被释放资源的资源管理器RCB.
 * 
 * 		n 	  释放的资源个数.
*********************************************************************************************************
*/

void ReleaseRes(PCB *pcb, RCB *rcb, int n)
{
	RES *ocprl, *prel;

	//将n个资源从进程的资源占用队列释放
	ocprl = pcb->OcpResList[rcb->ID];
	for (int i = 0; i < n; i++)
	{
		prel = ocprl->Next;
		ocprl->Next = prel->Next;
		prel->Next = NULL;
		prel->Status = FREE;
	}

	//减少占用计数,增加空闲资源计数
	pcb->OcpListCount[rcb->ID] -= n;
	rcb->FreeCount += n;

	printf("Release  %s-%d from %s\n", rcb->RID, n, pcb->PID);
}

/*
*********************************************************************************************************
 *									UpdateBlockList - 更新阻塞队列
 * 
 * 	描述: 
 * 		将进程从阻塞队列中移出.
 * 
 * 	参数:
 * 		blocks   指向对应资源阻塞队列的二级指针.
*********************************************************************************************************
*/

void UpdateBlockList(PCB **blocks)
{
	PCB **blockstemp;

	//遍历阻塞队列,将对应的进程移出,并把其余阻塞进程的节点向前移动
	blockstemp = blocks;
	while (*(++blocks) != NULL)
	{
		*blockstemp = *blocks;
		blockstemp = blocks;
		*blockstemp = NULL;
	}
	*blockstemp = NULL;
}

/*
*********************************************************************************************************
 *									ListAllPCB - 显示所有进程的信息
 * 
 * 	描述: 
 * 		将PCB队列中所有存在的进程的信息显示出来,包括进程名称,进程运行状态和进程所在的队列以及该队列
 * 		的内容.
 * 
 * 	参数:
 * 		无.
*********************************************************************************************************
*/

void ListAllPCB(void)
{
	PCB *ppcb, *pblock, **pblocks;
	RCB *prcb;

	/*printf("--------------------------------------------------------------------\n");*/
	printf("PID\tTYPE\t\tRL/BL(high -> low)\n\n");

	//遍历PCB队列
	for (int i = 0; i < PCBLIST_SIZE; i++)
	{
		//获取存在进程
		if (strcmp(PID[i], "?") != 0)
		{
			//输出进程的运行状态和所在队列内容
			printf("%s\t", PCBList[i]->PID);
			switch (PCBList[i]->Type)
			{
			case READY:
				printf("ready\t\t");
				printf("RL: ");
				//遍历就绪队列,按优先级顺序输出就绪队列进程
				for (int j = PRIO_NUM - 1; j >= 0; j--)
				{
					ppcb = RL[j];
					while (ppcb->Next != NULL)
					{
						printf("%s  ", ppcb->Next->PID);
						ppcb = ppcb->Next;
					}
				}
				break;

			case RUN:
				printf("running\t\t");
				break;

			case BLOCK:
				printf("block\t\t");
				//遍历阻塞队列,按先进先出顺序输出阻塞队列进程
				for (int j = 0; j < RESTYPE_NUM; j++)
				{
					if (PCBList[i]->RequestCount[j] != 0)
					{
						prcb = RCBList[j];
						printf("BL(%s): ", prcb->RID);
						pblocks = prcb->BlockList;
						while (*pblocks != NULL)
						{
							pblock = *pblocks++;
							printf("%s  ", pblock->PID);
						}
					}
				}
				break;

			default:
				break;
			}
			printf("\n");
		}
	}

	//输出当前运行进程名称
	/*printf("Current: %s\n", PCBList[current_id]->PID);*/

}

/*
*********************************************************************************************************
 *									ListAllRES - 显示所有资源的信息
 * 
 * 	描述: 
 * 		将RCB队列中的所有资源的信息显示出来,包括资源的名称,资源的空闲计数和资源的占用情况.
 * 
 * 	参数:
 * 		无.
*********************************************************************************************************
*/

void ListAllRES(void)
{
	RCB *prcb;
	RES *pres, **preses;

	printf("RID\tFreeCount\tStatus(O:free X:allocated)\n\n");

	//遍历RCB队列
	for (int i = 0; i < RESTYPE_NUM; i++)
	{
		prcb = RCBList[i];
		preses = prcb->ResList;

		//输出资源占用情况
		printf("%s\t", prcb->RID);
		printf("%d\t\t", prcb->FreeCount);
		while (*preses != NULL)
		{
			pres = *preses;
			switch (pres->Status)
			{
			case FREE:
				printf("O|"); //空闲资源
				break;

			case ALLOCATE:
				printf("X|"); //占用资源
				break;

			default:
				break;
			}
			preses++;
		}
		printf("\n");
	}

	//输出当前运行进程名称
	/*printf("Current: %s\n", PCBList[current_id]->PID);
	printf("--------------------------------------------------------------------\n");*/
}

/*
*********************************************************************************************************
 *									ShowInform - 显示单个进程的信息
 * 
 * 	描述: 
 * 		通过进程名称获取某个资源的信息,包括进程ID,进程优先级,进程运行状态,进程的父进程名称和子进程名
 * 		称.
 * 
 * 	参数:
 * 		无.
*********************************************************************************************************
*/

void ShowInform(char *pid)
{
	PCB *ppcb, *pparent, *pchild, **pchilds;
	int i;

	//遍历PCB队列,通过名称获取对应的PCB
	for (i = 0; i < PCBLIST_SIZE; i++)
	{
		if (strcmp(PID[i], pid) == 0)
		{
			ppcb = PCBList[i];
			break;
		}
	}

	//名称有误,出错并退出函数
	if (i == PCBLIST_SIZE)
	{
		printf("%s is not exsited", pid);
		return;
	}

	//输出进程的名称,进程ID和进程优先级
	/*printf("--------------------------------------------------------------------\n");*/
	printf("PID: %s\nID: %d\nPrio: ", ppcb->PID, ppcb->ID);
	switch (ppcb->Prio)
	{
	case INIT:
		printf("init-0");
		break;

	case USER:
		printf("user-1");
		break;

	case SYSTEM:
		printf("system-2");
		break;

	default:
		break;
	}

	//输出进程的运行状态
	printf("\nType: ");
	switch (ppcb->Type)
	{
	case READY:
		printf("ready\t\t");
		break;
	case RUN:
		printf("running\t\t");
		break;
	case BLOCK:
		printf("block\t\t");
		break;
	default:
		break;
	}

	//输出当前进程的父进程名称
	printf("\nParent: ");
	pparent = ppcb->Parent;
	if (pparent != NULL)
	{
		printf("%s", pparent->PID);
	}

	//输出当前进程的所有子进程名称
	pchilds = ppcb->Childs;
	printf("\nChilds: ");
	while (*pchilds != NULL)
	{
		pchild = *pchilds;
		printf("%s ", pchild->PID);
		pchilds++;
	}

	printf("\n");
}

/*
*********************************************************************************************************
 *									Uninstaller - 程序卸载
 * 
 * 	描述: 
 * 		通过init的进程树销毁所有进程,并把所有动态申请的内存空间释放.
 * 
 * 	参数:
 * 		无.
*********************************************************************************************************
*/

void Uninstaller(void)
{
	RES *pres;
	RES **preses;

	printf("--------------------------Uninstaller-------------------------------\n");
	printf("Kill Process Tree\n\n");
	//销毁所有进程
	KillPCBTree(PCBList[0]);

	//释放PCB队列和队列中的变量动态申请的内存空间
	for (int i = 0; i < PCBLIST_SIZE; i++)
	{
		free(PCBList[i]->Childs);
		for (int j = 0; j < RESTYPE_NUM; j++)
		{
			free(PCBList[i]->OcpResList[j]);
		}
		free(PCBList[i]);
	}

	//释放RL优先级进程队列内存空间
	for (int i = 0; i < PRIO_NUM; i++)
	{
		free(RL[i]);
	}

	//释放RCB队列和队列中的变量动态申请的内存空间
	for (int i = 0; i < RESTYPE_NUM; i++)
	{
		preses = RCBList[i]->ResList;
		for (int j = 0; j < RCBList[i]->FreeCount; j++)
		{
			pres = *preses;
			free(pres);
			preses++;
		}
		free(RCBList[i]->ResList);
		free(RCBList[i]->BlockList);
		free(RCBList[i]);
	}

	printf("\n\n end \n");
	printf("--------------------------------------------------------------------\n");
}
