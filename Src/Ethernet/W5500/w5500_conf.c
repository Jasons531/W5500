/*
**************************************************************************************************
* @file    		w5500_conf.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief  		配置MCU，移植W5500程序需要修改的文件，配置W5500的MAC和IP地址
**************************************************************************************************
*/
#include <stdio.h> 
#include <string.h>

#include "Ethernet/W5500/w5500_conf.h"
#include "Ethernet/W5500/utility.h"
#include "Ethernet/W5500/w5500.h"
#include "Ethernet/Internet/dhcp.h"
#include "spi.h"

SPI_HandleTypeDef hspi_w5500;

CONFIG_MSG  ConfigMsg, RecvMsg;

uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};		// 选择8个Socket每个Socket发送缓存的大小，在w5500.c的void sysinit()有设置过程
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};		// 选择8个Socket每个Socket接收缓存的大小，在w5500.c的void sysinit()有设置过程

extern uint8 MAC[6];

uint8 pub_buf[1460];

uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
uint8	ip_from_dhcp=1;
uint8   ip_from_define=0;

/*定义远端IP信息*/
//uint8  remote_ip[4]={120,77,213,80};			  						/*远端IP地址*/
//uint16 remote_port=3099;																/*远端端口号*/

uint8  remote_ip[4]={119,28,21,53};			  						/*远端IP地址*/
uint16 remote_port=1111;	

//uint8  remote_ip[4]={192,168,1,100};			  						/*远端IP地址*/
//uint16 remote_port=8080;

extern void Delay_us( uint32 time_us );
/**
*@brief		硬件复位W5500
*@param		无
*@return	无
*/
void reset_w5500(void)
{
  HAL_GPIO_WritePin(W5500_SCS_GPIO_Port, W5500_SCS_Pin,GPIO_PIN_RESET);
  Delay_us(500);  //至少拉低500us
  HAL_GPIO_WritePin(W5500_SCS_GPIO_Port, W5500_SCS_Pin,GPIO_PIN_SET);
  HAL_Delay(1600);
}


/**
*@brief		W5500片选信号设置函数
*@param		val: 为“0”表示片选端口为低，为“1”表示片选端口为高
*@return	无
*/
void wiz_cs(uint8_t val)
{
	if (val == LOW) 
	{
	  HAL_GPIO_WritePin(W5500_SCS_GPIO_Port, W5500_SCS_Pin, GPIO_PIN_RESET);
	}
	else if (val == HIGH)
	{
	  HAL_GPIO_WritePin(W5500_SCS_GPIO_Port, W5500_SCS_Pin, GPIO_PIN_SET);
	}
}

uint8_t SPI_SendByte(uint8_t byte)
{
  uint8_t d_read,d_send=byte;
  if(HAL_SPI_TransmitReceive(&hspi1,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
    d_read=0XFF;
  
  return d_read; 
}

void set_w5500_mac(void)
{
	memcpy(ConfigMsg.mac, mac, 6);
	setSHAR(ConfigMsg.mac);	/**/
	memcpy(DHCP_GET.mac, mac, 6);
}

void set_network(void)															// 配置初始化IP信息并打印，初始化8个Socket
{
  uint8 ip[6];
  setSHAR(ConfigMsg.mac);
  setSUBR(ConfigMsg.sub);
  setGAR(ConfigMsg.gw);
  setSIPR(ConfigMsg.lip);

  sysinit(txsize, rxsize); 													// 初始化8个socket
  setRTR(2000);																	// 设置溢出时间
  setRCR(3);																	// 设置最大重新发送次数
  
  getSHAR (ip);
  printf("MAC : %02x.%02x.%02x.%02x.%02x.%02x\r\n", ip[0],ip[1],ip[2],ip[3],ip[4],ip[5]);
  getSIPR (ip);
  printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getSUBR(ip);
  printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getGAR(ip);
  printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
}

void set_default(void)															// 设置默认MAC、IP、GW、SUB、DNS
{  
  uint8 lip[4]={192,168,1,199};
  uint8 sub[4]={255,255,255,0};
  uint8 gw[4]={192,168,1,1};
  uint8 dns[4]={8,8,8,8};
  if(ip_from_define==1)
	{
      printf("\r\nIP from define  \r\n");	
      memcpy(ConfigMsg.lip, lip, 4);
      memcpy(ConfigMsg.sub, sub, 4);
      memcpy(ConfigMsg.gw,  gw, 4);
      memcpy(ConfigMsg.mac, mac,6);
      memcpy(ConfigMsg.dns,dns,4);
	}
	if(ip_from_dhcp==1)								
	{
        printf("\r\nIP from DHCP	\r\n");		 
        memcpy(ConfigMsg.lip,DHCP_GET.lip, 4);
        memcpy(ConfigMsg.sub,DHCP_GET.sub, 4);
        memcpy(ConfigMsg.gw,DHCP_GET.gw, 4);
        memcpy(ConfigMsg.dns,DHCP_GET.dns,4);
	}

  ConfigMsg.dhcp=0;
  ConfigMsg.debug=1;
  ConfigMsg.fw_len=0;
  
  ConfigMsg.state=NORMAL_STATE;
  ConfigMsg.sw_ver[0]=FW_VER_HIGH;
  ConfigMsg.sw_ver[1]=FW_VER_LOW;  
}

