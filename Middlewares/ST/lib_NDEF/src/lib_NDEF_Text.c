/**
  ******************************************************************************
  * @file    lib_NDEF_Text.c
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    19-March-2014
  * @brief   This file help to manage NDEF file that represent Text.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lib_NDEF_Text.h"


/** @addtogroup NFC_libraries
 * 	@{
 */


/** @addtogroup lib_NDEF
  * @{
  */

/**
 * @brief  This buffer contains the data send/received by TAG
 */
extern uint8_t NDEF_Buffer [NDEF_MAX_SIZE];

/** @defgroup libText_Private_Functions
  * @{
  */



/**
  * @}
  */

/** @defgroup libText_Public_Functions
  * @{
  *	@brief  This file is used to manage Text (stored or loaded in tag)
  */ 

/**
  * @brief  This fonction read NDEF and retrieve Text information if any
	* @param	pRecordStruct : Pointer on the record structure
	* @param	pTextStruct : pointer on the structure to fill
  * @retval SUCCESS : Text information from NDEF have been retrieve
	* @retval ERROR : Not able to retrieve Text information
  */
uint16_t NDEF_ReadText(sRecordInfo *pRecordStruct, sTextInfo *pTextStruct)
{
	uint16_t status = ERROR;
	uint16_t FileId=0;
	sRecordInfo *pSPRecordStruct;	
	uint32_t PayloadSize, RecordPosition;
	uint8_t* pData;
  
	if( pRecordStruct->NDEF_Type == TEXT_TYPE )
	{	
		PayloadSize = ((uint32_t)(pRecordStruct->PayloadLength3)<<24) | ((uint32_t)(pRecordStruct->PayloadLength2)<<16) |
	    ((uint32_t)(pRecordStruct->PayloadLength1)<<8)  | pRecordStruct->PayloadLength0;

		/* Read record header */
		pData = (uint8_t*)(pRecordStruct->PayloadBufferAdd);
		memcpy(pTextStruct->Message, (pData + 3), PayloadSize - 3);
		status = SUCCESS;
	}
	

	CloseNDEFSession(FileId);
	
	return status;
}



/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2013 STMicroelectronics *****END OF FILE****/


