/*
******************************************************************************
* �� �� �� ��  AK745X_SPI.h
* ��    �� ��  AK745x spi����
* Ӳ��ƽ̨ ��  
******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef      _AK745X_SPI_H
  #define    _AK745X_SPI_H

/*
******************************************************************************
*��������˵��
******************************************************************************
*/
#ifdef      AK745X_SPI_GLOBALS
  #define   AK745X_SPI_EXT
#else
  #define   AK745X_SPI_EXT    extern
#endif
/*
******************************************************************************
*��������
******************************************************************************
*/
AK745X_SPI_EXT void AK745x_Spi_Gpio_Init(void);
AK745X_SPI_EXT u16 AK745x_Reg_Read(u8 Opcode, u8 Addr);
AK745X_SPI_EXT void AK745x_Reg_Write(u8 Opcode, u8 Addr, u16 Value);
AK745X_SPI_EXT void Entry_UserMode(void);
AK745X_SPI_EXT void Quit_UserMode(void);
/*
******************************************************************************
*WDG_TASK.h�������
******************************************************************************
*/
#endif