/**
 * \file
 *         Course work 2 configuration file.
 * \author
 *         AESE research group
 */

/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/
/* Change to match your configuration */
#define IEEE802154_CONF_PANID            0xABCD
#define RF_CORE_CONF_CHANNEL                 18
#define RF_BLE_CONF_ENABLED                   0


/* Coursework parameters */
#define PERIODIC_INTERVAL_MILISEC           200
#define PRIORITY_INTERVAL_SEC                30
#define ENABLE_PRIORITY_PACKET                1
#define DEBUG_ENABLED                         1
#define PRIORITY_REQUEST                 0x0000
#define PRIORITY_RESPONSE                0xFFFF
#define SENDER_NUM                            9
#define PACKAGE_SIZE                          2
#define PRIORITY_REQUEST_SIZE                 4
/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/