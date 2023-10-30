/********************************** (C) COPYRIGHT  *******************************
 * File Name          : system_ch641.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file contains all the functions prototypes for UART
 *                      Printf , Delay functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include <system_ch641.h>

TMR_PARAM timer;
uint8_t volatile tmr_ms_cnt;
uint16_t volatile decode_tick;


/*********************************************************************
 * @fn      Delay_Us
 *
 * @brief   Microsecond Delay Time.
 *
 * @param   n - Microsecond number.
 *
 * @return  None
 */
void mDelayuS(uint16_t n)
{
    uint16_t i,k1,k2,dk;

    k1 = (uint16_t)SysTick->CNT;
    i = (n-1) * 6;
    while(1)
    {
        k2 = (uint16_t)SysTick->CNT;
        if(k2 >= k1)
        {
            dk = k2-k1;
        }
        else {
            dk = k2+6000-k1;
        }
        if(i > dk)
        {
            i = i-dk;
        }
        else {
            break;
        }
        k1 = k2;
    }
}

/*********************************************************************
 * @fn      Delay_Ms
 *
 * @brief   Millisecond Delay Time.
 *
 * @param   n - Millisecond number.
 *
 * @return  None
 */
void mDelaymS(uint16_t n)
{
    while ( n )
    {
        mDelayuS( 1000 );
        -- n;
    }
}

extern void QI_Recv_Tick_Proc( void );

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      SysTick_Handler
 *
 * @brief   Interrupt callback function.
 *
 * @return  None
 */
void SysTick_Handler( void )
{
    SysTick->SR = 0;
    decode_tick += 6000;
    tmr_ms_cnt++;
    QI_Recv_Tick_Proc();                    //Qi decoding timeout handling
    Tmr_ISR_Callback();                     //interrupt callback function
}

/*********************************************************************
 * @fn      Timer_Init
 *
 * @brief   Timer initialization.
 *
 * @return  None
 */
void Timer_Init( void )
{
    SysTick->CNT = 0;
    SysTick->CMP = 6000-1;
    SysTick->SR = 0;
    SysTick->CTLR = (1<<3) | (1<<1) | (1<<0);         //Auto-reload; HCLK/8; turn on interrupt; start counter
    NVIC_SetPriority(SysTicK_IRQn, 0x80 | 0x60);      //Only bit7 bit6 is valid, lowest priority. Non-preemptive and same priority as QII decoding.
    NVIC_EnableIRQ(SysTicK_IRQn);
}

#if DE_PRINTF
/*********************************************************************
 * @fn      USART_Printf_Init
 *
 * @brief   Initializes the USARTx peripheral.
 *
 * @param   baudrate - USART communication baud rate.
 *
 * @return  None
 */
void USART_Printf_Init(uint32_t baudrate)
{
    GPIOB->CFGLR |= 0xF0;           //PB1
    AFIO->PCFR1 &= ~(7<<2);
    AFIO->PCFR1 |= (1<<2);

    USART1->BRR = 417;             //BRR = DIV_M+(DIV_F/16) = 48000000/16/115200=26.041666,The result of the calculation is then multiplied by 16 and fed into the registers
    USART1->CTLR1 = 0x2008;        //UE=1£¬uart enable    TE=1£¬transmit enable
}
#endif


#if (DE_PRINTF == 1)
void uart_send_dat(uint8_t udat)
{
    while((USART1->STATR & USART_FLAG_TXE) == 0 );
    USART1->DATAR = udat;
}

void uart_send_buf(uint8_t *ubuf,uint8_t ulen)
{
    uint8_t i;
    for(i = 0;i!=ulen;i++)
    {
        uart_send_dat(ubuf[i]);
    }
}

#endif
#if (DE_PRINTF == 2)
/*********************************************************************
 * @fn      _write
 *
 * @brief   Support Printf Function
 *
 * @param   *buf - UART send Data.
 *          size - Data length.
 *
 * @return  size - Data length
 */
__attribute__((used)) 
int _write(int fd, char *buf, int size)
{
    int i;

    for(i = 0; i < size; i++){
        while((USART1->STATR & USART_FLAG_TXE) == 0);
        USART1->DATAR = *buf++;
    }

    return size;
}
#endif

/*********************************************************************
 * @fn      SystemInit
 *
 * @brief   Setup the microcontroller system Initialize the Embedded Flash Interface,
 *        the PLL and update the SystemCoreClock variable.
 *
 * @return  none
 */
void SystemInit(void)
{
    RCC->CTLR |= (uint32_t)0x00000001;
    RCC->CFGR0 = (0x38<<11);           //ADC 48 crossover 1M
    RCC->INTR = 0x00140000;

    /* Flash 0 wait state */
    FLASH->ACTLR &= (uint32_t)((uint32_t)~FLASH_ACTLR_LATENCY);
    FLASH->ACTLR |= (uint32_t)FLASH_ACTLR_LATENCY_1;

    /* Enable PLL */
    RCC->CTLR |= RCC_PLLON;
    /* Wait till PLL is ready */
    while((RCC->CTLR & RCC_PLLRDY) == 0)
    {
    }
    /* Select PLL as system clock source */
    RCC->CFGR0 &= (uint32_t)((uint32_t)~(RCC_SW));
    RCC->CFGR0 |= (uint32_t)RCC_SW_PLL;
    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR0 & (uint32_t)RCC_SWS) != (uint32_t)0x08)
    {
    }

    //Trimmed HSI, 24MHZ
//    RCC->CTLR = (RCC->CTLR & 0xFFFFFF07) | (13<<3); //Default value 16, take the value 0~31, one frame 60KZ, plus or minus 4% adjustment.
    //Corresponding to 127K, that's one frame change 60K*2/376 = 320hz
}

/*********************************************************************
 * @fn      Enable_Globle_IRQ (re-write function in core_riscv.h)
 *
 * @brief   Enable Global Interrupt
 *
 * @return  none
 */
void Enable_Globle_IRQ()
{
  uint32_t result;

  __asm volatile("csrr %0," "mstatus": "=r"(result));
  result |= 0x88;
  __asm volatile ("csrw mstatus, %0" : : "r" (result) );
}

/*********************************************************************
 * @fn      Disable_Globle_IRQ (re-write function in core_riscv.h)
 *
 * @brief   Disable Global Interrupt
 *
 * @return  none
 */
void Disable_Globle_IRQ()
{
  uint32_t result;

  __asm volatile("csrr %0," "mstatus": "=r"(result));
  result &= ~0x88;
  __asm volatile ("csrw mstatus, %0" : : "r" (result) );
}

/*********************************************************************
 * @fn      _sbrk
 *
 * @brief   Change the spatial position of data segment.
 *
 * @return  size: Data length
 */
void *_sbrk(ptrdiff_t incr)
{
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
    return NULL - 1;

    curbrk += incr;
    return curbrk -incr;
}


