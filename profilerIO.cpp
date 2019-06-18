//g++ -O0 -std=c++11  profilerIO.cpp -o profilerIO.bin -lpthread
#include <iomanip>
#include <sstream>
#include <fcntl.h>

#include <stdlib.h>
#include <fstream>
#include <vector>
#include <cstdint>
#include <numeric>
#include <random>
#include <algorithm>
#include <cassert>

#include <iostream>
#include <stdio.h> /* c printf */
#include <cstdlib> /* stdlib.h for c*/
#include <pthread.h>
#include <unistd.h> /* for usleep */
#include <string>
#include <string.h>
#include <chrono>

/*
#define O_RDONLY    0x0000     // open for reading only /
#define O_WRONLY    0x0001     // open for writing only /
#define O_CREAT     0x0040     // create if nonexistant /
#define O_TRUNC     0x0200     // truncate to zero length/
*/

using namespace std;
int work =1;
int createfile=0;
pthread_barrier_t barr;
const std::size_t KB = 1024;
const std::size_t MB = 1024 * KB;
const std::size_t GB = 1024 * MB;
const std::size_t FILE_SIZE =256*KB;
const int NUM_FILES=4096; // 4 thread // 256 KB // 4096 files // = 4GB
struct threadData{
	int threadId;
	unsigned long long int returnData;
};

std::vector<uint64_t> GenerateData(std::size_t bytes)
{
    assert(bytes % sizeof(uint64_t) == 0);
    std::vector<uint64_t> data(bytes / sizeof(uint64_t));
    std::iota(data.begin(), data.end(), 8);
    std::shuffle(data.begin(), data.end(), std::mt19937{ std::random_device{}() });
    return data;
}

void* threadFunction(void *threadarg){

	struct threadData *my_data;
	my_data = (struct threadData *) threadarg;
	std::vector<uint64_t> data(FILE_SIZE/sizeof(uint64_t));
	std::vector<uint64_t> readFromDisk(FILE_SIZE/sizeof(uint64_t)); 
	if(createfile==1){	
		for(int i=1; i<=NUM_FILES; i++){
			std::string prefix("./IOfiles/Writenfile-t");
			int n_digits =4;
			std::stringstream ext;
			std::stringstream fileName;
			ext << std::setfill('0') << std::setw(n_digits) << my_data->threadId;
			fileName << prefix<<ext.str() <<"-f" << std::setfill('0') << std::setw(n_digits) << i ;
	
			data = GenerateData(FILE_SIZE);
			int file;
			file = open( fileName.str().c_str(),O_CREAT|O_WRONLY|O_TRUNC|O_SYNC,S_IRWXU);// |O_DIRECT, cause problems
			if (file <0) {
			    perror("Failed to create file  ");
			}
			write(file,&data[0],FILE_SIZE); 
			close(file);
		}
	}
	
	sync();
	std::ofstream ofs("/proc/sys/vm/drop_caches");
	ofs << "3" << std::endl;	

	cout << "files created for thread id: "<<my_data->threadId <<endl;	

	int i=1;
	register unsigned long long int accessCounter=0;
	register unsigned long long int temp=0;
	pthread_barrier_wait(&barr);
	while(work){
		while (work && i<=NUM_FILES){
			std::string prefix("./IOfiles/Writenfile-t");
	                int n_digits =4;
        	        std::stringstream ext;
	                std::stringstream fileName;
        		ext << std::setfill('0') << std::setw(n_digits) << my_data->threadId;
	                fileName << prefix<<ext.str() <<"-f" << std::setfill('0') << std::setw(n_digits) << i ;
			int file;
			file = open( fileName.str().c_str(),O_RDONLY|O_SYNC);// |O_DIRECT not working not able to read file 
			if (file <0) {
		    		perror("Failed: ");
			}
			read(file,&readFromDisk[0],FILE_SIZE); // &readFromDisk
			temp+=readFromDisk[0];
			close(file);

			i++;
			accessCounter+=1;
		}	
		i=1;
	}
	return (void *)accessCounter;
}

int main(int argc, char* argv[]){ 
	createfile = std::stoi(argv[3]); 
	int numberOfThread = std::stoi(argv[1]); /* use the -std=c++11 to enable in g++ */	
	double runTime = std::stoi(argv[2]);
	int threadErr;
	double microseconds=100000;// 1/10 second 
	struct threadData td[numberOfThread]; 
	pthread_t threadPool[numberOfThread];

	int nthreads=numberOfThread+1;
	pthread_barrier_init(&barr,NULL,nthreads);

	for(int i=0; i < numberOfThread; i++){ 
		cout << "main(): creating thread " << i << endl; /* c++ */
		/* printf("main(): creating thread %d",i);/* c, %d int, %lf double, %c char, %p address, %s stirng */
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

        unsigned long long int readMByte = (total*FILE_SIZE)/MB;
	cout << " Mbyte/sec: "<< ((10*readMByte)/runTime) <<endl;

	sync();

        std::ofstream ofse("/proc/sys/vm/drop_caches");
        ofse << "0" << std::endl;


}
/* g++ profiler.cpp -lpthread -std=c++11 -o profiler */
/* short int and int: -32,767 to 32,767
unsigned short int and unsigned int: 0 to 65,535
long int: -2,147,483,647 to 2,147,483,647
unsigned long int: 0 to 4,294,967,295
long long int: -9,223,372,036,854,775,807 to 9,223,372,036,854,775,807
unsigned long long int: 0 to 18,446,744,073,709,551,615 */
