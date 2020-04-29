																			   /**
  ******************************************************************************
  * @file    loopback.h
  * @author  Forrest
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   This file contains all the functions prototypes for the telneterver.c 
  *          file.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOOP_BACK_H__
#define __LOOP_BACK_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @
  * @ start server
  */

void tcpserver_init(void);
void loopclient_init(void);

#ifdef __cplusplus
}
#endif

#endif


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

