/********************************** (C) COPYRIGHT  *******************************
 * File Name          : iap.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/08/28
 * Description        : IAP
 *******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "iap.h"
#include "string.h"
#include "flash.h"
#include "core_riscv.h"

/******************************************************************************/

#define FLASH_Base   0x08000000
#define USBD_DATA_SIZE   64
vu32 Program_addr = FLASH_Base;
vu32 Verity_addr = FLASH_Base;
vu8 Verity_Star_flag = 0;
vu8 Fast_Program_Buf[128];
vu16 CodeLen = 0;
vu8 End_Flag = 0;
vu8 EP2_Rx_Buffer[USBD_DATA_SIZE];
#define  isp_cmd_t   ((isp_cmd  *)EP2_Rx_Buffer)

/*********************************************************************
 * @fn      USART1_CFG
 *
 * @brief   GPIOA-USART1 init
 *
 * @return
 */
void USART1_CFG(void)
{
    GPIOA->CFGHR = 0X4448B444;/* Set GPIOA Mode,Speed,USART_Parity */
    GPIOA->BCR = (((uint32_t)0x01) << 12);
    USART1->CTLR2 |= USART_StopBits_1;
    USART1->CTLR1 = 0X200C; /* Set USART mode,WordLength,GPIO_Pin */
    USART1->CTLR3 |= USART_HardwareFlowControl_None;
    USART1->BRR = 0X34;  /* Set 460800 baudrate; SystemCoreClock = SYSCLK_FREQ_24MHZ_HSI*/
    USART1->CTLR1 |= ((uint16_t)0x2000); /* Enables the specified USART peripheral */
}

/*********************************************************************
 * @fn      RecData_Deal
 *
 * @brief   UART-USB
 *
 * @return  ERR_ERROR - ERROR
 *          ERR_SCUESS - SCUESS
 *          ERR_End - End
 */
u8 RecData_Deal(void)
{
    u8 i, s, Lenth;

    Lenth = isp_cmd_t->Len;

    switch ( isp_cmd_t->Cmd){
        case CMD_IAP_ERASE:
            FLASH_Unlock_Fast();
            FLASH_EraseAllPages();
            s = ERR_SCUESS;
            break;

        case CMD_IAP_PROM:
            for(i = 0; i < Lenth; i++){
                Fast_Program_Buf[CodeLen + i] = isp_cmd_t->data[i];
            }
            CodeLen += Lenth;
            if(CodeLen >= 64){
                CH32_IAP_Program(Program_addr, (u32*) Fast_Program_Buf);
                CodeLen -= 64;
                for(i = 0; i < CodeLen; i++){
                    Fast_Program_Buf[i] = Fast_Program_Buf[64 + i];
                }

                Program_addr += 0x40;
            }
            s = ERR_SCUESS;
            break;

        case CMD_IAP_VERIFY:
            if(Verity_Star_flag == 0) {
                Verity_Star_flag = 1;

                for(i = 0; i < (64 - CodeLen); i++){
                    Fast_Program_Buf[CodeLen + i] = 0xFF;
                }

                CH32_IAP_Program(Program_addr, (u32*) Fast_Program_Buf);
                CodeLen = 0;
            }

            s = ERR_SCUESS;
            for(i = 0; i < Lenth; i++){
                if (isp_cmd_t->data[i] != *(u8*) (Verity_addr + i)){
                    s = ERR_ERROR;
                    break;
                }
            }

            Verity_addr += Lenth;

            break;

        case CMD_IAP_END:
            Verity_Star_flag = 0;
            End_Flag = 1;
            Program_addr = FLASH_Base;
            Verity_addr = FLASH_Base;

            s = ERR_End;
            break;

        default:
            s = ERR_ERROR;
            break;
    }

    return s;
}

/*********************************************************************
 * @fn      GPIO_Cfg_init
 *
 * @brief   GPIOA init
 *
 * @return  none
 */
void GPIO_Cfg_init(void)
{
   GPIOA->CFGLR = 0x800;
   GPIOA->BSHR = ((uint32_t)0x04);
}

/*********************************************************************
 * @fn      PA2_Check
 *
 * @brief   Check PA2 state
 *
 * @return  1 - IAP
 *          0 - APP
 */
u8 PA2_Check(void)
{
    GPIO_Cfg_init();

    if((GPIOA->INDR & GPIO_Pin_2) != (uint32_t)Bit_RESET)
    {
        return (uint8_t)Bit_SET;
    }
    else
    {
        return (uint8_t)Bit_RESET;
    }
}


/*********************************************************************
 * @fn      UART1_SendMultiyData
 *
 * @brief   Deal UART1 data.
 *
 * @param   l- Data length.
 *
 * @return  none
 */
void UART1_SendMultiyData(u8* pbuf, u8 num)
{
    u8 i = 0;

    while(i<num)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART_SendData(USART1, pbuf[i]);
        i++;
    }
}
/*********************************************************************
 * @fn      UART1_SendMultiyData
 *
 * @brief   USART1 send data
 *
 * @param   pbuf - Packet to be sent
 *          num - Number of data sent
 *
 * @return  none
 */
void UART1_SendData(u8 data)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    USART_SendData(USART1, data);
}

/*********************************************************************
 * @fn      Uart1_Rx
 *
 * @brief   Uart1 receive date
 *
 * @return  none
 */
u8 Uart1_Rx(void)
{
    while( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData( USART1);
}

/*********************************************************************
 * @fn      UART_Rx_Deal
 *
 * @brief   UART Rx data deal
 *
 * @return  none
 */
void UART_Rx_Deal(void)
{
    u8 i, s;
    u8 Data_add = 0;

    if(Uart1_Rx() == Uart_Sync_Head1)
    {
        if(Uart1_Rx() == Uart_Sync_Head2)
        {
            isp_cmd_t->Cmd = Uart1_Rx();
            Data_add += isp_cmd_t->Cmd;
            isp_cmd_t->Len = Uart1_Rx();
            Data_add += isp_cmd_t->Len;
            isp_cmd_t->Rev[0] = Uart1_Rx();
            Data_add += isp_cmd_t->Rev[0];
            isp_cmd_t->Rev[1] = Uart1_Rx();
            Data_add += isp_cmd_t->Rev[1];

            if((isp_cmd_t->Cmd == CMD_IAP_PROM)
                    || (isp_cmd_t->Cmd == CMD_IAP_VERIFY))
            {
                for (i = 0; i < isp_cmd_t->Len; i++) {
                    isp_cmd_t->data[i] = Uart1_Rx();
                    Data_add += isp_cmd_t->data[i];
                }
            }

            if(Uart1_Rx() == Data_add)
            {
                s = RecData_Deal();

                if(s != ERR_End)
                {
                    UART1_SendData(0x00);
                    if(s == ERR_ERROR)
                    {
                        UART1_SendData(0x01);
                    }
                    else
                    {
                        UART1_SendData(0x00);
                    }
                }
            }
        }
    }
}

