#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

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
 * int pthread_mutexattr_setpshared(
 *       pthread_mutexattr_t* mattr,
 *       int pshared  //PTHREAD_PROCESS_SHARE | PTHREAD_PROCESS_PRIVATE
 *       );
 * 
 * int pthread_mutexattr_getshared(
 * pthread_mutexattr_t* mattr,
 * int* pshared);
 *
 * int pthread_mutexattr_settype(
 *	     pthread_mutexattr_t* attr,
 *	     int type  //PTHREAD_MUTEX_TIMED_NP -- default value
 *				   //PTHREAD_MUTEX_RECURISIVE_NP -- allow a thread lock multitimes
 *				   //PTHREAD_MUTEX_ERRORCHECK_NO -- check error lock, return EDEADLK if the same thread want to LOCK
 *				   //PTHREAD_MUTEX_ADAPTIVE_NO -- adaptive lock, the simplest lock
 * )
 *
 *
 * int pthread_mutexattr_gettype(
 *		 pthread_mutexattr_t* attr,
 *		 int* type
 * )
 * *********************************************/



void* work_thread(void* p)
{
	if (NULL == p)
		return  const_cast<char*>("invalid thread argument");

	pthread_mutex_t* pMutex = (pthread_mutex_t*)(p);

	//current thread ID
	pthread_t nThreadID = pthread_self();

	int i = 0;
	while(++ i <= 3)
	{
		//lock multi times 
		pthread_mutex_lock(pMutex);
		pthread_mutex_lock(pMutex);
		
		cout << "Thread " << nThreadID << " is Running! " << endl;	
	
		//and so unlock multi times
		pthread_mutex_unlock(pMutex);
		pthread_mutex_unlock(pMutex);
		usleep(1000 * 1); //1 miliseconds
	}
		
	return const_cast<char*>("------ finish -----------");

}


void* work_thread2(void* p)
{
	if (NULL == p)
		return  const_cast<char*>("invalid thread argument");

	pthread_mutex_t* pMutex = (pthread_mutex_t*)(p);
	
	//current thread ID
	pthread_t nThreadID = pthread_self();

	int i = 0;
	while(++ i <= 3)
	{
		//if current thread can not enter mutex, 
		//and the function pthread_mutex_trylock will RETURN Immediatly
		if ( EBUSY == pthread_mutex_trylock(pMutex))
			cout << "Other thread is lock the resouce, i am waiting.." << endl;
		else
		{
			cout << "Thread " << nThreadID << " is Running! " << endl;	
			pthread_mutex_unlock(pMutex);
			usleep(1000 * 1); //1 miliseconds
		}

	}		
	return const_cast<char*>("------ finish -----------");

}


int main()
{
	const size_t nThreadCount = 3;
	pthread_t threadIDs[nThreadCount];
	int nRet = -1;
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;
	void* pRet = NULL; //thread return value

	//allow a thread lock multi times
	nRet = pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
	
	nRet = pthread_mutex_init(&mutex, &mutexattr);
	if (0 != nRet)
		return -1;

	for (size_t i = 0; i < nThreadCount - 1; ++ i)
	{
		nRet = pthread_create(&threadIDs[i], NULL, work_thread, (void*)(&mutex));
		if (0 != nRet)
			continue;
	}	

	nRet = pthread_create(&threadIDs[nThreadCount - 1], NULL, work_thread2, (void*)(&mutex));
	if (0 != nRet)
		cerr << endl << "work_thread2 created falied! " << endl;

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