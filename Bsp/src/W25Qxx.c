/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : W25Qxx.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : ����RS485ͨѶ C�ļ�
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
#define W25QXX_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "main.h"


u16 W25QXX_TYPE = 0;	//Ĭ����W25Q16
u8 g_byW25Qxx_WriteBuf[4096];
u8 g_byW25Qxx_ReadBuf[4096];

u8 g_byW25Qxx_Buf[4096];	
/* ��ֲ�������ʱ��Ҫ�޸����µĺ������ */

/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : ��ʼ������SSI�Ĺܽ�
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void SPI2_Init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
  

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE); 

    
   
    //CLK/MISO/MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);								  
   

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 |GPIO_Pin_7;  //WP/CS2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);	
    GPIO_SetBits(GPIOC,GPIO_Pin_6 |GPIO_Pin_7);
    /* SPI2 Config -------------------------------------------------------------*/ 								  
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; 
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; 
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);
    /* Enable SPI2 */ 
    SPI_Cmd(SPI2, ENABLE); 
}

//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
  
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    
    SPI2->CR1 &= 0xFFC7;
    SPI2->CR1 |= SPI_BaudRatePrescaler;	//����SPI2�ٶ� 
    SPI_Cmd(SPI2,ENABLE); 

} 

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI2_ReadWriteByte(u8 TxData)
{		
    u8 retry = 0;	
    
    while (RESET == SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
    {
        retry++;
        
        if(retry > 200)
        {
            return 0;
        }
    }	
    
    SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
    retry = 0;

    while (RESET == SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE))//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
    {
        retry++;
        
        if(retry > 200)
        {
            return 0;
        }
    }	
    
    return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����					    
}


//��ʼ��SPI FLASH��IO��
void W25QXX_Init(void)
{	    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ�� 
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 ���� 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_12);
    
    SPI_FLASH_CS_HIGH();		//SPI FLASH��ѡ��
    SPI2_Init();		   	//��ʼ��SPI
    SPI2_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ
    Delay_us(100);
 
    
    W25QXX_TYPE = W25QXX_ReadID();//��ȡFLASH ID. 
   
    if(W25X40 != W25QXX_TYPE)//�豸ID��ȡʧ��
    {
        
    }
    else
    {
        
    }
    
    
}  

//��ȡW25QXX��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 W25QXX_ReadSR(void)   
{  
    u8 byte = 0; 
    
    SPI_FLASH_CS_LOW();                            //ʹ������   
    SPI2_ReadWriteByte(W25X_ReadStatusReg); //���Ͷ�ȡ״̬�Ĵ�������
    
    byte = SPI2_ReadWriteByte(0xff);          //��ȡһ���ֽ�  
    SPI_FLASH_CS_HIGH();                            //ȡ��Ƭѡ     
    
    return byte;   
}

//дW25QXX״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void W25QXX_Write_SR(u8 sr)   
{   
    SPI_FLASH_CS_LOW();                            //ʹ������   
    SPI2_ReadWriteByte(W25X_WriteStatusReg);//����дȡ״̬�Ĵ�������    
    SPI2_ReadWriteByte(sr);               	//д��һ���ֽ�  
    SPI_FLASH_CS_HIGH();                            //ȡ��Ƭѡ     	      
}   

//W25QXXдʹ��	
//��WEL��λ   
void W25QXX_Write_Enable(void)   
{
    SPI_FLASH_CS_LOW();                          	//ʹ������   
    SPI2_ReadWriteByte(W25X_WriteEnable); 	//����дʹ��  
    SPI_FLASH_CS_HIGH();                           	//ȡ��Ƭѡ     	      
} 

//W25QXXд��ֹ	
//��WEL����  
void W25QXX_Write_Disable(void)   
{  
    SPI_FLASH_CS_LOW();                            //ʹ������   
    SPI2_ReadWriteByte(W25X_WriteDisable);  //����д��ָֹ��    
    SPI_FLASH_CS_HIGH();                            //ȡ��Ƭѡ     	      
} 	

