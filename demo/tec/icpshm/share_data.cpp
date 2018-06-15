﻿#include <iostream>
using namespace std;


#ifdef _WIN32
#pragma data_seg("MySectionName")
int g_instance_count = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:MySectionName,RWS")

void selSleep(long n)
{
	Sleep(n);
}
bool Exit(DWORD ctrltype)
{
	if (ctrltype == CTRL_CLOSE_EVENT)
		g_instance_count--;
	return true;
}
void listenClose()
{
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)Exit, true))
	{
		exit(0);
	}
}
int GetInstanceCount()
{
	return g_instance_count;
}
void InstanceAddOne()
{
	++g_instance_count;
}
#else
#include <stdio.h>  
#include <stdlib.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
void selSleep(long n)
{
	usleep(n*100);
}
void InstanceSubOne();
void handler(int sig)
{
	InstanceSubOne();
	exit(0);
}
void listenClose()
{
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGHUP, &act, NULL) < 0     
		|| sigaction(SIGINT, &act, NULL) < 0  
		|| sigaction(SIGQUIT, &act, NULL) < 0 
		//|| sigaction(SIGKILL,&act,NULL) < 0
		|| sigaction(SIGTERM, &act, NULL) < 0
		)
	{
		exit(0);
	}
}
#define SHARE_MEMORY_KEY 88
int GetInstanceCount()
{
	char *shmptr;
	int shmid = -1;
	shmid = shmget(SHARE_MEMORY_KEY, 5, 0);
	bool newOne = false;
	if (shmid == -1)
		newOne = true;

	shmid = shmget(SHARE_MEMORY_KEY, 5, IPC_CREAT);
	if (shmid == -1)
	{
		printf("Shared Memory Created error...\n"); exit(0);
	}
	shmptr = (char *)shmat(shmid, NULL, 0);
	if (shmptr == (void*)-1)
	{
		printf("shmat error,shmptr= %d \n", shmptr);
		exit(1);
	}
	if (newOne)
	{
		sprintf(shmptr, "%d", 0);
		return 0;
	}
	return atoi(shmptr);
}	
void InstanceAddOne()
{
	int shmid = shmget(SHARE_MEMORY_KEY, 5, IPC_CREAT);
	if (shmid == -1)
	{
		printf("Shared Memory Created error...\n"); exit(0);
	}
	char * shmptr = (char *)shmat(shmid, NULL, 0);
	if (shmptr == (void*)-1)
	{
		printf("shmat error,shmptr= %d \n", shmptr);
		exit(1);
	}
	sprintf(shmptr, "%d", atoi(shmptr) + 1);
}
void InstanceSubOne()
{
	int shmid = shmget(SHARE_MEMORY_KEY, 5, IPC_CREAT);
	if (shmid == -1)
	{
		printf("Shared Memory Created error...\n"); exit(0);
	}
	char * shmptr = (char *)shmat(shmid, NULL, 0);
	if (shmptr == (void*)-1)
	{
		printf("shmat error,shmptr= %d \n", shmptr);
		exit(1);
	}
	sprintf(shmptr, "%d", atoi(shmptr) - 1);

	if (atoi(shmptr) <= 0)
	{
		shmctl(shmid, IPC_RMID, NULL);
	}
}

#endif

int main()
{
	listenClose();
	InstanceAddOne();

	int count = -1;
	while (true)
	{
		int cur = GetInstanceCount();
		if (count != cur)
		{
			count = cur;
			cout << "当前开启的进程数:" << count << endl;
		}

		selSleep(100);
	}
	
	return 0;
}

