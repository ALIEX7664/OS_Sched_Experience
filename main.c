#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "process_and_resource_manager.h"

#define BUFSIZE 32 //单行指令空间大小
#define COMSIZE 5   //单行指令的指令容量

int main(void)
{
    int count;                                         //单词计数
    char *tmp;                                         //用于截断回车符
    char *str = (char *)calloc(1, BUFSIZE);            //读取行命令
	char **pstr = (char **)calloc(COMSIZE, BUFSIZE);   //存储多个单词
	
    //初始化
    Init();

    //打开文件
    FILE *fp = fopen("TestShell.txt", "r");
    if (fp == NULL)
    {
        printf("fopen error!\n");
		return -1;
    }

	

    //读取命令,执行操作
    while (!feof(fp))
    {
        tmp = NULL;
        count = 0;

        //从文件中获取指令
        fgets(str, BUFSIZE, fp);

        //获取回车符存储地址并截断回车符
        if ((tmp = strstr(str, "\n")) == NULL)
        {
            break;
        }
        *tmp = ' ';

		printf("--------------------------------------------------------------------\n");

        //获取字符串中的每个单词
        pstr[count] = strtok(str, " "); //获取第一个字符串,存储str
        while (pstr[count] != NULL)
        {
            pstr[++count] = strtok(NULL, " "); //分割字符串中余下的单词
        }

        //指令执行
        if (strcmp(pstr[0], "cr") == 0)
        {
            CreatePCB(pstr[1], atoi(pstr[2]));
        }
        else if (strcmp(pstr[0], "de") == 0)
        {
            DestoryPCB(pstr[1]);
        }
        else if (strcmp(pstr[0], "to") == 0)
        {
            Timeout();
        }
        else if (strcmp(pstr[0], "req") == 0)
        {
            if (pstr[2] == NULL)
            {
                Request(pstr[1], 1);
            }
            else
            {
                Request(pstr[1], atoi(pstr[2]));
            }
        }
        else if (strcmp(pstr[0], "rel") == 0)
        {
            if (pstr[2] == NULL)
            {
                Release(pstr[1], 1);
            }
            else
            {
                Release(pstr[1], atoi(pstr[2]));
            }
        }
        else if (strcmp(pstr[0], "list") == 0)
        {
            if (strcmp(pstr[1], "PCB") == 0)
            {
                ListAllPCB();
            }
            else
            {
                ListAllRES();
            }
        }
        else if (strcmp(pstr[0], "show") == 0)
        {
            ShowInform(pstr[1]);
        }
        else
        {
            printf("command error\n");
        }

        printf("\nCurrent: %s\n", PCBList[current_id]->PID); //输出当前运行进程名称
		printf("--------------------------------------------------------------------\n");
    }

    //销毁所有进程并释放所分配的空间
    Uninstaller();
	free(pstr);
    free(str);
    
    // 关闭文件
    fclose(fp);
    return 0;
}
