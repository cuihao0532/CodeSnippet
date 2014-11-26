
/*a demo for Linux MultiThread   */
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include  <unistd.h> 
using namespace std;

//thread function
void*  start_routine(void* p)
{
	if (0 == p)
		return 0;

	size_t nLoops = *( (size_t*) p );
	
	for (size_t i = 0; i < nLoops; ++ i)
	{
		cout << i << endl;
		usleep(1000 * 800);   //800 ms	
	}

	return 0;
}


int main()
{
	pthread_t ptThread1;
	size_t* pLoops = new size_t(10);
	int nRet = pthread_create(&ptThread1, 0, start_routine, (void*)pLoops);
	if (0 != nRet)
		cerr << endl << "create thread error!" << endl;
	else
		cerr << endl << "create thread successfully, return value code is " << nRet \
		   	<< endl << "thread ID is " << ptThread1 << endl;

	if (0 == nRet)
	{
		cout << endl << "wait for thread " << ptThread1 << endl;
		void* pRetVal = 0;
        int nJoinRet = pthread_join(ptThread1, &pRetVal);
		cout << endl << "thread " << ptThread1 << " finished !" << endl;
	}

	delete pLoops;
	pLoops = 0;

	system("ls");

	return 0;
}



