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
#define queue_len 10
#define arp_len 42
#define proto_arp 0x0806
#define proto_ip 0x0800

//tables will be used:
route_item		route_tab[MAX_ROUTE_INFO_SIZE];
arp_table_item	arp_tab[MAX_ARP_SIZE];
device_item		device_tab[MAX_DEVICE_SIZE];

const unsigned int my_ip_addr_int = 0;

int sock_fd = -1;//套接字
unsigned char socket_buffer[buffer_len];
unsigned char socket_queue[queue_len][buffer_len];
int queue_valid[queue_len];
int current_queue_len = 0;
arp_header arp_buffer_send;
unsigned char arp_buffer[arp_len];

struct sockaddr_ll	skt_addr;
struct ifreq		ifr;

inline int route_entry_hit(int i,unsigned int dst_ip){
	return ((route_tab[i].netmask & dst_ip) == route_tab[i].destination);
}

inline int in_subnet(unsigned int ip,int route_tab_index){
	return (ip&route_tab[route_tab_index].netmask) ==
		(route_tab[route_tab_index].gateway&route_tab[route_tab_index].netmask);
}

inline int arp_entry_hit(int i,unsigned int dst_ip){
	//printf("dst ip: 0x%x,\tarp entry ip: 0x%x\n",dst_ip,arp_tab[i].ip_addr);
	return (arp_tab[i].ip_addr == dst_ip);
}

void ip_datagram_handle(unsigned char* eth,ip_header* iph);

void forward_ip_datagram(unsigned char* eth,unsigned int dst_ip,int route_tab_index);

void init_device_tab(const char* filename){
	/*
	device_tab[0].valid			= 1;
	strcpy(device_tab[0].interface,"eth1");
	device_tab[0].mac_addr[0]	= 0x00; 
	device_tab[0].mac_addr[1]	= 0x0c; 
	device_tab[0].mac_addr[2]	= 0x29; 
	device_tab[0].mac_addr[3]	= 0x8d; 
	device_tab[0].mac_addr[4]	= 0x82; 
	device_tab[0].mac_addr[5]	= 0x8b; 
	device_tab[0].ip_addr		= inet_addr("192.168.1.1");
	device_tab[1].valid			= 1;
	strcpy(device_tab[1].interface,"eth2");
	device_tab[1].mac_addr[0]	= 0x00; 
	device_tab[1].mac_addr[1]	= 0x0c; 
	device_tab[1].mac_addr[2]	= 0x29; 
	device_tab[1].mac_addr[3]	= 0x8d; 
	device_tab[1].mac_addr[4]	= 0x82; 
	device_tab[1].mac_addr[5]	= 0x95; 
	device_tab[1].ip_addr		= inet_addr("192.168.2.1");

	int i;
	for(i=2;i<MAX_DEVICE_SIZE;++i){
		device_tab[i].valid = 0;
	}
	*/
	FILE* pfile;
	pfile = fopen(filename,"rb");
	if(pfile == NULL){
		printf("error opening route_table!\n");
		return;
	}
	int suc;
	suc = fread(device_tab,1,sizeof(device_tab),pfile);
	if(suc!=sizeof(device_tab)){
		printf("error when reading device_tab");
	}
	fclose(pfile);

}

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
	for(i=0;i<10;i++){
		queue_valid[i] = 0;
	}
}

void arp_buffer_init(){
	arp_buffer[12]		=	8;
	arp_buffer[13]		=	6;
	arp_header* arph	=	(arp_header*)(arp_buffer+14);
	arph->arp_hwtype	=	0x0100;//Ethernet
	arph->arp_proto		=	0x0008;//IP
	arph->arp_hwsz		=	6;
	arph->arp_protosz	=	4;
	arph->arp_opcode	=	0x0100;//request
}

/*void read_static_arp_tab(const char* filename){
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
}
*/

void socket_addr_init(){
	skt_addr.sll_addr[6] = 0x00;
	skt_addr.sll_addr[7] = 0x00;
	skt_addr.sll_family		=	PF_PACKET;
	skt_addr.sll_ifindex	=	0;//!!!!!!
	skt_addr.sll_hatype		=	0;
	skt_addr.sll_pkttype	=	0;
	skt_addr.sll_halen		=	6;
}

