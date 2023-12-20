/*
********************** (C) COPYRIGHT 2019 SOLAR SUPCON *************************
* File Name          : W25Qxx.c
* Author             : dingyongjian 
* Date First Issued  : 02/01/15
* Description        : 串口RS485通讯 C文件
********************************************************************************
* History:
* 02/01/19 v1.0
********************************************************************************
*/
#define W25QXX_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "main.h"


u16 W25QXX_TYPE = 0;	//默认是W25Q16
u8 g_byW25Qxx_WriteBuf[4096];
u8 g_byW25Qxx_ReadBuf[4096];

u8 g_byW25Qxx_Buf[4096];	
/* 移植本软件包时需要修改以下的函数或宏 */

/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : 初始化控制SSI的管脚
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

//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
  
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    
    SPI2->CR1 &= 0xFFC7;
    SPI2->CR1 |= SPI_BaudRatePrescaler;	//设置SPI2速度 
    SPI_Cmd(SPI2,ENABLE); 

} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{		
    u8 retry = 0;	
    
    while (RESET == SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        retry++;
        
        if(retry > 200)
        {
            return 0;
        }
    }	
    
    SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
    retry = 0;

    while (RESET == SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE))//检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        retry++;
        
        if(retry > 200)
        {
            return 0;
        }
    }	
    
    return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据					    
}


//初始化SPI FLASH的IO口
void W25QXX_Init(void)
{	    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能 
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // PB12 推挽 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_12);
    
    SPI_FLASH_CS_HIGH();		//SPI FLASH不选中
    SPI2_Init();		   	//初始化SPI
    SPI2_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式
    Delay_us(100);
 
    
    W25QXX_TYPE = W25QXX_ReadID();//读取FLASH ID. 
   
    if(W25X40 != W25QXX_TYPE)//设备ID读取失败
    {
        
    }
    else
    {
        
    }
    
    
}  

//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 W25QXX_ReadSR(void)   
{  
    u8 byte = 0; 
    
    SPI_FLASH_CS_LOW();                            //使能器件   
    SPI2_ReadWriteByte(W25X_ReadStatusReg); //发送读取状态寄存器命令
    
    byte = SPI2_ReadWriteByte(0xff);          //读取一个字节  
    SPI_FLASH_CS_HIGH();                            //取消片选     
    
    return byte;   
}

//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(u8 sr)   
{   
    SPI_FLASH_CS_LOW();                            //使能器件   
    SPI2_ReadWriteByte(W25X_WriteStatusReg);//发送写取状态寄存器命令    
    SPI2_ReadWriteByte(sr);               	//写入一个字节  
    SPI_FLASH_CS_HIGH();                            //取消片选     	      
}   

//W25QXX写使能	
//将WEL置位   
void W25QXX_Write_Enable(void)   
{
    SPI_FLASH_CS_LOW();                          	//使能器件   
    SPI2_ReadWriteByte(W25X_WriteEnable); 	//发送写使能  
    SPI_FLASH_CS_HIGH();                           	//取消片选     	      
} 

//W25QXX写禁止	
//将WEL清零  
void W25QXX_Write_Disable(void)   
{  
    SPI_FLASH_CS_LOW();                            //使能器件   
    SPI2_ReadWriteByte(W25X_WriteDisable);  //发送写禁止指令    
    SPI_FLASH_CS_HIGH();                            //取消片选     	      
} 	

//读取芯片ID
//返回值如下:				   
//0xEF13,表示芯片型号为W25Q80  
//0xEF14,表示芯片型号为W25Q16    
//0xEF15,表示芯片型号为W25Q32  
//0xEF16,表示芯片型号为W25Q64 
//0xEF17,表示芯片型号为W25Q128 	  
u16 W25QXX_ReadID(void)
{
    u16 Temp = 0;	
    
    SPI_FLASH_CS_LOW();	
    
    SPI2_ReadWriteByte(0x90);//发送读取ID命令	    
    SPI2_ReadWriteByte(0x00); 	    
    SPI2_ReadWriteByte(0x00); 	    
    SPI2_ReadWriteByte(0x00); 
    
    Temp |= SPI2_ReadWriteByte(0xFF) << 8;  
    Temp |= SPI2_ReadWriteByte(0xFF);	
    
    SPI_FLASH_CS_HIGH();	
    
    return Temp;
}   

//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
    u16 i;   		
    
    SPI_FLASH_CS_LOW();                            	//使能器件
    
    SPI2_ReadWriteByte(W25X_ReadData);         	//发送读取命令   
    SPI2_ReadWriteByte((u8)((ReadAddr) >> 16));  	//发送24bit地址    
    SPI2_ReadWriteByte((u8)((ReadAddr) >> 8));   
    SPI2_ReadWriteByte((u8)ReadAddr);   
    
    for(i = 0; i < NumByteToRead; i++)
    { 
        pBuffer[i] = SPI2_ReadWriteByte(0xFF);   	//循环读数  
    }
    
    SPI_FLASH_CS_HIGH();  				    	      
} 

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
    u16 i;
    
    W25QXX_Write_Enable();                  	//SET WEL 
    SPI_FLASH_CS_LOW();                            	//使能器件  
    
    SPI2_ReadWriteByte(W25X_PageProgram);      	//发送写页命令   
    SPI2_ReadWriteByte((u8)((WriteAddr) >> 16)); 	//发送24bit地址    
    SPI2_ReadWriteByte((u8)((WriteAddr) >> 8));   
    SPI2_ReadWriteByte((u8)WriteAddr);   
    
    for(i = 0;i < NumByteToWrite; i++)
    {
        SPI2_ReadWriteByte(pBuffer[i]);//循环写数  
    }
	
    SPI_FLASH_CS_HIGH();                            	//取消片选 	
    W25QXX_Wait_Busy();					   		//等待写入结束
} 

