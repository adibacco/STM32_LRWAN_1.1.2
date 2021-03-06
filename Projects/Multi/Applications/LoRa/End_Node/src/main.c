 /*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic lora driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Wael Guibene
*/
/******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    08-September-2017
  * @brief   this is the main!
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power.h"
#include "lora.h"
#include "bsp.h"
#include "timeServer.h"
#include "vcom.h"
#include "version.h"
#include "main.h"
#include "drv_I2C_M24SR.h"
#include <time.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern uint8_t DevEui[];
extern uint8_t AppKey[];
/*!
 * CAYENNE_LPP is myDevices Application server.
 */
//#define CAYENNE_LPP
#define LPP_DATATYPE_DIGITAL_INPUT  0x0
#define LPP_DATATYPE_DIGITAL_OUTPUT 0x1
#define LPP_DATATYPE_HUMIDITY       0x68
#define LPP_DATATYPE_TEMPERATURE    0x67
#define LPP_DATATYPE_BAROMETER      0x73

#define LPP_APP_PORT 99

/*!
 * Defines the application data transmission duty cycle. 5s, value in [ms].
 */
#define APP_TX_DUTYCYCLE                            60000
/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                              1
/*!
 * LoRaWAN confirmed messages
 */
#define LORAWAN_CONFIRMED_MSG                    DISABLE
/*!
 * LoRaWAN application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_APP_PORT                            2
/*!
 * Number of trials for the join request.
 */
#define JOINREQ_NBTRIALS                            3

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* call back when LoRa will transmit a frame*/
static void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed);

/* call back when LoRa has received a frame*/
static void LoraRxData( lora_AppData_t *AppData);

/* Private variables ---------------------------------------------------------*/
sTextInfo textInfo;
char* tokens[16];

/* load Main call backs structure*/
static LoRaMainCallback_t LoRaMainCallbacks ={ HW_GetBatteryLevel,
                                               HW_GetUniqueId,
                                               HW_GetRandomSeed,
                                               LoraTxData,
                                               LoraRxData};

/*!
 * Specifies the state of the application LED
 */
static uint8_t AppLedStateOn = RESET;


#ifdef USE_B_L072Z_LRWAN1
/*!
 * Timer to handle the application Tx Led to toggle
 */
static TimerEvent_t TxLedTimer;
static void OnTimerLedEvent( void );
#endif
/* !
 *Initialises the Lora Parameters
 */
static  LoRaParam_t LoRaParamInit= {TX_ON_TIMER,
                                    APP_TX_DUTYCYCLE,
                                    CLASS_C,
                                    LORAWAN_ADR_ON,
                                    DR_0,
                                    LORAWAN_PUBLIC_NETWORK,
                                    JOINREQ_NBTRIALS};

/* Private functions ---------------------------------------------------------*/
void parseNfcInfo(char* str, char** tokens) {

	   const char s[2] = ";";
	   int i = 0;
	   char* token;

	   /* get the first token */
	   token = tokens[i++] = strtok(str, s);

	   /* walk through other tokens */
	   while( token != NULL ) {
		  token = tokens[i++] = strtok(NULL, s);
	   }

}

void getDevEui(const char* str, uint8_t* data) {

#define EUI_LEN		8

	int values[EUI_LEN];
	int i;


	if( EUI_LEN == sscanf( str, "%x:%x:%x:%x:%x:%x:%x:%x%*c",
		&values[0], &values[1], &values[2], &values[3],
		&values[4], &values[5], &values[6], &values[7]
		) )
	{
		/* convert to uint8_t */
		for( i = 0; i < EUI_LEN; ++i )
			data[i] = (uint8_t) values[i];
	}
	else
	{
		PRINTF("EUI is invalid\n\r");
	}
}


