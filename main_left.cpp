#include <iostream>
#include <unistd.h>
using namespace std;
int main(){
	pid_t pid = fork();
	if(pid==0){
		cout<<"This is floor1 process"<<endl;
	}
	else{
		if(fork()==0){
			cout<<"This is floor2 process"<<endl;
		}
		else{
			if(fork()==0){
				cout<<"This is floor3 process"<<endl;
			}
			else{
				if(fork()==0){
					cout<<"This is elevator controller process"<<endl;
				}
				else{
					cout<<"This is elevator process"<<endl;
				}
			}
		}
	}
}
