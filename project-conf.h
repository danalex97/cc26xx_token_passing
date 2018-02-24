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
//Define the network stack
#define NETSTACK_CONF_NETWORK rime_driver // Define the network driver to use
#define NETSTACK_CONF_MAC     csma_driver // Define the MAC driver to use
#define NETSTACK_CONF_RDC     nullrdc_driver // Define the RDC driver to use.
#define NETSTACK_CONF_FRAMER  framer_802154 // Define the framer driver to use
#define NETSTACK_CONF_RADIO   cc2650_driver // Define the radio driver to use.

#undef TIMESYNCH_CONF_ENABLED  // TO ENABLE THE Implicit Network Time Synchronization
#define TIMESYNCH_CONF_ENABLED 1 // TO ENABLE THE Implicit Network Time Synchronization

//Define the channel to be used
#define RF_CHANNEL 20
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