void getAppKey(const char* str, uint8_t* data) {

#define APPKEY_LEN		16

	int values[APPKEY_LEN];
	int i;


	if( APPKEY_LEN == sscanf( str, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x%*c",
		&values[0], &values[1], &values[2], &values[3],
		&values[4], &values[5], &values[6], &values[7],
		&values[8], &values[9], &values[10], &values[11],
		&values[12], &values[13], &values[14], &values[15]
		) )
	{
		/* convert to uint8_t */
		for( i = 0; i < APPKEY_LEN; ++i )
			data[i] = (uint8_t) values[i];
	}
	else
	{
		PRINTF("APPKEY is invalid\n\r");
	}
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main( void )
{
  /* STM32 HAL library initialization*/
  HAL_Init( );
  
  /* Configure the system clock*/
  SystemClock_Config( );
  
  /* Configure the debug mode*/
  DBG_Init( );
  
  /* Configure the hardware*/
  HW_Init( );
  
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */
  
  LED_On(LED_GREEN);
  HAL_Delay(50);
  LED_Off(LED_GREEN);

#if defined (X_NUCLEO_NFC01A1)
  // Init of the Type Tag 4 component (M24SR)
  // Thanks to a call to KillSession command during init no issue can occurs
  // If customer modify the code to avoid Kill session command call,
  // he must retry Init until success (session can be lock by RF )
  while (TT4_Init() != SUCCESS);

  TT4_ReadText(&textInfo);

  parseNfcInfo(&textInfo.Message, tokens);

  getDevEui(tokens[1], DevEui);
  getAppKey(tokens[2], AppKey);

  PRINTF("NFC: %s\n\r", textInfo.Message);

#endif

  /* Configure the Lora Stack*/
  lora_Init( &LoRaMainCallbacks, &LoRaParamInit);
  
  PRINTF("VERSION: %X\n\r", VERSION);

  /* main loop*/
  while( 1 )
  {
    /* run the LoRa class A state machine*/
    lora_fsm( );
    
    DISABLE_IRQ( );
    /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending 
     * and cortex will not enter low power anyway  */
    if ( lora_getDeviceState( ) == DEVICE_STATE_SLEEP )
    {
#ifndef LOW_POWER_DISABLE
      LowPower_Handler( );
#endif
    }
    ENABLE_IRQ();
    
    /* USER CODE BEGIN 2 */
    /* USER CODE END 2 */
  }
}

static void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed)
{
  /* USER CODE BEGIN 3 */
  uint16_t pressure = 0;
  int16_t temperature = 0;
  uint16_t humidity = 0;
  uint8_t batteryLevel;
  sensor_t sensor_data;
  
	#ifdef USE_B_L072Z_LRWAN1
  TimerInit( &TxLedTimer, OnTimerLedEvent );
  
  TimerSetValue(  &TxLedTimer, 200);
  
  LED_On( LED_RED1 ) ; 
  
  TimerStart( &TxLedTimer );  
#endif
#ifndef CAYENNE_LPP
  int32_t latitude, longitude = 0;
  uint16_t altitudeGps = 0;
#endif
  BSP_sensor_Read( &sensor_data );

#ifdef CAYENNE_LPP
  uint8_t cchannel=0;
  temperature = ( int16_t )( sensor_data.temperature * 10 );     /* in �C * 10 */
  pressure    = ( uint16_t )( sensor_data.pressure * 100 / 10 );  /* in hPa / 10 */
  humidity    = ( uint16_t )( sensor_data.humidity * 2 );        /* in %*2     */
  uint32_t i = 0;

  batteryLevel = HW_GetBatteryLevel( );                     /* 1 (very low) to 254 (fully charged) */

  AppData->Port = LPP_APP_PORT;
  
  *IsTxConfirmed =  LORAWAN_CONFIRMED_MSG;
  AppData->Buff[i++] = cchannel++;
  AppData->Buff[i++] = LPP_DATATYPE_BAROMETER;
  AppData->Buff[i++] = ( pressure >> 8 ) & 0xFF;
  AppData->Buff[i++] = pressure & 0xFF;
  AppData->Buff[i++] = cchannel++;
  AppData->Buff[i++] = LPP_DATATYPE_TEMPERATURE; 
  AppData->Buff[i++] = ( temperature >> 8 ) & 0xFF;
  AppData->Buff[i++] = temperature & 0xFF;
  AppData->Buff[i++] = cchannel++;
  AppData->Buff[i++] = LPP_DATATYPE_HUMIDITY;
  AppData->Buff[i++] = humidity & 0xFF;
#if defined( REGION_US915 ) || defined( REGION_US915_HYBRID ) || defined ( REGION_AU915 )
  /* The maximum payload size does not allow to send more data for lowest DRs */
#else
  AppData->Buff[i++] = cchannel++;
  AppData->Buff[i++] = LPP_DATATYPE_DIGITAL_INPUT; 
  AppData->Buff[i++] = batteryLevel*100/254;
  AppData->Buff[i++] = cchannel++;
  AppData->Buff[i++] = LPP_DATATYPE_DIGITAL_OUTPUT; 
  AppData->Buff[i++] = AppLedStateOn;
#endif  /* REGION_XX915 */
#else  /* not CAYENNE_LPP */

  temperature = ( int16_t )( sensor_data.temperature * 100 );     /* in �C * 100 */
  pressure    = ( uint16_t )( sensor_data.pressure * 100 / 10 );  /* in hPa / 10 */
  humidity    = ( uint16_t )( sensor_data.humidity * 10 );        /* in %*10     */
  latitude = sensor_data.latitude;
  longitude= sensor_data.longitude;
  uint32_t i = 0;

  batteryLevel = HW_GetBatteryLevel( );                     /* 1 (very low) to 254 (fully charged) */

  AppData->Port = LORAWAN_APP_PORT;
  
  *IsTxConfirmed =  LORAWAN_CONFIRMED_MSG;

#if defined( REGION_US915 ) || defined( REGION_US915_HYBRID ) || defined ( REGION_AU915 )
  AppData->Buff[i++] = AppLedStateOn;
  AppData->Buff[i++] = ( pressure >> 8 ) & 0xFF;
  AppData->Buff[i++] = pressure & 0xFF;
  AppData->Buff[i++] = ( temperature >> 8 ) & 0xFF;
  AppData->Buff[i++] = temperature & 0xFF;
  AppData->Buff[i++] = ( humidity >> 8 ) & 0xFF;
  AppData->Buff[i++] = humidity & 0xFF;
  AppData->Buff[i++] = batteryLevel;
  AppData->Buff[i++] = 0;
  AppData->Buff[i++] = 0;
  AppData->Buff[i++] = 0;
#else  /* not REGION_XX915 */
  AppData->Buff[i++] = AppLedStateOn;
  AppData->Buff[i++] = ( pressure >> 8 ) & 0xFF;
  AppData->Buff[i++] = pressure & 0xFF;
  AppData->Buff[i++] = ( temperature >> 8 ) & 0xFF;
  AppData->Buff[i++] = temperature & 0xFF;
  AppData->Buff[i++] = ( humidity >> 8 ) & 0xFF;
  AppData->Buff[i++] = humidity & 0xFF;
  AppData->Buff[i++] = batteryLevel;
  AppData->Buff[i++] = ( latitude >> 16 ) & 0xFF;
  AppData->Buff[i++] = ( latitude >> 8 ) & 0xFF;
  AppData->Buff[i++] = latitude & 0xFF;
  AppData->Buff[i++] = ( longitude >> 16 ) & 0xFF;
  AppData->Buff[i++] = ( longitude >> 8 ) & 0xFF;
  AppData->Buff[i++] = longitude & 0xFF;
  AppData->Buff[i++] = ( altitudeGps >> 8 ) & 0xFF;
  AppData->Buff[i++] = altitudeGps & 0xFF;
#endif  /* REGION_XX915 */
#endif  /* CAYENNE_LPP */
	// sending raw data in TEXT format (not good for security)
#if defined (X_NUCLEO_NFC01A1)
  AppData->BuffSize = sprintf((char *) AppData->Buff, "%s,%s,%s,%d", tokens[0], tokens[3], tokens[4], batteryLevel);
#else
  AppData->BuffSize = sprintf((char *) AppData->Buff, "%f,%f,%f,%d", sensor_data.temperature, sensor_data.pressure, sensor_data.humidity, batteryLevel);
#endif
  /* USER CODE END 3 */
}

