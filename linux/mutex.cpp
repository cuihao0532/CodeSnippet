#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

/***********************************************
 * 
 * Initialize a mutex with attribute(can be NULL)
 * int pthread_mutex_init(
 *      pthread_mutex_t* mutex, 
 *      const pthread_mutexattr_t* mutexattr);
 * 
 * lock a mutex
 * int pthread_mutex_lock(pthread_mutex_t* mutex);
 *
 * unlock a mutex
 * int pthread_mutex_unlock(pthread_mutex_t* mutex);
 *
 * destroy a mutex
 * int pthread_mutex_destroy(pthread_mutex_t* mutex);
 *
 *
 *
 * *********************************************/


void* work_thread(void* p)
{
	if (NULL == p)
		return  const_cast<char*>("invalid thread argument");

	pthread_mutex_t* pMutex = (pthread_mutex_t*)(p);

	//current thread ID
	pthread_t nThreadID = pthread_self();

	int i = 0;
	while(++ i <= 10)
	{
		pthread_mutex_lock(pMutex);
		cout << "Thread " << nThreadID << " is Running! " << endl;	
		pthread_mutex_unlock(pMutex);
		usleep(1000 * 1000 * 1); //1 seconds
	}
		
	return const_cast<char*>("------ finish -----------");

}
 

int main()
{
	const size_t nThreadCount = 2;
	pthread_t threadIDs[nThreadCount];
	int nRet = -1;
	
	void* pRet = NULL; //thread return value
	pthread_mutex_t mutex;
	
	nRet = pthread_mutex_init(&mutex, NULL);
	if (0 != nRet)
		return -1;

	for (size_t i = 0; i < nThreadCount; ++ i)
	{
		nRet = pthread_create(&threadIDs[i], NULL, work_thread, (void*)(&mutex));
		if (0 != nRet)
			continue;
	}	

	for (size_t i = 0; i < nThreadCount; ++ i)
	{
		nRet = pthread_join(threadIDs[i], &pRet);
		if (0 == nRet)
		{
			cout << " Thread " << threadIDs[i] << " Finished ! " \
				" It's return value is " << (char*)pRet << endl;
		}

	}

	pthread_mutex_destroy(&mutex);

	return 0;
}

