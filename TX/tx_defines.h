#pragma once

//#define POWER_MEASUREMENT

/* Packet TX Configuration */

//#define PAYLOAD_LENGTH      60
#ifdef POWER_MEASUREMENT
#define PACKET_INTERVAL     5  /* For power measurement set packet interval to 5s */
#else
#define PACKET_INTERVAL     500000  /* Set packet interval to 500000us or 500ms */
#endif

#define HELLO_MSG "\n\r=================== Hello World @ Tx ==================\n\r"
#define KEY_WAITING_MSG "      Waiting for peer public key...\n\rNOTE: restart Rx and press its LEFT button to send key\n\r"
#define PRESS_BTN_MSG "NOTE: Press RIGHT button to share my public key...\n\r"
#define WELCOME_MSG "      Key exchange done.\n\r======================= Welcome =======================\n\r"
#define TIP_EDIT_MSG " TIP: To edit your msg use ARROW LEFT instead of Bacspace\n\r"
#define TIP_LOREM_MSG " TIP: To send a placeholder msg type: lorem + ENTER\n\r"
#define FIN_LINE "======================= ======== =======================\n\n\r"

#define NEWLINE "\n\r--------------------------\n\r"

#define TX_ID "L41009CU"
#define LOREM_IPSUM "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

// key n' crypto
#define AES_KEY_LEN 16
#define PRIVATE_KEY_LEN 32
#define PUBLIC_KEY_LEN 65

#define MSG_PKG 0
#define KEY_PKG 1

#define PKG_ID_BYTE 0
#define CUR_PKG_NUM_BYTE 1
#define TOTAL_PKG_NUM_BYTE 2


#define HEADER_LEN 1
#define MSG_HEADER_LEN 3
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
