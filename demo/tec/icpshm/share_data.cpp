#define ACTIVE_SD
#ifdef ACTIVE_SD

#include <iostream>
using namespace std;

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#pragma data_seg("MySectionName")
int g_instance_count = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:MySectionName,RWS")

#include <Windows.h>

void msSleep(long n)
{
	Sleep(n);
}
bool Exit(DWORD ctrltype)
{
	if (ctrltype == CTRL_CLOSE_EVENT)
	{
		g_instance_count--;
	}
	return true;
}
void listenCloseEvent()
{
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)Exit, true))
	{
		exit(0);
	}
}
int getInstanceCount()
{
	return g_instance_count;
}
void instanceAddOne()
{
	++g_instance_count;
}
#elif defined(__linux__)
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
void msSleep(long n)
{
	usleep(n*100);
}
void instanceSubOne();
void handler(int sig)
{
	instanceSubOne();
	exit(0);
}
void listenCloseEvent()
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

int g_shm_id = -1;
char* g_share_buff = NULL;
void checkAndShareMemory()
{
	if (-1 != g_shm_id || NULL != g_share_buff)
		return;

	g_shm_id = shmget(SHARE_MEMORY_KEY, 5, IPC_CREAT);
	if (g_shm_id == -1)
	{
		printf("Shared Memory Created error...\n"); exit(0);
	}
	g_share_buff = (char *)shmat(g_shm_id, NULL, 0);
	if (g_share_buff == (void*)-1)
	{
		printf("shmat error,shmptr= %d \n", g_share_buff);
		exit(1);
	}
}
void breakFromShareMemory()
{
	if (NULL == g_share_buff)
		return;

	shmdt(g_share_buff);
}
void closeShareMemory()
{
	if (-1 == g_shm_id)
		return;

	struct shmid_ds buf;
	shmctl(g_shm_id, IPC_RMID, &buf);
}
int getInstanceCount()
{
	checkAndShareMemory();
	if (NULL == g_share_buff)
		return -1;

	return atoi(g_share_buff);
}	
void instanceAddOne()
{
	checkAndShareMemory();
	if (NULL == g_share_buff)
		return;

	sprintf(g_share_buff, "%d", atoi(g_share_buff) + 1);
}
void instanceSubOne()
{
	checkAndShareMemory();
	if (NULL == g_share_buff)
		return;

	int result_count = atoi(g_share_buff) - 1;
	sprintf(g_share_buff, "%d", result_count);

	if (result_count < 1)
		closeShareMemory();
	else
		breakFromShareMemory();
}

#endif

int main()
{
	listenCloseEvent();
	instanceAddOne();

	int count = -1;
	while (true)
	{
		int cur = getInstanceCount();
		if (count != cur)
		{
			count = cur;
			cout << "当前开启的进程数:" << count << endl;
		}

		msSleep(100);
	}
	return 0;
}

#endif

