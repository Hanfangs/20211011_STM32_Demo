/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : FlashRom.c
* Author             : puhuafeng 
* Date First Issued  : 02/02/15
* Description        : FLASH���� C�ļ�
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

static u8 s_byStmFlash_Buf[STM_SECTOR_SIZE];//�����2K�ֽ�

/* Private functions ------------------------------------------------------- */
/*
�ú�������Addr��ʼ��wlen�ֽڵ����ݿ�����pBuf��
*/
/*
*******************************************************************************
* Function Name  : FlashRead
* Description    : ��Flash
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
* Description    : ��У��дFlash
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
         WriteAddr += 2;//��ַ����2. 
    } 
}

/*
*******************************************************************************
* Function Name  : FlashWrite
* Description    : дFlash
* Input          : u32 WriteAddr,u8 *pBuffer,u16 NumToWrite
* Output         : None
* Return         : None
*******************************************************************************
*/
void FlashWrite(u32 WriteAddr, u8 *pBuffer, u16 NumToWrite)  
{
    u32 secpos;        //������ַ
    u16 secoff;        //������ƫ�Ƶ�ַ(16λ�ּ���)
    u16 secremain; //������ʣ���ַ(16λ�ּ���)    
    u16 i = 0,j = 0;    
    
    u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
    
    if(WriteAddr < STM32_FLASH_BASE ||(WriteAddr >= (STM32_FLASH_BASE + 1024*STM32_FLASH_SIZE)))
    {
        return;//�Ƿ���ַ
    }
    
    //STMFLASH_Unlock();                                            //����
    FLASH_Unlock();
    
    offaddr = WriteAddr - STM32_FLASH_BASE;             //ʵ��ƫ�Ƶ�ַ.
    
    secpos = offaddr/STM_SECTOR_SIZE; //������ַ  0~255 for STM32F103RET6
    
    secoff = (offaddr % STM_SECTOR_SIZE);  //�������ڵ�ƫ��1���ֽ�Ϊ������λ.)
    
    secremain = STM_SECTOR_SIZE - secoff;                //����ʣ��ռ��С   
    
    if(NumToWrite <= secremain)
    {
        secremain = NumToWrite;//�����ڸ�������Χ
    }
    
    while(1)    
    {         
        FlashRead(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE,s_byStmFlash_Buf,STM_SECTOR_SIZE);//������������������
        //STMFLASH_Read(WriteAddr,s_byStmFlash_Buf,STM_SECTOR_SIZE);
        
        for(i = 0;i < secremain;i++)//У������
        {
            if(s_byStmFlash_Buf[secoff+i] != 0xFF) 
            {
                break;//��Ҫ���� 
            }
            
            j=i+1;
            
            if(j == secremain) 
            {
                break;
            }
        }
  
       // if(i<secremain)//��Ҫ����
        if(j < secremain)
        {
            //STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
             FLASH_ErasePage(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE);
             
             for(i = 0;i < secremain; i++) 
             {
                s_byStmFlash_Buf[i+secoff] = pBuffer[i];//����
             }
             
             STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE,s_byStmFlash_Buf,STM_SECTOR_SIZE);
  //д����������  
        }
        else 
        {
            STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);
        }
  //д�Ѿ������˵�,ֱ��д������ʣ������.                              
  
        if(NumToWrite == secremain)              
        {
            break;//д�������
        }
        else//д��δ����
        {
            secpos++;                            //������ַ��1
            secoff = 0;                      //ƫ��λ��Ϊ0      
            pBuffer += secremain;    //ָ��ƫ��

            WriteAddr += secremain; //д��ַƫ��     

            NumToWrite -= secremain;      //�ֽ�(16λ)���ݼ�

            if(NumToWrite > (STM_SECTOR_SIZE))
            {
                secremain = STM_SECTOR_SIZE;
            }
//��һ����������д����
            else 
            {
                secremain = NumToWrite;//��һ����������д����
            }
        }     
    };    
    
    //STMFLASH_Lock();//����
    FLASH_Lock();

}

/*
*******************************************************************************
* Function Name  : RamhModify
* Description    : ���ݿ���
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

