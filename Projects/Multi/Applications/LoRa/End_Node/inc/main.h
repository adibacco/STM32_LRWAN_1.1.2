/**
  ******************************************************************************
  * @file    Applications/WriteURI/Inc/main.h 
  * @author  MMY Application Team
  * @version V1.2.0
  * @date    20-January-2015
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "lib_TagType4.h"
#include "hw_conf.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define wait_ms(time) HAL_Delay(time)

/* M24SR GPIO mapping -------------------------------------------------------------------------*/
/* PB8 --> D15 SCL */
/* PB9 --> D14 SDA */

#define M24SR_SDA_PIN 														GPIO_PIN_9
#define M24SR_SDA_PIN_PORT 												GPIOB
#define M24SR_SCL_PIN 														GPIO_PIN_8
#define M24SR_SCL_PIN_PORT 												GPIOB
#define M24SR_GPO_PIN 														GPIO_PIN_6
#define M24SR_GPO_PIN_PORT 												GPIOA
#define M24SR_RFDIS_PIN 													GPIO_PIN_7
#define M24SR_RFDIS_PIN_PORT 											GPIOA


#if (defined USE_STM32F0XX_NUCLEO || defined USE_STM32F1XX_NUCLEO || defined USE_STM32F3XX_NUCLEO)
#define __GPIOA_CLK_ENABLE() 								__HAL_RCC_GPIOA_CLK_ENABLE()
#define __GPIOB_CLK_ENABLE() 								__HAL_RCC_GPIOB_CLK_ENABLE()
#define INIT_CLK_GPO_RFD() 								__HAL_RCC_GPIOA_CLK_ENABLE()
#define I2Cx_CLK_ENABLE()                   			 	                __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       						__HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       						__HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C1_FORCE_RESET()                                                              __HAL_RCC_I2C1_FORCE_RESET()
#define I2C1_RELEASE_RESET()                                                            __HAL_RCC_I2C1_RELEASE_RESET()
#else
#define INIT_CLK_GPO_RFD() 								__GPIOA_CLK_ENABLE()
#define I2Cx_CLK_ENABLE()                   			 	                __I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       						__GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       						__GPIOB_CLK_ENABLE()
#define I2C1_FORCE_RESET()               						__I2C1_FORCE_RESET()
#define I2C1_RELEASE_RESET()             						__I2C1_RELEASE_RESET()
#endif

#if (defined USE_STM32F0XX_NUCLEO || defined USE_STM32F1XX_NUCLEO || defined USE_STM32F3XX_NUCLEO)
#define GPIO_SPEED_HIGH                                                                 GPIO_SPEED_FREQ_HIGH
#endif

/* 	I2C config	------------------------------------------------------------------------------*/
#define M24SR_I2C                  					       	I2C1



/* I2C functionality is not mapped on the same Alternate function regarding the MCU used */
#if (defined USE_STM32F4XX_NUCLEO) || (defined USE_STM32F3XX_NUCLEO) || (defined USE_B_L072Z_LRWAN1) || \
    (defined USE_STM32L0XX_NUCLEO) || (defined USE_STM32L1XX_NUCLEO) || (defined USE_STM32L4XX_NUCLEO)
  #define I2Cx_SCL_AF 											      	GPIO_AF4_I2C1
#elif (defined USE_STM32F0XX_NUCLEO)
	#define I2Cx_SCL_AF 											      	GPIO_AF1_I2C1
#elif (defined USE_STM32F1XX_NUCLEO)      
  /* Not supported */    
#endif     
/* I2C SPEED
 * F4 uses directly the speed (100,400) and F0, L0, L3 use the TIMMINGR register defined below */
#if (defined USE_STM32F4XX_NUCLEO) || (defined USE_STM32F1XX_NUCLEO) || \
    (defined USE_STM32L1XX_NUCLEO)
	#define M24SR_I2C_SPEED_10													10000
	#define M24SR_I2C_SPEED_100													100000
	#define M24SR_I2C_SPEED_400													400000
	#define M24SR_I2C_SPEED_1000												1000000
/* Timing samples with PLLCLK 48MHz set in SystemClock_Config(), I2C CLK on SYSCLK value computed with CubeMx */ 	
#elif (defined USE_STM32F0XX_NUCLEO)
	#define M24SR_I2C_SPEED_10													0x9010DEFF
	#define M24SR_I2C_SPEED_100													0x20303E5D
	#define M24SR_I2C_SPEED_400													0x2010091A
	#define M24SR_I2C_SPEED_1000												0x00200818	
/* Timing samples with PLLCLK 32MHz set in SystemClock_Config(), I2C CLK on SYSCLK value computed with CubeMx */    
#elif (defined USE_STM32L0XX_NUCLEO) || (defined USE_B_L072Z_LRWAN1)
	#define M24SR_I2C_SPEED_10													0x6010C7FF
	#define M24SR_I2C_SPEED_100													0x00707CBB
	#define M24SR_I2C_SPEED_400													0x00300F38
	#define M24SR_I2C_SPEED_1000												0x00100413
/* Timing samples with PLLCLK 64MHz set in SystemClock_Config(), I2C CLK on SYSCLK value computed with CubeMx */ 		
#elif (defined USE_STM32F3XX_NUCLEO)
	#define M24SR_I2C_SPEED_10													0xE010A9FF
	#define M24SR_I2C_SPEED_100													0x10707DBC
	#define M24SR_I2C_SPEED_400													0x00602173
	#define M24SR_I2C_SPEED_1000												0x00300B29
#elif (defined USE_STM32L4XX_NUCLEO)
	#define M24SR_I2C_SPEED_10													0xF000F3FE /* Clock 80MHz, Fast Mode, Analog Filter ON, Rise time 25ns, Fall time 10ns */
	#define M24SR_I2C_SPEED_100													0x203012F1 /* Clock 80Mhz, Fast Mode, Analog Filter ON, Rise time 50ns, Fall time 10ns */
	#define M24SR_I2C_SPEED_400													0x00B0298B /* Clock 80Mhz, Fast Mode, Analog Filter ON, Rise time 50ns, Fall time 25ns */
	#define M24SR_I2C_SPEED_1000												0x00700E2E /* Clock 80Mhz, Fast Mode Plus, Analog Filter ON, Rise time 50ns, Fall time 25ns */
#else
	#error "You need to update your code to this new microcontroller"
#endif


#define M24SR_I2C_SPEED				M24SR_I2C_SPEED_400

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
