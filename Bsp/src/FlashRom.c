/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : FlashRom.c
* Author             : puhuafeng 
* Date First Issued  : 02/02/15
* Description        : FLASH驱动 C文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
#define FLASHROM_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "stm32f10x_flash.h"
#include "main.h"

static u8 s_byStmFlash_Buf[STM_SECTOR_SIZE];//最多是2K字节

/* Private functions ------------------------------------------------------- */
/*
该函数将从Addr开始的wlen字节的数据拷贝到pBuf中
*/
/*
*******************************************************************************
* Function Name  : FlashRead
* Description    : 读Flash
* Input          : u32 Addr, u8 *pBuf, u16 wlen
* Output         : None
* Return         : None
*******************************************************************************
*/
void FlashRead(u32 Addr, u8 *pBuf, u16 wlen)
{
    u8 *pData;
    
    pData = (u8 *)Addr;
    
    memcpy(pBuf, pData, wlen);
}

/*
*******************************************************************************
* Function Name  : STMFLASH_Write_NoCheck
* Description    : 无校验写Flash
* Input          : u32 WriteAddr,u8 *pBuffer,u16 NumToWrite
* Output         : None
* Return         : None
*******************************************************************************
*/
void STMFLASH_Write_NoCheck(u32 WriteAddr, u8 *pBuffer, u32 NumToWrite)
{ 
    u32 HalfWord;
    
    NumToWrite /= 2;  
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);  
    
    while(NumToWrite --)  
    {
         HalfWord = *(pBuffer++);
         HalfWord |= *(pBuffer++) << 8 ;
         //STMFLASH_WriteHalfWord(WriteAddr,HalfWord); 
         FLASH_ProgramHalfWord(WriteAddr,HalfWord); 
         WriteAddr += 2;//地址增加2. 
    } 
}

/*
*******************************************************************************
* Function Name  : FlashWrite
* Description    : 写Flash
* Input          : u32 WriteAddr,u8 *pBuffer,u16 NumToWrite
* Output         : None
* Return         : None
*******************************************************************************
*/
void FlashWrite(u32 WriteAddr, u8 *pBuffer, u16 NumToWrite)  
{
    u32 secpos;        //扇区地址
    u16 secoff;        //扇区内偏移地址(16位字计算)
    u16 secremain; //扇区内剩余地址(16位字计算)    
    u16 i = 0,j = 0;    
    
    u32 offaddr;   //去掉0X08000000后的地址
    
    if(WriteAddr < STM32_FLASH_BASE ||(WriteAddr >= (STM32_FLASH_BASE + 1024*STM32_FLASH_SIZE)))
    {
        return;//非法地址
    }
    
    //STMFLASH_Unlock();                                            //解锁
    FLASH_Unlock();
    
    offaddr = WriteAddr - STM32_FLASH_BASE;             //实际偏移地址.
    
    secpos = offaddr/STM_SECTOR_SIZE; //扇区地址  0~255 for STM32F103RET6
    
    secoff = (offaddr % STM_SECTOR_SIZE);  //在扇区内的偏移1个字节为基本单位.)
    
    secremain = STM_SECTOR_SIZE - secoff;                //扇区剩余空间大小   
    
    if(NumToWrite <= secremain)
    {
        secremain = NumToWrite;//不大于该扇区范围
    }
    
    while(1)    
    {         
        FlashRead(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE,s_byStmFlash_Buf,STM_SECTOR_SIZE);//读出整个扇区的内容
        //STMFLASH_Read(WriteAddr,s_byStmFlash_Buf,STM_SECTOR_SIZE);
        
        for(i = 0;i < secremain;i++)//校验数据
        {
            if(s_byStmFlash_Buf[secoff+i] != 0xFF) 
            {
                break;//需要擦除 
            }
            
            j=i+1;
            
            if(j == secremain) 
            {
                break;
            }
        }
  
       // if(i<secremain)//需要擦除
        if(j < secremain)
        {
            //STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
             FLASH_ErasePage(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE);
             
             for(i = 0;i < secremain; i++) 
             {
                s_byStmFlash_Buf[i+secoff] = pBuffer[i];//复制
             }
             
             STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE,s_byStmFlash_Buf,STM_SECTOR_SIZE);
  //写入整个扇区  
        }
        else 
        {
            STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);
        }
  //写已经擦除了的,直接写入扇区剩余区间.                              
  
        if(NumToWrite == secremain)              
        {
            break;//写入结束了
        }
        else//写入未结束
        {
            secpos++;                            //扇区地址增1
            secoff = 0;                      //偏移位置为0      
            pBuffer += secremain;    //指针偏移

            WriteAddr += secremain; //写地址偏移     

            NumToWrite -= secremain;      //字节(16位)数递减

            if(NumToWrite > (STM_SECTOR_SIZE))
            {
                secremain = STM_SECTOR_SIZE;
            }
//下一个扇区还是写不完
            else 
            {
                secremain = NumToWrite;//下一个扇区可以写完了
            }
        }     
    };    
    
    //STMFLASH_Lock();//上锁
    FLASH_Lock();

}

/*
*******************************************************************************
* Function Name  : RamhModify
* Description    : 数据拷贝
* Input          : u8 *pdes, u8 *psrc, u16 wlen
* Output         : None
* Return         : None
*******************************************************************************
*/
void RamhModify(u8 *pdes, u8 *psrc, u16 wlen)
{
   memcpy(pdes, psrc, wlen);
}
/************** (C) COPYRIGHT 2015 SOLAR SUPCON ************END OF FILE********/

