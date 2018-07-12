/*
**************************************************************************************************
* @file    		w5500_conf.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief  		����MCU����ֲW5500������Ҫ�޸ĵ��ļ�������W5500��MAC��IP��ַ
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

uint8 txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};		// ѡ��8��Socketÿ��Socket���ͻ���Ĵ�С����w5500.c��void sysinit()�����ù���
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};		// ѡ��8��Socketÿ��Socket���ջ���Ĵ�С����w5500.c��void sysinit()�����ù���

extern uint8 MAC[6];

uint8 pub_buf[1460];

uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
uint8	ip_from_dhcp=1;
uint8   ip_from_define=0;

/*����Զ��IP��Ϣ*/
//uint8  remote_ip[4]={120,77,213,80};			  						/*Զ��IP��ַ*/
//uint16 remote_port=3099;																/*Զ�˶˿ں�*/

uint8  remote_ip[4]={119,28,21,53};			  						/*Զ��IP��ַ*/
uint16 remote_port=1111;	

//uint8  remote_ip[4]={192,168,1,100};			  						/*Զ��IP��ַ*/
//uint16 remote_port=8080;

extern void Delay_us( uint32 time_us );
/**
*@brief		Ӳ����λW5500
*@param		��
*@return	��
*/
void reset_w5500(void)
{
  HAL_GPIO_WritePin(W5500_SCS_GPIO_Port, W5500_SCS_Pin,GPIO_PIN_RESET);
  Delay_us(500);  //��������500us
  HAL_GPIO_WritePin(W5500_SCS_GPIO_Port, W5500_SCS_Pin,GPIO_PIN_SET);
  HAL_Delay(1600);
}


/**
*@brief		W5500Ƭѡ�ź����ú���
*@param		val: Ϊ��0����ʾƬѡ�˿�Ϊ�ͣ�Ϊ��1����ʾƬѡ�˿�Ϊ��
*@return	��
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

void set_network(void)															// ���ó�ʼ��IP��Ϣ����ӡ����ʼ��8��Socket
{
  uint8 ip[6];
  setSHAR(ConfigMsg.mac);
  setSUBR(ConfigMsg.sub);
  setGAR(ConfigMsg.gw);
  setSIPR(ConfigMsg.lip);

  sysinit(txsize, rxsize); 													// ��ʼ��8��socket
  setRTR(2000);																	// �������ʱ��
  setRCR(3);																	// ����������·��ʹ���
  
  getSHAR (ip);
  printf("MAC : %02x.%02x.%02x.%02x.%02x.%02x\r\n", ip[0],ip[1],ip[2],ip[3],ip[4],ip[5]);
  getSIPR (ip);
  printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getSUBR(ip);
  printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getGAR(ip);
  printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
}

void set_default(void)															// ����Ĭ��MAC��IP��GW��SUB��DNS
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

