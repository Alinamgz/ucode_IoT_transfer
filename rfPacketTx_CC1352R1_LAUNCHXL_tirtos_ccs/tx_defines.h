#pragma once

//#define POWER_MEASUREMENT

/* Packet TX Configuration */
#define PAYLOAD_LENGTH      30
#ifdef POWER_MEASUREMENT
#define PACKET_INTERVAL     5  /* For power measurement set packet interval to 5s */
#else
#define PACKET_INTERVAL     500000  /* Set packet interval to 500000us or 500ms */
#endif

#define TX_ID "L41009CU"
#define TEST_MSG "-234567890123456789-234567890123456789-234567890123456789"