static void LoraRxData( lora_AppData_t *AppData )
{
  /* USER CODE BEGIN 4 */
  switch (AppData->Port)
  {
  case LORAWAN_APP_PORT:
    if( AppData->BuffSize == 1 )
    {
      AppLedStateOn = AppData->Buff[0] & 0x01;
      if ( AppLedStateOn == RESET )
      {
        PRINTF("LED OFF\n\r");
        LED_Off( LED_BLUE ) ; 
        LED_Off( LED_GREEN ) ;
        
      }
      else
      {
        PRINTF("LED ON\n\r");
        LED_On( LED_BLUE ) ; 
        LED_On( LED_GREEN ) ;

      } 
      //GpioWrite( &Led3, ( ( AppLedStateOn & 0x01 ) != 0 ) ? 0 : 1 );
    }
    break;
  case LPP_APP_PORT:
  {
    AppLedStateOn= (AppData->Buff[2] == 100) ?  0x01 : 0x00;
      if ( AppLedStateOn == RESET )
      {
        PRINTF("LED OFF\n\r");
        //LED_Off( LED_BLUE ) ; 
        
      }
      else
      {
        PRINTF("LED ON\n\r");
        //LED_On( LED_BLUE ) ; 
      }
    break;
  }
  default:
    break;
  }
  /* USER CODE END 4 */
}

