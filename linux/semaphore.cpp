#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/**********************************************
 *
int semt_init(
 semt_t* sem,  //a semaphore pointer
 int     pshared, //0 as a local semaphore of cuurent process, or the semaphore can be shared between mulit processes
 unsigned value  //the init  value of this memaphore
 )


//minus ONE  value of semaphore
int sem_wait(sem_t* sem);

//add ONE value of semaphore
int sem_post(sem_t* sem);


//destroy the semaphore
//int sem_destroy(sem_t* sem);


NOTE:
All the functions above Return Zero IF SUCCESS !

*************************************************/ 

using namespace std;

sem_t g_semt;

void* work_thread(void* p)
{
	pthread_t tID = pthread_self();

	cout << "-------" << tID << " is waiting for a semaphore -------" << endl;	
	sem_wait(&g_semt);
	cout << "-------" << tID << " got a semaphore, is Runing -------" << endl << endl;
	usleep(1000 * 1000 * 2);  //2 seconds
	sem_post(&g_semt);

	static char* pRet = "thread finished! \n";

	return pRet;
}

int main()
{
	const size_t nThreadCount = 5; //amounts of thread array
	const unsigned int nSemaphoreCount = 2; //initial value of semaphore
	int nRet = -1;
	void* pRet = NULL;
	pthread_t threadIDs[nThreadCount] = {0};
	
	nRet = sem_init(&g_semt, 0, nSemaphoreCount);
	if (0 != nRet)
		return -1;

	for (size_t i = 0; i < nThreadCount; ++ i)
	{
		nRet = pthread_create(&threadIDs[i], NULL, work_thread, NULL); 
		if (0 != nRet)
			continue;
	}

	for (size_t i = 0; i < nThreadCount; ++ i)
	{
		int nRet2 = pthread_join(threadIDs[i], &pRet);
		cout << endl << threadIDs[i] << " return value is " << (char*)pRet << endl;
	}

	cout << endl << endl;

	sem_destroy(&g_semt);

	return 0;
}







