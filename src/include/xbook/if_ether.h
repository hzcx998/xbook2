#ifndef _XBOOK_IF_ETHER_H
#define _XBOOK_IF_ETHER_H

#define ETH_ALEN 6  //定义了以太网接口的MAC地址的长度为6个字节 
#define ETH_HLAN 14  //定义了以太网帧的头长度为14个字节
#define ETH_ZLEN 60  //定义了以太网帧的最小长度为 ETH_ZLEN + ETH_FCS_LEN = 64个字节
#define ETH_DATA_LEN 1500  //定义了以太网帧的最大负载为1500个字节
#define ETH_FRAME_LEN 1514  //定义了以太网正的最大长度为ETH_DATA_LEN + ETH_FCS_LEN = 1518个字节
#define ETH_FCS_LEN 4   //定义了以太网帧的CRC值占4个字节

struct ethhdr
{
    unsigned  char  h_dest[ETH_ALEN];  //目的MAC地址
    unsigned  char  h_source[ETH_ALEN];  //源MAC地址
    unsigned short  h_proto ;  //网络层所使用的协议类型
}__attribute__((packed));   //用于告诉编译器不要对这个结构体中的缝隙部分进行填充操作；

#endif  /* _XBOOK_IF_ETHER_H */