void print_ip(unsigned int* ip){
	unsigned char* ip_char = (unsigned char*)ip;
	printf("%d.%d.%d.%d\n",ip_char[0],ip_char[1],ip_char[2],ip_char[3]);
}

void arp_handle(unsigned char* eth, arp_header* arph){
	if(arph->arp_opcode == 0x0100){
		printf("-----arp request\n");
		//ignore arp request now;
		return;
	}
	else if(arph->arp_opcode == 0x0200){//arp reply,add it into arp table
		printf("-----arp reply\n");
		int i,j;
		for(i=0;i<MAX_ARP_SIZE;++i){
			if(arp_tab[i].valid == 0){
				break;
			}
		}
		/********** insert into arp table ************/
		arp_tab[i].valid	=	1;
		arp_tab[i].ip_addr	=	arph->arp_spa;
		//printf("-------ip: 0x%x\n",arph->arp_spa);
		for(j=0;j<6;j++){
			arp_tab[i].mac_addr[j]	=	arph->arp_sha[j];
			//printf("%x ",arph->arp_sha[j]);
		}
		printf("\ncurrent queue len: %d\n",current_queue_len);
		if(current_queue_len>0){
			for(i=0;i<current_queue_len;++i){
				if(queue_valid[i] == 1){
					unsigned char * eth	=	socket_queue[i];
					ip_header* iph = (ip_header*)(eth+14);
					ip_datagram_handle(eth,iph);
					queue_valid[i] = 0;
					current_queue_len--;
				}
			}
		}
	}
}

void ip_datagram_handle(unsigned char* eth,ip_header* iph){
	int i;
	for(i=0;i<MAX_DEVICE_SIZE;i++){
		if(device_tab[i].valid == 0){
			break;
		}
		if(iph->iph_destip == device_tab[i].ip_addr ||
					iph->iph_sourceip == device_tab[i].ip_addr){
			printf("throw a datagram,sent to myself\n");
			return;

		}
	}

	for(i=0;i<MAX_ROUTE_INFO_SIZE;++i){
		if(route_tab[i].valid==0){
			break;
		}
		if(route_entry_hit(i,iph->iph_destip)){//找到转发规则
			printf("hit a route entry\n");
			if(in_subnet(iph->iph_destip,i)){
				forward_ip_datagram(eth,iph->iph_destip,i);
			}
			else{
				printf("not in one subnet!!\n");
				forward_ip_datagram(eth,route_tab[i].gateway,i);
			}
			return ;
		}
	}
	//没有查到相关的表项，丢弃
	printf("throw a datagram,dst ip:");
	print_ip(&(iph->iph_destip));
	return;
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
		//printf("---------------------catch a datagram!\n");
		eth_head = socket_buffer;
		proto_type = ((unsigned short) eth_head[12] <<8) + eth_head[13];
		//printf("protocol type: 0x%x\n",proto_type);
		//判断类型并分发
		if(proto_type == proto_arp){
			printf("\n------------------------catch an arp datagram!\n");
			arp_head = eth_head + 14;
			arp_handle(eth_head,(arp_header*)arp_head);
		}
		else if(proto_type == proto_ip){
			printf("\n------------------------catch an ip datagram!\n");
			ip_head = eth_head + 14;
			print_ip(&(((ip_header*)ip_head)->iph_sourceip));
			printf(" =====>");
			print_ip(&(((ip_header*)ip_head)->iph_destip));
			ip_datagram_handle(eth_head,(ip_header*)ip_head);
		}
		else{
			continue;
		}
	}
}

