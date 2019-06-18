//g++ -O0 profilerMem-j16i-16.cpp -lpthread -std=c++11 -o profilerMem-j16i-16.bin
#include <iostream>
#include <stdio.h> /* c printf */
#include <cstdlib> /* stdlib.h for c*/
#include <pthread.h>
#include <unistd.h> /* for usleep */
#include <string>

#include <chrono>
#include <vector>

using namespace std;
pthread_barrier_t barr;
const std::size_t KB = 1024;
const std::size_t MB = 1024 * KB;
const std::size_t GB = 1024 * MB;
const int ij=4096;
const int jumpi=1;
const int jumpj=16;
struct threadData{
	int threadId;
	unsigned long long int returnData[ij][ij]={};
};
struct threadData td[16];
int work=1;

void* threadFunction(void *threadarg){

	register unsigned long long int accessCounter=0;
	register unsigned long long int one=1;
	struct threadData *my_data;
	my_data = (struct threadData *) threadarg;
	
	pthread_barrier_wait(&barr);	

	while (work){
		for (int i=0; i<ij; i+=jumpi){
			for (int j =0; j<ij; j+=jumpj){
				my_data->returnData[i][j]+=one;
				accessCounter+=one;
				if (work<1){break;};
			}
			if (work<1){break;};
		}

	}

        return (void *)  accessCounter;

}

int main(int argc, char* argv[]){ 

	int numberOfThread = std::stoi(argv[1]); /* std::stoi is a C++11 function. You have to use the -std=c++11 to enable it in both g++ and clang++ */
	unsigned long long int runTime = std::stoll(argv[2]);
	
	int threadErr;
	double microseconds=100000;// 1/10 second 
	pthread_t threadPool[numberOfThread];

	int nthreads=numberOfThread+1;
	pthread_barrier_init(&barr,NULL,nthreads);

	for(int t=0; t < numberOfThread; t++){ 
		cout << "main(): creating thread " << t << endl; /* c++ */
		/* printf("main(): creating thread %d",i);/* c, %d int, %lf double, %c char, %p address, %s stirng */
		td[t].threadId=t;
		//td[t].returnData;//={};
		/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg); */
		threadErr = pthread_create(&threadPool[t],NULL, threadFunction,(void*)&td[t] ); 
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
                td[i].returnData[0][0]=(unsigned long long int) outData;
        }

	auto endTime = std::chrono::high_resolution_clock::now();

	unsigned long long int total=0;

 	for(int i=0; i< numberOfThread; i++){
                cout << "thread id: "<<td[i].threadId << " count: "<< td[i].returnData[0][0] <<endl;
                total+=td[i].returnData[0][0];
        }


	cout << "main thread time:" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << endl;
        cout << "total: "<< total <<endl;

	unsigned long long int readMByte = (total*8)/MB;
        cout << " Mbyte/sec: "<< ((readMByte*10)/runTime) <<endl;//runtime 1 means 1/10 secound

	/*	pthread_exit(NULL);*/
}
/* g++ profiler.cpp -lpthread -std=c++11 -o profiler */
/* short int and int: -32,767 to 32,767
unsigned short int and unsigned int: 0 to 65,535
long int: -2,147,483,647 to 2,147,483,647
unsigned long int: 0 to 4,294,967,295
long long int: -9,223,372,036,854,775,807 to 9,223,372,036,854,775,807
unsigned long long int: 1 to 18,446,744,073,709,551,615 */
