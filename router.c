#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "conf.h"
#include "headers.h"

#define buffer_len 1024
#define proto_arp 0x0806
#define proto_ip 0x0800

//tables will be used:
route_item		route_tab[MAX_ROUTE_INFO_SIZE];
arp_table_item	arp_tab[MAX_ARP_SIZE];
device_item		device_tab[MAX_DEVICE_SIZE];

const unsigned int my_ip_addr_int = 0;
const char* my_ip_addr = "192.168.x.x";
const char* my_mac_addr = "x.x.x...";

int sock_fd = -1;//套接字
unsigned char socket_buffer[buffer_len];
arp_header arp_buffer_send;
unsigned char socket_buffer_arp[buffer_len];

struct sockaddr_ll	skt_addr;
struct ifreq		ifr;
//int ifindex		=	0;

void read_route_tab(const char* filename){
	FILE* pfile;
	pfile = fopen(filename,"rb");
	if(pfile == NULL){
		printf("error opening route_table!\n");
		return;
	}
	int suc;
	suc = fread(route_tab,1,sizeof(route_tab),pfile);
	if(suc!=sizeof(route_tab)){
		printf("error when reading route_tab");
	}
	fclose(pfile);
	/*************test for table*************/
	int i;
	for(i=0;i<MAX_ROUTE_INFO_SIZE;i++){
		if(route_tab[i].valid==1){
			printf("ip: 0x%x\n",route_tab[i].destination);
			printf("interface: %s\n",route_tab[i].interface);
		}
		else{
			break;
		}
	}
}

void arp_table_init(){
	int i;
	for(i=0;i<MAX_ARP_SIZE;++i){
		arp_tab[i].valid = 0;
	}
}

void read_static_arp_tab(const char* filename){
	FILE* pfile;
	pfile = fopen(filename,"rb");
	if(pfile == NULL){
		printf("error opening arp_table!\n");
		return;
	}
	int suc;
	suc = fread(arp_tab,1,sizeof(arp_tab),pfile);
	if(suc!=sizeof(arp_tab)){
		printf("error when reading arp_tab");
	}
	fclose(pfile);
	/*************test for table*************/
	int i;
	for(i=0;i<MAX_ARP_SIZE;i++){
		if(arp_tab[i].valid==1){
			printf("ip: 0x%x\n",arp_tab[i].ip_addr);
		}
		else{
			break;
		}
	}
}

void socket_addr_init(){
	skt_addr.sll_addr[6] = 0x00;
	skt_addr.sll_addr[7] = 0x00;
	skt_addr.sll_family		=	PF_PACKET;
	skt_addr.sll_protocol	=	htons(ETH_P_IP);
	skt_addr.sll_ifindex	=	0;//!!!!!!
	skt_addr.sll_hatype		=	0;
	skt_addr.sll_pkttype	=	0;
	skt_addr.sll_halen		=	6;
}

void arp_reply(unsigned char* eth, arp_header* arph){
}

void forward_ip_datagram(unsigned char* eth,unsigned int dst_ip,char* interface);

inline int route_entry_hit(int i,unsigned int dst_ip){
	return ((route_tab[i].netmask & dst_ip) == route_tab[i].destination);
}

void ip_datagram_handle(unsigned char* eth,ip_header* iph){
	if(iph->iph_destip == my_ip_addr_int){//不用做转发
		printf("throw a datagram,sent to myself:\n");
		return;
	}
	else{
		int i;
		for(i=0;i<MAX_ROUTE_INFO_SIZE;++i){
			if(route_tab[i].valid==0){
				break;
			}
			if(route_entry_hit(i,iph->iph_destip)){//找到转发规则
				printf("hit a route entry\n");
				forward_ip_datagram(eth,iph->iph_destip,
							route_tab[i].interface);
				return ;
			}
		}
		//没有查到相关的表项，丢弃
		printf("throw a datagram,dst ip: 0x%x\n",iph->iph_destip);
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
		printf("---------------------catch a datagram!\n");
		eth_head = socket_buffer;
		proto_type = ((unsigned short) eth_head[12] <<8) + eth_head[13];
		//printf("protocol type: 0x%x\n",proto_type);
		//判断类型并分发
		if(proto_type == proto_arp){
			arp_head = eth_head + 14;
			arp_reply(eth_head,(arp_header*)arp_head);
		}
		else if(proto_type == proto_ip){
			//printf("catch an ip datagram!\n");
			ip_head = eth_head + 14;
			ip_datagram_handle(eth_head,(ip_header*)ip_head);
		}
		else{
			continue;
		}
	}
}



int main(){
	socket_addr_init();
	read_route_tab("./route_tab/route_table.binary");
	read_static_arp_tab("./arp_tab/arp_table.binary");
	if((sock_fd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0){
		printf("error create raw socket\n");
		return -1;
	}
	main_loop();
	return 0;
}

//pthread_mutex_t arp_timeout;

int arp_request(unsigned int dst_ip,int index){//index是在arp table中的下标
	return 0;
}

inline int arp_entry_hit(int i,unsigned int dst_ip){
	printf("dst ip: 0x%x,\tarp entry ip: 0x%x\n",dst_ip,arp_tab[i].ip_addr);
	return (arp_tab[i].ip_addr == dst_ip);
}

void change_dstmac_forward_datagram(unsigned char* eth,
			unsigned char* mac_addr,char* interface){
	int i;
	printf("my_mac:\n");
	for(i=0;i<6;i++){
		printf("0x%x ",eth[i]);
	}
	printf("\ndst_mac:\n");
	for(i=0;i<6;i++){
		printf("0x%x ",mac_addr[i]);
	}
	printf("\n");
	for(i=0;i<6;i++){
		eth[i+6]				=	eth[i];
		eth[i]					=	mac_addr[i];
		skt_addr.sll_addr[i]	=	mac_addr[i];
	}
	//get ifindex
	strncpy(ifr.ifr_name, interface, IFNAMSIZ);
    if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        exit(1);
    }

    skt_addr.sll_ifindex = ifr.ifr_ifindex;

	//printf("interface: %s\n",interface);
	//setsockopt(sock_fd,SOL_SOCKET,SO_BINDTODEVICE,interface,4);
	printf("ifindex: %d\n",skt_addr.sll_ifindex);
	int sent = sendto(sock_fd,socket_buffer,buffer_len,0,
				(struct sockaddr*)&skt_addr,sizeof(skt_addr));
	assert(sent>0);
	printf("-------------------forwarded a datagram\n");

}

void forward_ip_datagram(unsigned char* eth,unsigned int dst_ip,char* interface){
	int i;
	for(i=0;i<MAX_ARP_SIZE;++i){
		printf("i = %d\n",i);
		if(arp_tab[i].valid==0){
			int suc = arp_request(dst_ip,i);
			if(suc){//forward via arp entry i
				change_dstmac_forward_datagram(eth,arp_tab[i].mac_addr,interface);
				break;
			}
			else{
				printf("arp for dstip failed\n");
				break;
			}
		}
		else{
			if(arp_entry_hit(i,dst_ip)){//forward via arp entry i
				printf("hit a arp entry\n");
				change_dstmac_forward_datagram(eth,arp_tab[i].mac_addr,interface);
				break;
			}
		}
	}
	return;
}

