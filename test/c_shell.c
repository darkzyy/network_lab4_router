#include<stdlib.h>
#include<unistd.h>

int main(){
		system("arping -c 1 -I eth0 192.168.80.1 > /tmp/route_tmp");
		sleep(1);
	return 0;
}
