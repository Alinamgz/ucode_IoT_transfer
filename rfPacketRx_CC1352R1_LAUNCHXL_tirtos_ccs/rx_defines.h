#pragma once

/***** Defines *****/

#define MSG_PREFIX "received: "
#define WELCOME_MSG "\n\r--- Hello World @ Rx ---\n\r"
#define NEWLINE "\n\r"

// ================ KEY stuff =====================
#define AES_KEY_LEN 16
#define PRIVATE_KEY_LEN 32
#define PUBLIC_KEY_LEN 65

#define MSG_PKG 0
#define KEY_PKG 1

#define PKG_ID_BYTE 0
#define PKG_LEN_BYTE 1

#define HEADER_LEN 1
#define NONCE_LEN 12
#define MSG_LEN 64
#define MAC_LEN 4

#define MSG_PKG_LEN (HEADER_LEN + NONCE_LEN + MSG_LEN + MAC_LEN + PRIVATE_KEY_LEN)
#define KEY_PKG_LEN (HEADER_LEN + PUBLIC_KEY_LEN + PRIVATE_KEY_LEN + PRIVATE_KEY_LEN)

/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 10  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             KEY_PKG_LEN /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     2  /* The Data Entries data field will contain:
                                   * 1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                   * Max 30 payload bytes
                                   * 1 status byte (RF_cmdPropRx.rxConf.bAppendStatus = 0x1) */


