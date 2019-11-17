#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <semaphore.h>
using namespace std;

sem_t floor_mutex;

struct floor_info{
	int floor;		//Contro panel's floor
	int* shm_current_floor;		//Elevator current floor (a share memory adress)
};

void *printFloor(void* floorinfo){
	while(1){
		cout<<"in print thread circle!"<<endl;
		sem_wait(&floor_mutex);
		floor_info *finfo = (floor_info*)floorinfo;
		int* cur_floor = (int*)(finfo->shm_current_floor);
		if(finfo->floor==4)
			cout<<"Elevator";
		else
			cout<<"Floor"<<finfo->floor;
		cout<<" control panel: Elevator is at "<<*cur_floor<<" floor"<<endl;
		sem_post(&floor_mutex);
		sleep(3);
	}
	return NULL;
}

int main(){
	//use share memory
	int shm_id = shmget(IPC_PRIVATE,1,IPC_CREAT | 0666);  //Elevator current floor
	if(shm_id<0)
	  cerr<<"create share memory failed!"<<endl;
	int* current_floor = (int*)shmat(shm_id,0,0);
	*current_floor=112312341;
	
	//use semaphore
	sem_init(&floor_mutex,0,1);

	pid_t p1 = fork();
	if(p1==0){
		int* current_floor = (int*)shmat(shm_id,0,0);

		//create a new thread to print current elevator position every 3 seconds
		pthread_t print_thread;
		//pack a struct for thread arguments
		floor_info f1_info;
		f1_info.floor=1;
		f1_info.shm_current_floor=current_floor;
		int print_thread_id = pthread_create(&print_thread,NULL,printFloor,(void*)(&f1_info));
		if(print_thread_id!=0){
		  cerr<<"create print thread failed in floor 1"<<endl;
		  cerr<<"Error Number: "<<print_thread_id<<endl;
		}
		else
		  cout<<"create print thread success!"<<endl;
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}

	pid_t p2 = fork();	
	if(p2==0){
		int* current_floor = (int*)shmat(shm_id,0,0);
		//create a new thread to print current elevator position every 3 seconds
		pthread_t print_thread;
		//pack a struct for thread arguments
		floor_info f2_info;
		f2_info.floor=2;
		f2_info.shm_current_floor=current_floor;
		int print_thread_id = pthread_create(&print_thread,NULL,printFloor,(void*)(&f2_info));
		if(print_thread_id!=0){
		  cerr<<"create print thread failed in floor 2"<<endl;
		  cerr<<"Error Number: "<<print_thread_id<<endl;
		}
		else
		  cout<<"create print thread success!"<<endl;
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}
	pid_t p3 = fork();
	if(p3==0){
		int* current_floor = (int*)shmat(shm_id,0,0);
		//create a new thread to print current elevator position every 3 seconds
		pthread_t print_thread;
		//pack a struct for thread arguments
		floor_info f3_info;
		f3_info.floor=3;
		f3_info.shm_current_floor=current_floor;
		int print_thread_id = pthread_create(&print_thread,NULL,printFloor,(void*)(&f3_info));
		if(print_thread_id!=0){
		  cerr<<"create print thread failed in floor 3"<<endl;
		  cerr<<"Error Number: "<<print_thread_id<<endl;
		}
		else
		  cout<<"create print thread success!"<<endl;
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}
	pid_t p4 = fork();	
	if(p4==0){
		int* current_floor = (int*)shmat(shm_id,0,0);
		//create a new thread to print current elevator position every 3 seconds
		pthread_t print_thread;
		//pack a struct for thread arguments
		floor_info elevator_info;
		elevator_info.floor=4;
		elevator_info.shm_current_floor=current_floor;
		int print_thread_id = pthread_create(&print_thread,NULL,printFloor,(void*)(&elevator_info));
		if(print_thread_id!=0){
		  cerr<<"create print thread failed in elevator"<<endl;
		  cerr<<"Error Number: "<<print_thread_id<<endl;
		}
		else
		  cout<<"create print thread success!"<<endl;
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}
	pid_t p5 = fork();	
	if(p5==0){
		cout<<"This is elevator process"<<endl;
		int* current_floor = (int*)shmat(shm_id,0,0);
		//sem_wait(&floor_mutex);
		//cout<<"Elevator control panel: Elevator is at "<<*current_floor<<" floor"<<endl;
		//sem_post(&floor_mutex);
		return 0;
	}
	return 0;
}
