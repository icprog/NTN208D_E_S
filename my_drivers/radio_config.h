/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef RADIO_CONFIG_H
#define RADIO_CONFIG_H
#include "nrf.h"
#include "App_Var.h"

#define PACKET_BASE_ADDRESS_LENGTH       (2UL)  //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH             (0UL)  //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE           100  //!< Packet payload maximum size in bytes
typedef struct
{
	uint8_t length;//payload长度
	uint8_t packet[PACKET_PAYLOAD_MAXSIZE];//射频
	uint8_t flag;
}Payload_Typedef;


#define 	RADIO_OVER_TIME							100000
//2.4G状态
typedef enum
{
    RADIO_STATUS_IDLE = 1,
    RADIO_STATUS_RX,
    RADIO_STATUS_TX,
}RADIO_Status;



void radio_configure(void);
void Radio_Init(void);
void radio_modulated_tx_carrier(uint8_t txpower, uint8_t mode, uint8_t channel);
void radio_rx_carrier(uint8_t mode, uint8_t channel);
void radio_modulated_rx_carrier(uint8_t mode, uint8_t channel);
void startTX(void);
void startRX(void);
//void radio_tx_carrier(uint8_t txpower, uint8_t mode, uint8_t channel);
void radio_tx_carrier( uint8_t mode, uint8_t channel);
void radio_disable(void);
uint8_t radio_tx_isbusy(void);
#endif
