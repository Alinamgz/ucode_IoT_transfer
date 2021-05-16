#pragma once

//#define POWER_MEASUREMENT

/* Packet TX Configuration */

//#define PAYLOAD_LENGTH      60
#ifdef POWER_MEASUREMENT
#define PACKET_INTERVAL     5  /* For power measurement set packet interval to 5s */
#else
#define PACKET_INTERVAL     500000  /* Set packet interval to 500000us or 500ms */
#endif

#define TX_ID "L41009CU"
#define BUF_SZ 240


// key n' crypto
#define AES_KEY_LEN 16
#define PRIVATE_KEY_LEN 32
#define PUBLIC_KEY_LEN (1 + 2 * PRIVATE_KEY_LEN)

#define MSG_PKG 0
#define KEY_PKG 1

#define PKG_ID_BYTE 0
#define PKG_LEN_BYTE 1

#define HEADER_LEN 2
#define NONCE_LEN 12
#define MSG_LEN 64
#define MAC_LEN 4

#define MSG_PKG_LEN (HEADER_LEN + NONCE_LEN + MSG_LEN + MAC_LEN + PRIVATE_KEY_LEN + PRIVATE_KEY_LEN)
#define KEY_PKG_LEN (HEADER_LEN + PUBLICK_KEY_LEN + PRIVATE_KEY_LEN + PRIVATE_KEY_LEN + PRIVATEKEY_LEN)

//void mx_generate_random_bytes(CryptoKey *entropyKey);
