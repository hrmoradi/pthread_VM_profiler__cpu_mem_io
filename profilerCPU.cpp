//g++ -O0 -std=c++11  profilerCPU.cpp -o profilerCPU.bin -lpthread
#include <iostream>
#include <stdio.h> /* c printf */
#include <cstdlib> /* stdlib.h for c*/
#include <pthread.h>
#include <unistd.h> /* for usleep */
#include <string>
#include <chrono>
#include <limits.h>

using namespace std;

pthread_barrier_t barr;
struct threadData{
	int threadId;
	unsigned long long int returnData;
};
int work=1;

void* threadFunction(void *threadArg){

	register unsigned long long int accessCounter=0;
	register unsigned long long int one =1;
	struct threadData *td;
	td = (struct threadData *) threadArg;
	
	pthread_barrier_wait(&barr);
	
	//auto startTime = std::chrono::high_resolution_clock::now();
	while (work){
		accessCounter+=one;

	}
	//auto endTime = std::chrono::high_resolution_clock::now();
		
	//cout << " threadId: " << td->threadId << " counter is:" << counter <<" time: "<< (void *)std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << endl;
	return (void *) accessCounter;
}

int main(int argc, char* argv[]){

	int a;
	printf("size int:                    %5lu %25d %25d\n", (unsigned long)sizeof(a), INT_MIN , INT_MAX);// returns size_t type	
	unsigned int b;
        printf("size unsigned int:           %5lu %25u\n", (unsigned long)sizeof(b), UINT_MAX);
	long int c;
	printf("size long int:               %5lu %25ld %25ld\n", (unsigned long)sizeof(c), LONG_MIN, LONG_MAX);
	long long int d;
	printf("size long long int:          %5lu %25lld %25lld\n", (unsigned long)sizeof(d), LLONG_MIN , LLONG_MAX);
	unsigned long long int e;
	printf("size unsigned long long int: %5lu %25llu\n", (unsigned long)sizeof(e), ULLONG_MAX);
					


	int numberOfThread = std::stoi(argv[1]); /* for std::stoi/stoll use the -std=c++11 in g++ */
	int runTime = std::stoi(argv[2]);
	int threadErr;
	long microseconds=100000;// 1/10 of second 
	struct threadData td[numberOfThread];
	pthread_t threadPool[numberOfThread];

	int nthreads=numberOfThread+1;
	pthread_barrier_init(&barr,NULL,nthreads);

	for(int i=0; i < numberOfThread; i++){ /* in c int declaration out of for loop */
		cout << "main(): creating thread " << i << endl; /* c++ */
		/* printf("main(): creating thread %d",i);/*in C: %d int, %lf double, %c char, %p address, %s string */
		td[i].threadId=i;
		td[i].returnData=0;
		/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg); */
		threadErr = pthread_create(&threadPool[i],NULL, threadFunction,(void*)&td[i] ); /* (void* i) can return any structure with void8 */
		if (threadErr){
			cout << "Error creating thread" << threadErr << endl;
			exit(-1);
		}
	}
	
	void* outData;
	pthread_barrier_wait(&barr);

	auto startTime = std::chrono::high_resolution_clock::now();

	usleep(microseconds*runTime);
	work =0;
	//usleep(100000); /* 1/10 sec */ 
	
	for(int i=0; i<numberOfThread;i++){
		pthread_join(threadPool[i],&outData);
		td[i].returnData=(unsigned long long int) outData;
	}

	auto endTime = std::chrono::high_resolution_clock::now();



	unsigned long long int total=0;
	for(int i=0; i< numberOfThread; i++){
		cout << "thread id: "<<td[i].threadId << " count: "<< td[i].returnData <<endl;
		total+=td[i].returnData;
	}
	cout << "main thread time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << endl;
	cout << "total: "<< total <<endl;

	long double frequency = (((long double)total*10)/ (numberOfThread*runTime));// runtime 1 mean 1/10 second 

	cout << " frequency: "<< frequency/1000000000 <<endl;	

	//pthread_exit(NULL);
}
/* g++ -O0 profilerCPU.cpp -lpthread -std=c++11 -o profilerCPU.bin */
/* g++ profiler.cpp -lpthread -std=c++11 -o profiler */
/* short int and int: -32,767 to 32,767
unsigned short int and unsigned int: 0 to 65,535
long int: -2,147,483,647 to 2,147,483,647
unsigned long int: 0 to 4,294,967,295
long long int: -9,223,372,036,854,775,807 to 9,223,372,036,854,775,807
unsigned long long int: 0 to 18,446,744,073,709,551,615 */
/*./profiler 16 2000000000*/
