#include<stdlib.h>
#include<unistd.h>

int main(){
	while(1){
		system("arping -I eth0 192.168.80.1");
		sleep(1);
	}
	return 0;
}
