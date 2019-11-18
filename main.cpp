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

struct panel_info{
	int floor;		//which floor's control panel
	int* shm_user_choice;	//user current choose which floor's control panel (a share memory address)
	int* input_mode;	//share memory address
};

struct input_info{
	int* input_mode;	//share memory address
	int* user_choice;	//share memory address
};

//show current elevator's position on every floor's conrol panel
void *printFloor(void* floorinfo){
	while(1){
		//cout<<"in print thread circle!"<<endl;
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

//the only place that userchoice can be changed. In other place, userchoice is read only
void *changeUserChoice(void* inputinfo){
	input_info* info = (input_info*)inputinfo;
	int* input_mode = info->input_mode;
	int* user_choice = info->user_choice;
	char key;
	while(1){
		if(*input_mode==2)
		  continue;
		key=cin.get();
		if(key=='1'||key=='2'||key=='3'){
			*user_choice = key-'0';
			*input_mode=2;
			cout<<"[CHANGE] user now choose floor "<<*user_choice<<" control panel"<<endl;
		}
	}
	return NULL;
}

void *callElevator(void* panelinfo){
	panel_info* pinfo = (panel_info*)panelinfo;
	int floor = pinfo->floor;
	int* user_choice = pinfo->shm_user_choice;
	int* input_mode = pinfo->input_mode;
	while(1){
		if(*input_mode==1)
		  continue;
		if(*user_choice == floor)	//user choose this floor's control panel
		{
			char cmd = cin.get();
			if(cmd=='w'||cmd=='s'){
				cout<<"this is in floor "<<floor<<" and this thread shows user choice is: "<<*user_choice<<endl;
				cout<<"[CALL] Floor "<<floor<<" calls the elevator!"<<endl;
				*input_mode=1;
			}
		}
	}
	return NULL;
}

int main(){
	//use share memory
	int shm_id = shmget(IPC_PRIVATE,1,IPC_CREAT | 0666);  //Elevator current floor
	if(shm_id<0)
		cerr<<"create share memory for current floor failed!"<<endl;
	int* current_floor = (int*)shmat(shm_id,0,0);
	*current_floor=2;

	int shm_id2 = shmget(IPC_PRIVATE,1,IPC_CREAT | 0666);	//indicate user choose which floor now
	if(shm_id2<0)
		cerr<<"create share memory for user choice failed!"<<endl;
	int* user_choice = (int*)shmat(shm_id2,0,0);
	*user_choice = 1;		//default

	int shm_id3 = shmget(IPC_PRIVATE,1,IPC_CREAT | 0666);	//indicate input mode
	int* input_mode = (int*)shmat(shm_id3,0,0);
	*input_mode = 1;		//1 = choose which floor, 2 = choose up or down
	
	//create a thread to receive user choose which floor's control panel to use
	pthread_t get_key_thread;
	input_info inputinfo;
	inputinfo.input_mode=input_mode;
	inputinfo.user_choice=user_choice;
	int get_key_thread_id = pthread_create(&get_key_thread,NULL,changeUserChoice,(void*)(&inputinfo));
	if(get_key_thread_id!=0){
		cerr<<"create print thread failed: get_key_thread"<<endl;
		cerr<<"Error Number: "<<get_key_thread_id<<endl;
	}
	else
		cout<<"create get_key_thread success!"<<endl;
	
	//use semaphore
	sem_init(&floor_mutex,0,1);

	//fork 5 times to create 5 process
	pid_t p1 = fork();		//floor1 process
	if(p1==0){
		//share memory bind
		int* current_floor = (int*)shmat(shm_id,0,0);
		int* user_choice = (int*)shmat(shm_id2,0,0);
		int* input_mode = (int*)shmat(shm_id3,0,0);

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

		//create a new thread to handle call elevator request every floor
		pthread_t call_elevator_thread;
		panel_info p1_info;
		p1_info.floor=1;
		p1_info.shm_user_choice=user_choice;
		p1_info.input_mode=input_mode;
		int call_elevator_thread_id = pthread_create(&call_elevator_thread,NULL,callElevator,(void*)(&p1_info));
		if(call_elevator_thread_id!=0){
		  cerr<<"create call elevator thread failed in floor 1"<<endl;
		  cerr<<"Error Number: "<<call_elevator_thread_id<<endl;
		}
		else
		  cout<<"create call elevator thread success!"<<endl;


		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		pthread_join(call_elevator_thread,&ret_val);
		return 0;
	}

	pid_t p2 = fork();		//floor2 process
	if(p2==0){
		int* current_floor = (int*)shmat(shm_id,0,0);
		int* input_mode = (int*)shmat(shm_id3,0,0);
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
		int* user_choice = (int*)shmat(shm_id2,0,0);
		//create a new thread to handle call elevator request every floor
		pthread_t call_elevator_thread;
		panel_info p2_info;
		p2_info.floor=2;
		p2_info.shm_user_choice=user_choice;
		p2_info.input_mode=input_mode;
		int call_elevator_thread_id = pthread_create(&call_elevator_thread,NULL,callElevator,(void*)(&p2_info));
		if(call_elevator_thread_id!=0){
		  cerr<<"create call elevator thread failed in floor 2"<<endl;
		  cerr<<"Error Number: "<<call_elevator_thread_id<<endl;
		}
		else
		  cout<<"create call elevator thread success!"<<endl;
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}
	pid_t p3 = fork();		//floor3 process
	if(p3==0){
		int* current_floor = (int*)shmat(shm_id,0,0);
		int* input_mode = (int*)shmat(shm_id3,0,0);
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
		int* user_choice = (int*)shmat(shm_id2,0,0);
		//create a new thread to handle call elevator request every floor
		pthread_t call_elevator_thread;
		panel_info p3_info;
		p3_info.floor=3;
		p3_info.shm_user_choice=user_choice;
		p3_info.input_mode=input_mode;
		int call_elevator_thread_id = pthread_create(&call_elevator_thread,NULL,callElevator,(void*)(&p3_info));
		if(call_elevator_thread_id!=0){
		  cerr<<"create call elevator thread failed in floor 3"<<endl;
		  cerr<<"Error Number: "<<call_elevator_thread_id<<endl;
		}
		else
		  cout<<"create call elevator thread success!"<<endl;
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}
	pid_t p4 = fork();		//elevator control panel process	
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
		int* user_choice = (int*)shmat(shm_id2,0,0);
		void* ret_val;
		//wait for print thread over (never)
		pthread_join(print_thread,&ret_val);
		return 0;
	}
	pid_t p5 = fork();		//elevator it self process	
	if(p5==0){
		cout<<"This is elevator process"<<endl;
		int* current_floor = (int*)shmat(shm_id,0,0);
		int* user_choice = (int*)shmat(shm_id2,0,0);
		//sem_wait(&floor_mutex);
		//cout<<"Elevator control panel: Elevator is at "<<*current_floor<<" floor"<<endl;
		//sem_post(&floor_mutex);
		return 0;
	}
	
	while(1){
		cout<<"User now choose "<<*user_choice<<" floor's control panel"<<endl;
		sleep(1);
	}
	void* ret;
	pthread_join(get_key_thread,&ret);
	return 0;
}