//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0xFF,否则在非0xFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
    u16 pageremain;
    
    pageremain = 256 - WriteAddr%256; //单页剩余的字节数	
    
    if(NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;//不大于256个字节
    }
    
    while(1)
    {	   
        W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
        
        if(NumByteToWrite == pageremain)
        {
            break;//写入结束了
        }
        else //NumByteToWrite>pageremain
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;	

            NumByteToWrite -= pageremain;			  //减去已经写入了的字节数
            
            if(NumByteToWrite > 256)
            {
                pageremain = 256; //一次可以写入256个字节
            }
            else 
            {
                pageremain = NumByteToWrite; 	  //不够256个字节了
            }
        }
    }	    
} 

//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)    
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
    u32 secpos;
    u16 secoff;
    u16 secremain;	   
    u16 i;    
    u8 * W25QXX_BUF;	
    
    W25QXX_BUF = g_byW25Qxx_Buf;	     
    secpos = WriteAddr/4096;//扇区地址  
    secoff = WriteAddr%4096;//在扇区内的偏移
    secremain = 4096 - secoff;//扇区剩余空间大小   
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
    
    if(NumByteToWrite <= secremain)
    {
        secremain = NumByteToWrite;//不大于4096个字节
    }
    
    while(1) 
    {	
        W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
        
        for(i = 0; i < secremain; i++)//校验数据
        {
            if(W25QXX_BUF[secoff+i] != 0xFF)
            {
                break;//需要擦除  
            }
        }
        
        if(i < secremain)//需要擦除
        {
            W25QXX_Erase_Sector(secpos);		//擦除这个扇区
            
            for(i  = 0; i < secremain; i++)	   		//复制
            {
                W25QXX_BUF[i+secoff] = pBuffer[i];	  
            }
            
            W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  

        }
        else 
        {
            W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 	
        }
        
        if(NumByteToWrite == secremain)
        {
            break;//写入结束了
        }
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff = 0;//偏移位置为0 	 

            pBuffer += secremain;  				//指针偏移
            WriteAddr += secremain;				//写地址偏移	   
            NumByteToWrite -= secremain;			//字节数递减
            
            if(NumByteToWrite > 4096)
            {
                secremain = 4096;//下一个扇区还是写不完
            }
            else 
            {
                secremain = NumByteToWrite;		//下一个扇区可以写完了
            }
        }	 
    } 
}

//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                 	 	//SET WEL 
    W25QXX_Wait_Busy();   
    SPI_FLASH_CS_LOW();                            	//使能器件   
    SPI2_ReadWriteByte(W25X_ChipErase);        	//发送片擦除命令  
    SPI_FLASH_CS_HIGH();                            	//取消片选     	      
    W25QXX_Wait_Busy();   				   		//等待芯片擦除结束
} 

//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
	//监视falsh擦除情况,测试用   
 	//printf("fe:%x\r\n",Dst_Addr);	  
    Dst_Addr *= 4096;
    
    W25QXX_Write_Enable();                  	//SET WEL 	 
    W25QXX_Wait_Busy();   
    SPI_FLASH_CS_LOW();                            	//使能器件   
    
    SPI2_ReadWriteByte(W25X_SectorErase);      	//发送扇区擦除指令 
    SPI2_ReadWriteByte((u8)((Dst_Addr) >> 16));  	//发送24bit地址    
    SPI2_ReadWriteByte((u8)((Dst_Addr) >> 8));   
    SPI2_ReadWriteByte((u8)Dst_Addr);  
    
    SPI_FLASH_CS_HIGH();                            	//取消片选     	      
    W25QXX_Wait_Busy();   				   		//等待擦除完成
} 

//等待空闲
void W25QXX_Wait_Busy(void)   
{   
    while(0x01 == (W25QXX_ReadSR() &0x01));  		// 等待BUSY位清空
}  

//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
    SPI_FLASH_CS_LOW();                           	 	//使能器件   
    SPI2_ReadWriteByte(W25X_PowerDown);        //发送掉电命令  
    SPI_FLASH_CS_HIGH();                            	//取消片选     	      
    //delay_us(3);                               //等待TPD  
}

//唤醒
void W25QXX_WAKEUP(void)   
{  
    SPI_FLASH_CS_LOW();                            	//使能器件   
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
    SPI_FLASH_CS_HIGH();                            	//取消片选     	      
    //delay_us(3);                            	//等待TRES1
}

//将4k内容全部读取，替换相应偏移地址内容后，再将4K重新写入，需修改函数
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