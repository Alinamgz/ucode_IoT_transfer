 #pragma once


// Keys
uint8_t packet[MAX_LENGTH + NUM_APPENDED_BYTES - 1]; /* The length byte is stored in a separate variable */

uint8_t private_key_material[PRIVATE_KEY_LEN];
uint8_t public_key_material[PUBLIC_KEY_LEN];

uint8_t peer_pub_key_material[PUBLIC_KEY_LEN];

uint8_t shared_secret_material[PUBLIC_KEY_LEN];
uint8_t symmetric_key_material[AES_KEY_LEN];
//==================================================
CryptoKey private_key;
CryptoKey public_key;

CryptoKey peer_priv_key;
CryptoKey peer_pub_key;

CryptoKey shared_secret;
CryptoKey symmetric_key;

/* Semaphores to pend on button presses */
static Semaphore_Handle send_btn_pressed;

/***** Variable declarations *****/
//RADIO
RF_Handle rfHandle;
RF_Object rfObject;
RF_EventMask terminationReason;
uint32_t cmdStatus;

/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

//============================================================================

UART2_Handle uart;
static UART2_Params uart_params;


/* Buffer which contains all Data Entries for receiving data.
 * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN (rxDataEntryBuffer, 4);
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)];
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 4
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)];
#elif defined(__GNUC__)
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)]
                                                  __attribute__((aligned(4)));
#else
#error This compiler is not supported.
#endif





