#ifndef __HEADERS_H__
#define __HEADERS_H__

struct __attribute__((packed)) ip_header {
    unsigned char      iph_ihl:4, iph_ver:4;
    unsigned char      iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned short int iph_flag_offset;
    unsigned char      iph_ttl;
    unsigned char      iph_protocol;
    unsigned short int iph_chksum;
    unsigned int       iph_sourceip;
    unsigned int       iph_destip;
};

struct __attribute__((packed)) arp_header{
    unsigned short arp_hwtype;
    unsigned short arp_proto;
    unsigned char arp_hwsz;
    unsigned char arp_protosz;
    unsigned short arp_opcode;
    unsigned char arp_sha[6];
    unsigned int arp_spa;
    unsigned char arp_dha[6];
    unsigned int arp_dpa;
};


typedef struct ip_header ip_header;
typedef struct arp_header arp_header;


unsigned short chksum(unsigned short*buff,int len){
    unsigned long sum=0;
    while(len>1){
        sum += *buff++;
        len -= 2;
    }
    if(len){
        sum +=*(unsigned char*)buff;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum += sum>>16;
    return (unsigned short)(~sum);
}


#endif