int main(){
	socket_addr_init();
	init_device_tab("./dev_tab/dev_tab.binary");
	arp_buffer_init();
	arp_table_init();
	read_route_tab("./route_tab/route_table.binary");
	//read_static_arp_tab("./arp_tab/arp_table.binary");
	if((sock_fd=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0){
		printf("error create raw socket\n");
		return -1;
	}
	main_loop();
	return 0;
}

int arp_request(unsigned int dst_ip,int route_tab_index){//index是在arp table中的下标
	arp_header* arph = (arp_header*)(arp_buffer+14);
	int i,device_tab_index;
	printf("required interface: %s\n",route_tab[route_tab_index].interface);
	for(i=0;i<MAX_ROUTE_INFO_SIZE;++i){
		if(strcmp(route_tab[route_tab_index].interface,
						device_tab[i].interface)==0){
			device_tab_index = i;
			break;
		}
	}
	if(i==MAX_ROUTE_INFO_SIZE){
		return 0;
	}
	for(i=0;i<6;++i){
		arph->arp_sha[i]	=	device_tab[device_tab_index].mac_addr[i];
		arp_buffer[6+i]		=	device_tab[device_tab_index].mac_addr[i];
		arph->arp_dha[i]	=	0xff;
		arp_buffer[i]		=	0xff;
	}
	arph->arp_spa = device_tab[device_tab_index].ip_addr;
	arph->arp_dpa = dst_ip;

	//get ifindex
	strncpy(ifr.ifr_name, route_tab[route_tab_index].interface, IFNAMSIZ);
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		//exit(1);
		return 0;
	}
	skt_addr.sll_ifindex	=	ifr.ifr_ifindex;
	skt_addr.sll_protocol	=	htons(ETH_P_ARP);
	int sent = sendto(sock_fd,arp_buffer,arp_len,0,
				(struct sockaddr*)&skt_addr,sizeof(skt_addr));
	assert(sent>0);
	printf("--------------------------send an arp\n");
	return 1;
}

void change_dstmac_forward_datagram(unsigned char* eth,
			unsigned char* mac_addr,char* interface){
	int i;
	for(i=0;i<6;i++){
		eth[i+6]				=	eth[i];
		eth[i]					=	mac_addr[i];
		skt_addr.sll_addr[i]	=	mac_addr[i];
	}
	//get ifindex
	//printf("\ninterface: %s",interface);
	strncpy(ifr.ifr_name, interface, IFNAMSIZ);
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) == -1) {
		printf("ioctl error\n");
		perror("SIOCGIFINDEX");
		//exit(1);
		return;
	}
	skt_addr.sll_ifindex = ifr.ifr_ifindex;
	skt_addr.sll_protocol	=	htons(ETH_P_IP);

	ip_header* iph = (ip_header*)(eth+14);
	int packet_len = htons(iph->iph_len) + 14;
	int sent = sendto(sock_fd,eth,packet_len,0,
				(struct sockaddr*)&skt_addr,sizeof(skt_addr));
	assert(sent>0);
	printf("----------forwarded a datagram ==>");
	print_ip(&(iph->iph_destip));
}

void forward_ip_datagram(unsigned char* eth,unsigned int dst_ip,int route_tab_index){
	int i;
	char* interface = route_tab[route_tab_index].interface;
	//printf("interface tp be used index:%d name:%s\n",route_tab_index,interface);
	for(i=0;i<MAX_ARP_SIZE;++i){
		//printf("i = %d\n",i);
		if(arp_tab[i].valid==0){
			int suc = arp_request(dst_ip,route_tab_index);
			if(suc){//add this packet to queue
				int j;
				if(current_queue_len == 10){//when queue is full,flush it
					for(j=0;j<10;j++){
						queue_valid[j] = 0;
					}
				}
				for(j=0;j<10;j++){
					if(queue_valid[j]==0){
						ip_header* iph = (ip_header*)(eth+14);
						int packet_len = htons(iph->iph_len) + 14;
						int i_tmp;
						for(i_tmp=0;i_tmp<packet_len;i_tmp++){
							socket_queue[j][i_tmp] = eth[i_tmp];
						}
						//printf("======================enqueue addr: 0x%x\n",(int)socket_queue[j]);
						memcpy((void*)socket_queue[j],(void*)eth,packet_len);
						queue_valid[j]=1;
						current_queue_len++;
						printf("entered packet queue,len: %d\n",packet_len);
						break;
					}
				}
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