//��ȡоƬID
//����ֵ����:				   
//0xEF13,��ʾоƬ�ͺ�ΪW25Q80  
//0xEF14,��ʾоƬ�ͺ�ΪW25Q16    
//0xEF15,��ʾоƬ�ͺ�ΪW25Q32  
//0xEF16,��ʾоƬ�ͺ�ΪW25Q64 
//0xEF17,��ʾоƬ�ͺ�ΪW25Q128 	  
u16 W25QXX_ReadID(void)
{
    u16 Temp = 0;	
    
    SPI_FLASH_CS_LOW();	
    
    SPI2_ReadWriteByte(0x90);//���Ͷ�ȡID����	    
    SPI2_ReadWriteByte(0x00); 	    
    SPI2_ReadWriteByte(0x00); 	    
    SPI2_ReadWriteByte(0x00); 
    
    Temp |= SPI2_ReadWriteByte(0xFF) << 8;  
    Temp |= SPI2_ReadWriteByte(0xFF);	
    
    SPI_FLASH_CS_HIGH();	
    
    return Temp;
}   

//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
    u16 i;   		
    
    SPI_FLASH_CS_LOW();                            	//ʹ������
    
    SPI2_ReadWriteByte(W25X_ReadData);         	//���Ͷ�ȡ����   
    SPI2_ReadWriteByte((u8)((ReadAddr) >> 16));  	//����24bit��ַ    
    SPI2_ReadWriteByte((u8)((ReadAddr) >> 8));   
    SPI2_ReadWriteByte((u8)ReadAddr);   
    
    for(i = 0; i < NumByteToRead; i++)
    { 
        pBuffer[i] = SPI2_ReadWriteByte(0xFF);   	//ѭ������  
    }
    
    SPI_FLASH_CS_HIGH();  				    	      
} 

//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
    u16 i;
    
    W25QXX_Write_Enable();                  	//SET WEL 
    SPI_FLASH_CS_LOW();                            	//ʹ������  
    
    SPI2_ReadWriteByte(W25X_PageProgram);      	//����дҳ����   
    SPI2_ReadWriteByte((u8)((WriteAddr) >> 16)); 	//����24bit��ַ    
    SPI2_ReadWriteByte((u8)((WriteAddr) >> 8));   
    SPI2_ReadWriteByte((u8)WriteAddr);   
    
    for(i = 0;i < NumByteToWrite; i++)
    {
        SPI2_ReadWriteByte(pBuffer[i]);//ѭ��д��  
    }
	
    SPI_FLASH_CS_HIGH();                            	//ȡ��Ƭѡ 	
    W25QXX_Wait_Busy();					   		//�ȴ�д�����
} 

//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0xFF,�����ڷ�0xFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
    u16 pageremain;
    
    pageremain = 256 - WriteAddr%256; //��ҳʣ����ֽ���	
    
    if(NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;//������256���ֽ�
    }
    
    while(1)
    {	   
        W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
        
        if(NumByteToWrite == pageremain)
        {
            break;//д�������
        }
        else //NumByteToWrite>pageremain
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;	

            NumByteToWrite -= pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
            
            if(NumByteToWrite > 256)
            {
                pageremain = 256; //һ�ο���д��256���ֽ�
            }
            else 
            {
                pageremain = NumByteToWrite; 	  //����256���ֽ���
            }
        }
    }	    
} 

