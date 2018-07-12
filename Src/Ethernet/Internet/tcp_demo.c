/**
************************************************************************************************
* @file   		tcp_demo.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief   		TCP ��ʾ����
* @attention  
************************************************************************************************
**/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "Ethernet/Internet/tcp_demo.h"
#include "Ethernet/W5500/W5500_conf.h"
#include "Ethernet/W5500/w5500.h"
#include "Ethernet/W5500/socket.h"
#include "Ethernet/Internet/dhcp.h"

static uint32_t  fac_us = 0;//us��ʱ������

uint32_t seq = 0;

uint8 server_ip[4] = {119,28,21,53};   ///{192,168,1,187};   				// ����Զ�̷�����IP��ַ
uint16 server_port = 1111;								// ����Զ�̷������˿�
uint16 local_port = 6000;									// ��ʼ��һ�����ض˿�
uint16 len = 0;

uint8 buffer[2048];	

void Systick_Init (uint8_t SYSCLK)
{
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTickƵ��ΪHCLK
	fac_us=SYSCLK;	
}								    

void Delay_ms( uint32_t time_ms )
{	 		  	  
	uint32_t i;
	for(i=0;i<time_ms;i++) 
	Delay_us(1000);  	    
}   
//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void Delay_us( uint32_t time_us )
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=time_us*fac_us; 						//��Ҫ�Ľ����� 
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};	 
}


/**
*@brief		do_dhcp_ip��ȡ��̬IP������
*@param		��
*@return	��
*/
void do_dhcp_ip(void)
{
   uint8 dhcpret=0;
    /* ��ѯDHCP״̬��־λ��ֵ
        dhcpretȡֵ��
        0			DHCP_RET_NONE
        1			DHCP_RET_ERR
        2			DHCP_RET_TIMEOUT
        3			DHCP_RET_UPDATE
        4			DHCP_RET_CONFLICT
    */
    dhcpret = check_DHCP_state(SOCK_DHCP);
    
    while(dhcpret != STATE_DHCP_LEASED)
    {
        printf("hello world %d\n",dhcpret);
        HAL_Delay(500);
        dhcpret = check_DHCP_state(SOCK_DHCP);
    }		
    switch(dhcpret)
    {
        case DHCP_RET_NONE:						// DHCP��־λΪ��
        break;
        case DHCP_RET_TIMEOUT:				// DHCP��ʱ
        break;
        case DHCP_RET_UPDATE:					// ��ȡ���µ�IP��ַ
                set_default();						// ����IP��ַ
                set_network();						// ����IP��ַ
        break;
        case DHCP_RET_CONFLICT:				// DHCP��־λ��ͻ
        while(1);
        default:
        break;
    }
}

/**
*@brief		TCP Server�ػ���ʾ������
*@param		��
*@return	��
*/
void do_tcp_server(void)
{	
	uint16 len=0;  
	switch(getSn_SR(SOCK_TCPS))											            	/*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:													                  /*socket���ڹر�״̬*/
			socket(SOCK_TCPS ,Sn_MR_TCP,local_port,Sn_MR_ND);	        /*��socket*/
		  break;     
    
		case SOCK_INIT:														                  /*socket�ѳ�ʼ��״̬*/
			listen(SOCK_TCPS);												                /*socket��������*/
		  break;
		
		case SOCK_ESTABLISHED:												              /*socket�������ӽ���״̬*/
		
			if(getSn_IR(SOCK_TCPS) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCPS, Sn_IR_CON);								          /*��������жϱ�־λ*/
			}
			len=getSn_RX_RSR(SOCK_TCPS);									            /*����lenΪ�ѽ������ݵĳ���*/
			if(len>0)
			{
				recv(SOCK_TCPS,buffer,len);								              	/*��������Client������*/
				buffer[len]=0x00; 											                  /*����ַ���������*/
				printf("%s\r\n",buffer);
				send(SOCK_TCPS,buffer,len);									              /*��Client��������*/
		  }
		  break;
		
		case SOCK_CLOSE_WAIT:												                /*socket���ڵȴ��ر�״̬*/
			close(SOCK_TCPS);
		  break;
	}
}

/**
*@brief		TCP Client�ػ���ʾ������
*@param		��
*@return	��
*/
void do_tcp_client(void)
{	
    /*Socket״̬����MCUͨ����Sn_SR(0)��ֵ�����ж�SocketӦ�ô��ں���״̬
        Sn_SR״̬������
        0x00		SOCK_CLOSED
        0x13		SOCK_INIT
        0x14		SOCK_LISTEN
        0x17		SOCK_ESTABLISHED
        0x1C		SOCK_CLOSE_WAIT
        0x22		SOCK_UDP
    */
    uint16_t SR = getSn_SR(SOCK_TCPC);
    
    printf("SR: %02x\r\n",SR);
    switch(SR)														// ��ȡsocket0��״̬
    {
        case SOCK_INIT:															// Socket���ڳ�ʼ�����(��)״̬
             printf("SOCK_INIT: %02x\r\n",SOCK_INIT);
             uint8_t ret = connect(SOCK_TCPC, server_ip,server_port);			// ����Sn_CRΪCONNECT������TCP������������������
             printf("connect ret: %02x\r\n",ret);
        break;
        case SOCK_ESTABLISHED:											// Socket�������ӽ���״̬
                if(getSn_IR(SOCK_TCPC) & Sn_IR_CON)   					
                {
                    printf("%d:Connected to - %d.%d.%d.%d : %d\r\n",SOCK_TCPC, server_ip[0], server_ip[1], server_ip[2], server_ip[3], server_port);
                    setSn_IR(SOCK_TCPC, Sn_IR_CON);								// Sn_IR��CONλ��1��֪ͨW5500�����ѽ���
                }
                // ���ݻػ����Գ������ݴ���λ������������W5500��W5500���յ����ݺ��ٻظ�������
                len=getSn_RX_RSR(SOCK_TCPC);										// len=Socket0���ջ������ѽ��պͱ�������ݴ�С
                if(len>0)
                {
                    recv(SOCK_TCPC,buffer,len);										// W5500�������Է����������ݣ���ͨ��SPI���͸�MCU
                    printf("%s\r\n",buffer);							// ���ڴ�ӡ���յ�������
                    send(SOCK_TCPC,buffer,len);										// ���յ����ݺ��ٻظ���������������ݻػ�
                }
                char buf[30];
    
                sprintf(buf,"helloworld123seq : %d",seq);
                
                seq++;
                
                send(SOCK_TCPC,(uint8_t *)buf,strlen(buf));
                printf(" send....����%s\r\n",buf);
                HAL_Delay(500);
        break;
        case SOCK_CLOSE_WAIT:												// Socket���ڵȴ��ر�״̬
                printf("Socket Server Close\r\n");      ///�������ر�
                close(SOCK_TCPC);																// �ر�Socket0
        break;
        case SOCK_CLOSED:														// Socket���ڹر�״̬
                printf("Socket Server Nonentity\r\n");   ///�����������ڻ���û����
                socket(SOCK_TCPC,Sn_MR_TCP,local_port,Sn_MR_ND);		// ��Socket0��������ΪTCP����ʱģʽ����һ�����ض˿�
                printf("Socket opened\r\n");
        break;
    }          
}

