/*
********************** (C) COPYRIGHT 2015 SOLAR SUPCON *************************
* File Name          : LED_TASK.c
* Author             : dingyongjian 
* Date First Issued  : 02/02/15
* Description        : DRV8323驱动 C文件
********************************************************************************
* History:
* 02/02/15 v1.1
********************************************************************************
*/
#define AK745X_SPI_GLOBALS

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "AK745x_Spi.h"
#include "delay_time.h"


#define DELAY_TIME 1//单位us

#define SET_nSCS_LOW()   GPIO_ResetBits(GPIOA,GPIO_Pin_15)
#define SET_nSCS_HIGH()  GPIO_SetBits(GPIOA,GPIO_Pin_15)

#define SET_SCLK_LOW()   GPIO_ResetBits(GPIOB,GPIO_Pin_3)
#define SET_SCLK_HIGH()  GPIO_SetBits(GPIOB,GPIO_Pin_3)

#define SET_MOSI_LOW()   GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define SET_MOSI_HIGH()  GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define GET_MISO_DATA()  (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4))
/* Private functions ------------------------------------------------------- */
/*
******************************************************************************
* 函 数 名 ：  void AK745x_Spi_Gpio_Init(void)
* 描    述 ：  AK745xspi GPIO初始化
* 硬件连接：   -------------------------
                |         | 
               -------------------------
******************************************************************************
*/
void AK745x_Spi_Gpio_Init(void)//DRV8323 SPI-Driver Init
{
    GPIO_InitTypeDef GPIO_InitStructure;
   // SPI_InitTypeDef SPI_InitStructure;
    
    /*开启GPIOA时钟信号*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA,ENABLE );
    /*开启GPIOB时钟信号*/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB,ENABLE );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE); 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
 
    
  //模拟SPI模式
    //GPIOA SPI_nSCS输出：PA15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);
     Delay_us(100);
    GPIO_SetBits(GPIOA, GPIO_Pin_15);
    
    //GPIOA SPI_SCK_MOSI输出：SPI_SCK->PB3 SPI_MOSI->PB5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_5);
     //Delay_us(100);
    //GPIO_SetBits(GPIOB, GPIO_Pin_3);
    
     //SPI_MISO：PB4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
       
    //SPI模式
 
}


u16 AK745x_Reg_Read(u8 Opcode, u8 Addr)//Read DRV8323 REG
{
    u16 i = 0;
    u16 Read_Gpio = 0;
    u8  Shift_Addr = 0;
    u16 reading = 0x0000;
    
    Shift_Addr = (Addr << 1);
    
    SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW
    Delay_us(DELAY_TIME);
    SET_nSCS_LOW();	// uncomment if --> Set Low initially,nSCS->low
    Delay_us(DELAY_TIME);
    
    for(i = 0x0008;i >= 1;i >>= 0x0001 )
    {
        SET_SCLK_HIGH();		// uncomment if --> Set High initially,SPICLKA ->High
        Delay_us(DELAY_TIME);
        
        if((Opcode & i) >0)
        {
            SET_MOSI_HIGH();		// uncomment if --> Set High initially,SDO ->High
        }
        else
        {
            SET_MOSI_LOW();	// uncomment if --> Set Low initially,SDO ->LOW
        }
        
        Delay_us(DELAY_TIME);
        SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW   
        Delay_us(DELAY_TIME);	
        Delay_us(DELAY_TIME);		 
    }
    
    for(i = 0x0080;i >= 1;i >>= 0x0001 )
    {
        SET_SCLK_HIGH();		// uncomment if --> Set High initially,SPICLKA ->High
        Delay_us(DELAY_TIME);
        
        if((Shift_Addr & i) >0)
        {
            SET_MOSI_HIGH();		// uncomment if --> Set High initially,SDO ->High
        }
        else
        {
            SET_MOSI_LOW();	// uncomment if --> Set Low initially,SDO ->LOW
        }
        
        Delay_us(DELAY_TIME);
        SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW  
        Delay_us(DELAY_TIME);
        Delay_us(DELAY_TIME);		 
    }
    
    for(i = 0x0800;i >= 1;i >>= 0x0001 )
    {
        SET_SCLK_HIGH();		// uncomment if --> Set High initially,SPICLKA ->High
        Delay_us(DELAY_TIME);
        
        Delay_us(DELAY_TIME);
        SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW  
               
        
        Read_Gpio = GET_MISO_DATA();//Read MISO电平
        if(Read_Gpio > 0)
        {
            reading |= i;
        }
        else
        {
            reading &= ~i;
        }
    
        Delay_us(DELAY_TIME);
        Delay_us(DELAY_TIME);	
    }
    //Delay_us(10);
    SET_nSCS_HIGH();		// uncomment if --> Set High initially,nSCS->High
    Delay_us(DELAY_TIME);
    
    return reading;
}