#ifdef USE_B_L072Z_LRWAN1
static void OnTimerLedEvent( void )
{
  LED_Off( LED_RED1 ) ; 
}
#endif

void M24SR_I2CInit ( void )
{
	/* check if we re-init after a detected issue */
	if( hi2c1.Instance == M24SR_I2C)
	  HAL_I2C_DeInit(&hi2c1);

  /* Configure I2C structure */
  hi2c1.Instance 	     = M24SR_I2C;
  hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
#if defined (STM32F302x8)
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
#elif defined (STM32F401xE)
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
#endif
  hi2c1.Init.OwnAddress1     = 0;
  hi2c1.Init.OwnAddress2     = 0;
#if (defined USE_STM32F4XX_NUCLEO) || (defined USE_STM32L1XX_NUCLEO) || \
	  (defined USE_STM32F1XX_NUCLEO)
  hi2c1.Init.ClockSpeed      = M24SR_I2C_SPEED;
  hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
#elif (defined USE_STM32F0XX_NUCLEO) || (defined USE_STM32L0XX_NUCLEO) || (defined USE_B_L072Z_LRWAN1) || \
      (defined USE_STM32F3XX_NUCLEO) || (defined USE_STM32L4XX_NUCLEO)
  hi2c1.Init.Timing          = M24SR_I2C_SPEED;
#endif

	if(HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

}

void M24SR_GPOInit ( void )
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  INIT_CLK_GPO_RFD();

  /* Configure GPIO pins for GPO (PA6)*/
#ifndef I2C_GPO_INTERRUPT_ALLOWED
  GPIO_InitStruct.Pin = M24SR_GPO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(M24SR_GPO_PIN_PORT, &GPIO_InitStruct);
#else
  GPIO_InitStruct.Pin = M24SR_GPO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

  HAL_GPIO_Init(M24SR_GPO_PIN_PORT, &GPIO_InitStruct);
  /* Enable and set EXTI9_5 Interrupt to the lowest priority */
#if (defined USE_STM32F4XX_NUCLEO) || (defined USE_STM32F3XX_NUCLEO) || \
     (defined USE_STM32L1XX_NUCLEO) || (defined USE_STM32F1XX_NUCLEO) || (defined USE_STM32L4XX_NUCLEO)
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
#elif (defined USE_STM32L0XX_NUCLEO) || (defined USE_B_L072Z_LRWAN1) || (defined USE_STM32F0XX_NUCLEO)
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
#endif

#endif

  /* Configure GPIO pins for DISABLE (PA7)*/
  GPIO_InitStruct.Pin = M24SR_RFDIS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(M24SR_RFDIS_PIN_PORT, &GPIO_InitStruct);
}

/**
  * @brief  This function wait the time given in param (in milisecond)
	* @param	time_ms: time value in milisecond
  */
void M24SR_WaitMs(uint32_t time_ms)
{
	wait_ms(time_ms);
}

/**
  * @brief  This function retrieve current tick
  * @param	ptickstart: pointer on a variable to store current tick value
  */
void M24SR_GetTick( uint32_t *ptickstart )
{
	*ptickstart = HAL_GetTick();
}
/**
  * @brief  This function read the state of the M24SR GPO
	* @param	none
  * @retval GPIO_PinState : state of the M24SR GPO
  */
void M24SR_GPO_ReadPin( GPIO_PinState * pPinState)
{
	*pPinState = HAL_GPIO_ReadPin(M24SR_GPO_PIN_PORT,M24SR_GPO_PIN);
}

/**
  * @brief  This function set the state of the M24SR RF disable pin
	* @param	PinState: put RF disable pin of M24SR in PinState (1 or 0)
  */
void M24SR_RFDIS_WritePin( GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(M24SR_RFDIS_PIN_PORT,M24SR_RFDIS_PIN,PinState);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