//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535)    
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
    u32 secpos;
    u16 secoff;
    u16 secremain;	   
    u16 i;    
    u8 * W25QXX_BUF;	
    
    W25QXX_BUF = g_byW25Qxx_Buf;	     
    secpos = WriteAddr/4096;//������ַ  
    secoff = WriteAddr%4096;//�������ڵ�ƫ��
    secremain = 4096 - secoff;//����ʣ��ռ��С   
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
    
    if(NumByteToWrite <= secremain)
    {
        secremain = NumByteToWrite;//������4096���ֽ�
    }
    
    while(1) 
    {	
        W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//������������������
        
        for(i = 0; i < secremain; i++)//У������
        {
            if(W25QXX_BUF[secoff+i] != 0xFF)
            {
                break;//��Ҫ����  
            }
        }
        
        if(i < secremain)//��Ҫ����
        {
            W25QXX_Erase_Sector(secpos);		//�����������
            
            for(i  = 0; i < secremain; i++)	   		//����
            {
                W25QXX_BUF[i+secoff] = pBuffer[i];	  
            }
            
            W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//д����������  

        }
        else 
        {
            W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 	
        }
        
        if(NumByteToWrite == secremain)
        {
            break;//д�������
        }
        else//д��δ����
        {
            secpos++;//������ַ��1
            secoff = 0;//ƫ��λ��Ϊ0 	 

            pBuffer += secremain;  				//ָ��ƫ��
            WriteAddr += secremain;				//д��ַƫ��	   
            NumByteToWrite -= secremain;			//�ֽ����ݼ�
            
            if(NumByteToWrite > 4096)
            {
                secremain = 4096;//��һ����������д����
            }
            else 
            {
                secremain = NumByteToWrite;		//��һ����������д����
            }
        }	 
    } 
}

//��������оƬ		  
//�ȴ�ʱ�䳬��...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                 	 	//SET WEL 
    W25QXX_Wait_Busy();   
    SPI_FLASH_CS_LOW();                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_ChipErase);        	//����Ƭ��������  
    SPI_FLASH_CS_HIGH();                            	//ȡ��Ƭѡ     	      
    W25QXX_Wait_Busy();   				   		//�ȴ�оƬ��������
} 

//����һ������
//Dst_Addr:������ַ ����ʵ����������
//����һ��ɽ��������ʱ��:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
	//����falsh�������,������   
 	//printf("fe:%x\r\n",Dst_Addr);	  
    Dst_Addr *= 4096;
    
    W25QXX_Write_Enable();                  	//SET WEL 	 
    W25QXX_Wait_Busy();   
    SPI_FLASH_CS_LOW();                            	//ʹ������   
    
    SPI2_ReadWriteByte(W25X_SectorErase);      	//������������ָ�� 
    SPI2_ReadWriteByte((u8)((Dst_Addr) >> 16));  	//����24bit��ַ    
    SPI2_ReadWriteByte((u8)((Dst_Addr) >> 8));   
    SPI2_ReadWriteByte((u8)Dst_Addr);  
    
    SPI_FLASH_CS_HIGH();                            	//ȡ��Ƭѡ     	      
    W25QXX_Wait_Busy();   				   		//�ȴ��������
} 

//�ȴ�����
void W25QXX_Wait_Busy(void)   
{   
    while(0x01 == (W25QXX_ReadSR() &0x01));  		// �ȴ�BUSYλ���
}  

//�������ģʽ
void W25QXX_PowerDown(void)   
{ 
    SPI_FLASH_CS_LOW();                           	 	//ʹ������   
    SPI2_ReadWriteByte(W25X_PowerDown);        //���͵�������  
    SPI_FLASH_CS_HIGH();                            	//ȡ��Ƭѡ     	      
    //delay_us(3);                               //�ȴ�TPD  
}

//����
void W25QXX_WAKEUP(void)   
{  
    SPI_FLASH_CS_LOW();                            	//ʹ������   
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
    SPI_FLASH_CS_HIGH();                            	//ȡ��Ƭѡ     	      
    //delay_us(3);                            	//�ȴ�TRES1
}

//��4k����ȫ����ȡ���滻��Ӧƫ�Ƶ�ַ���ݺ��ٽ�4K����д�룬���޸ĺ���
void Spi_Flash_MWrite(u8 *pbuf,u32 Addr,u16 wlen)
{
    u32 AddrTemp;
    AddrTemp = (Addr/4096)*4096;
    W25QXX_Read(g_byW25Qxx_ReadBuf,AddrTemp,4096);
    W25QXX_Erase_Sector(Addr/4096);
    AddrTemp = Addr%4096;
    RamhModify(g_byW25Qxx_ReadBuf+AddrTemp,pbuf,wlen);
    AddrTemp = (Addr/4096)*4096;
    W25QXX_Write(g_byW25Qxx_ReadBuf,AddrTemp,4096);
}