/*************************************************************************************************************************************************
 *  AK745x_Reg_Write
 **************************************************************************************************************************************************/
/*!
 * @brief function call to write SPI register
 *
 * This function is an interface provided to the application to perform write to AK745x_Reg_Write registers.
 *
 * @parameter[in]	address	target reading address of AK745x_Reg_Write register
 * 					value 	write value to the target AK745x_Reg_Write register
 * @return  		reading	value of the target AK745x_Reg_Write read
 **************************************************************************************************************************************************/
void AK745x_Reg_Write(u8 Opcode, u8 Addr, u16 Value)//Write AK745x_Reg_Write REG
{
    u16 i = 0;
    //u16 Read_Gpio = 0;
    u8  Shift_Addr = 0;
   // u16 reading = 0x0000;
    
    Shift_Addr = (Addr << 1);
    
    SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW
    Delay_us(DELAY_TIME);
    SET_nSCS_LOW();	// uncomment if --> Set Low initially,nSCS->low
    Delay_us(DELAY_TIME);
       
    for(i = 0x0008; i >= 1; i >>= 0x0001 )
    {
        SET_SCLK_HIGH();		// uncomment if --> Set High initially,SPICLKA ->High
        Delay_us(DELAY_TIME);
        
        if((Opcode & i) > 0)
        {
            SET_MOSI_HIGH();		// uncomment if --> Set High initially,SDO ->High
        }
        else
        {
            SET_MOSI_LOW();	// uncomment if --> Set Low initially,SDO ->LOW
        }
        
        Delay_us(DELAY_TIME);
        SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW   
        Delay_us(DELAY_TIME);	
        Delay_us(DELAY_TIME);		 
    }
       
    for(i = 0x0080; i >= 1; i >>= 0x0001 )
    {
        SET_SCLK_HIGH();		// uncomment if --> Set High initially,SPICLKA ->High
        Delay_us(DELAY_TIME);
        
        if((Shift_Addr & i) > 0)
        {
            SET_MOSI_HIGH();		// uncomment if --> Set High initially,SDO ->High
        }
        else
        {
            SET_MOSI_LOW();	// uncomment if --> Set Low initially,SDO ->LOW
        }
        
        Delay_us(DELAY_TIME);
        SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW  
        Delay_us(DELAY_TIME);
        Delay_us(DELAY_TIME);		 
    }
    
    for(i = 0x0800; i >= 1; i >>= 0x0001 )
    {
        SET_SCLK_HIGH();		// uncomment if --> Set High initially,SPICLKA ->High
        Delay_us(DELAY_TIME);
        
        if((Value & i) > 0)
        {
            SET_MOSI_HIGH();		// uncomment if --> Set High initially,SDO ->High
        }
        else
        {
            SET_MOSI_LOW();	// uncomment if --> Set Low initially,SDO ->LOW
        }
        
        Delay_us(DELAY_TIME);
        SET_SCLK_LOW();	// uncomment if --> Set Low initially,SPICLKA ->LOW  
               
        Delay_us(DELAY_TIME);
        Delay_us(DELAY_TIME);	
    }
    
    SET_nSCS_HIGH();		// uncomment if --> Set High initially,nSCS->High
    Delay_us(DELAY_TIME);
  
}

void Entry_UserMode(void)
{
    AK745x_Reg_Write(0x05,0x03,0x050F);
}

void Quit_UserMode(void)
{
    AK745x_Reg_Write(0x05,0x03,0x0000);
}


/************** (C) COPYRIGHT 2015 SOLAR  ************END OF FILE********/
