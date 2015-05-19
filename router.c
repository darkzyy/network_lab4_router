#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "conf.h"
#include "headers.h"

#define buffer_len 1024
#define proto_arp 0x0806
#define proto_ip 0x8000

//tables will be used:
route_item		route_tab[MAX_ROUTE_INFO_SIZE];
arp_table_item	arp_tab[MAX_ARP_SIZE];
device_item		device_tab[MAX_DEVICE_SIZE];

const unsigned int my_ip_addr_int = 0;
const char* my_ip_addr = "192.168.x.x";
const char* my_mac_addr = "x.x.x...";

int sock_fd;//套接字
unsigned char socket_buffer[buffer_len];
arp_header arp_buffer_send;
unsigned char socket_buffer_arp[buffer_len];

void read_route_tab(const char* filename){
	FILE* pfile;
	pfile = fopen("route_table.binary","rb");
	if(pfile == NULL){
		printf("error opening a file!\n");
		return;
	}
	int suc;
	suc = fread(route_tab,1,sizeof(route_tab),pfile);
	if(suc!=sizeof(route_tab)){
		printf("error when reading n");
	}
	fclose(pfile);
}

void arp_table_init(){
	int i;
	for(i=0;i<MAX_ARP_SIZE;++i){
		arp_tab[i].valid = 0;
	}
}

void arp_reply(unsigned char* eth, arp_header* arph){
}

void forward_ip_datagram(unsigned char* eth,unsigned int dst_ip,char* interface);

inline int route_entry_hit(int i,unsigned int dst_ip){
	return ((route_tab[i].netmask & dst_ip) == route_tab[i].destination);
}

void ip_datagram_handle(unsigned char* eth,ip_header* iph){
	if(iph->iph_destip == my_ip_addr_int){//不用做转发
		return;
	}
	else{
		int i;
		for(i=0;i<MAX_ROUTE_INFO_SIZE;++i){
			if(route_tab[i].valid==0){
				break;
			}
			if(route_entry_hit(i,iph->iph_destip)){//找到转发规则
				forward_ip_datagram(eth,route_tab[i].gateway,
							route_tab[i].interface);
			}
		}
		//没有查到相关的表项，丢弃
		return;
	}
}

void main_loop(){
	int n_read;
	unsigned short proto_type;
	unsigned char *eth_head,*ip_head,*arp_head;
	while(1){
		//获取数据包
		n_read = recvfrom(sock_fd,socket_buffer,buffer_len,0,NULL,NULL);
		if(n_read <42){
			printf("error when recv msg");
			continue;
		}
		eth_head = socket_buffer;
		proto_type = ((unsigned short) eth_head[12] <<8) + eth_head[13];
		//判断类型并分发
		if(proto_type == proto_arp){
			arp_head = eth_head + 14;
			arp_reply(eth_head,(arp_header*)arp_head);
		}
		else if(proto_type == proto_ip){
			ip_head = eth_head + 14;
			ip_datagram_handle(eth_head,(ip_header*)ip_head);
		}
		else{
			continue;
		}
	}
}



int main(){
	read_route_tab("route_tab.binary");
	if((sock_fd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0){
		printf("error create raw socket\n");
		return -1;
	}
	main_loop();
	return 0;
}

pthread_mutex_t arp_timeout;

int arp_request(unsigned int dst_ip,int index){//index是在arp table中的下标

}

inline int arp_entry_hit(int i,unsigned int dst_ip){
	return (arp_tab[i].ip_addr == dst_ip);
}

void forward_ip_datagram(unsigned char* eth,unsigned int dst_ip,char* interface){
	int i;
	for(i=0;i<MAX_ARP_SIZE;++i){
		if(arp_tab[i].valid==0){
			int suc = arp_request(dst_ip,i);
			if(suc){//forward via arp entry i
			}
			else{
				printf("arp for dstip failed\n");
				return;
			}
		}
		else{
			if(arp_entry_hit(i,dst_ip)){//forward via arp entry i
			}
		}
	}
}

