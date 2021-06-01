/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/
#include "tx_defines.h"

/***** Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

/***** Prototypes *****/
static void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);
static void mx_handle_keypkg(uint8_t *packet, CryptoKey *peer_pub_key);
static void mx_generate_aes_key(CryptoKey *my_private_key,
								CryptoKey *peer_pub_key,
								CryptoKey *shared_secret,
								CryptoKey *symetric_key);

void mx_recv_n_proceed_peer_key(void) {
	uint32_t cmdStatus;
	terminationReason = RF_runCmd(rfHandle,
								  (RF_Op*)&RF_cmdPropRx,
								  RF_PriorityNormal,
								  &callback,
								  RF_EventRxEntryDone);
	cmdStatus = ((volatile RF_Op*)&RF_cmdPropRx)->status;

	if (terminationReason == RF_EventLastCmdDone
		&& cmdStatus == PROP_DONE_OK
		&& packet[0] == KEY_PKG) {
		mx_handle_keypkg(packet, &peer_pub_key);
		mx_generate_aes_key(&private_key,
							&peer_pub_key,
							&shared_secret,
							&symmetric_key);
		//  ---------- ?????? ------------------
		mx_share_my_pubkey()
		mx_do_msg();
	}
	else {
		mx_say_err("peer key recv n proceed");
	}
}

// =============================================================================
// =============================================================================

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e) {
    if (e & RF_EventRxEntryDone) {
            /* Toggle pin to indicate RX */
            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
            /* Get current unhandled data entry */
            currentDataEntry = RFQueue_getDataEntry();

            /* Handle the packet data, located at &currentDataEntry->data:
             * - Length is the first byte with the current configuration
             * - Data starts from the second byte */
            packetLength      = *(uint8_t*)(&currentDataEntry->data);
            packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

            /* Copy the payload + the status byte to the packet variable */
            memset(packet,0, sizeof(packet));
            memcpy(packet, packetDataPointer, (packetLength + 1));

            switch(*packetDataPointer) {
                case KEY_PKG:
                    UART2_write(uart, "\r\nrecv key:\n\r", sizeof("\r\nrecv key:\n\r"), NULL);
                    GPIO_toggle(CONFIG_GPIO_LED_RED);
                    break;
                case MSG_PKG:
                    UART2_write(uart, "\r\nrecv msg:\n\r", sizeof("\r\nrecv msg:\n\r"), NULL);
                    break;
                default:
                    UART2_write(uart, "!!! ERR: unknown pkg type !!!\\n\r", sizeof("!!! ERR: unknown pkg type !!!\\n\r"), NULL);
                    break;
            }
        UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
        RFQueue_nextEntry();
    }
}

//==============================================================================
//==============================================================================

void mx_handle_keypkg(uint8_t *packet, CryptoKey *peer_pub_key) {
    ECDSA_Handle handle_verify;
    ECDSA_OperationVerify operation_verify;

    int_fast16_t rslt;
    uint8_t hash_buf[PRIVATE_KEY_LEN];
//
    int i = 0;
    char status[4];
    uint8_t chck[] = LOREM_IPSUM;

    UART2_write(uart, "Sending key pkg done. RECV:\n\r", sizeof("Sending key pkg done RECV:\n\r"), NULL);
    UART2_write(uart, packet, MAX_LENGTH, NULL);
    UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
    for (i = 0; i < MAX_LENGTH; i++) {
        memset(status, 0, sizeof(status));
        sprintf(status, " %d", packet[i]);
        UART2_write(uart, status, sizeof(status), NULL);
    }

    UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
    /* Copy the public keys from the packet into the parameters */
    memcpy(peer_pub_key_material, &packet[HEADER_LEN], PUBLIC_KEY_LEN);
    CryptoKeyPlaintext_initKey(peer_pub_key, peer_pub_key_material, PUBLIC_KEY_LEN);

    UART2_write(uart, "\n\rPEER PUB KEY\n\r", sizeof("\n\rPEER PUB KEY\n\r"), NULL);
    for (i = 0; i < peer_pub_key->u.plaintext.keyLength; i++) {
        memset(status, 0, sizeof(status));
        sprintf(status, " %d", peer_pub_key->u.plaintext.keyMaterial[i]);
        UART2_write(uart, status, sizeof(status), NULL);
    }
    UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);

//================================== Check hash ========================================================
    do_sha(chck, HEADER_LEN + PUBLIC_KEY_LEN, hash_buf, 1);

    UART2_write(uart, "CHECK HASH\n", sizeof("CHECK HASH\n"), NULL);
    for (i = 0; i < SHA2_DIGEST_LENGTH_BYTES_256; i++) {
        memset(status, 0, sizeof(status));
        sprintf(status, " %d", hash_buf[i]);
        UART2_write(uart, status, sizeof(status), NULL);
    }
    UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);
//===================================            =======================================================


    /* Hash the header and public key component of the message. Pass NULL to use the default parameters */
    memset(hash_buf, 0, sizeof(hash_buf));
    do_sha(packet, HEADER_LEN + PUBLIC_KEY_LEN, hash_buf, 1);

    UART2_write(uart, "PEER PUB KEY HASH\n", sizeof("PEER PUB KEY HASH\n"), NULL);
    for (i = 0; i < SHA2_DIGEST_LENGTH_BYTES_256; i++) {
        memset(status, 0, sizeof(status));
        sprintf(status, " %d", hash_buf[i]);
        UART2_write(uart, status, sizeof(status), NULL);
    }
    UART2_write(uart, NEWLINE, sizeof(NEWLINE), NULL);

UART2_write(uart, "\n\rsha done\n\r", sizeof("\n\rsha done\n\r"), NULL);

    /* Verify signature of public key */
    ECDSA_OperationVerify_init(&operation_verify);
    operation_verify.curve = &ECCParams_NISTP256;
    operation_verify.theirPublicKey = peer_pub_key;
    operation_verify.hash = hash_buf;
    operation_verify.r = &packet[HEADER_LEN + PUBLIC_KEY_LEN];
    operation_verify.s = &packet[HEADER_LEN + PUBLIC_KEY_LEN + PRIVATE_KEY_LEN];

UART2_write(uart, "ECDSA_OperationVerify_init done\n\r", sizeof("ECDSA_OperationVerify_init done\n\r"), NULL);
    /* Pass NULL to use the default parameters */
    handle_verify = ECDSA_open(CONFIG_ECDSA_0, NULL);
    if (!handle_verify) {
        mx_say_err("ECDSA_open @verification");
    }


    rslt = ECDSA_verify(handle_verify, &operation_verify);
    if (rslt != ECDSA_STATUS_SUCCESS) {
        char chck[32];
        memset(chck, 0, sizeof(chck));
        sprintf(chck, "ECDSA_verify code %d", rslt);

        mx_say_err(chck);
    }

    ECDSA_close(handle_verify);
    UART2_write(uart, "pub key pkg verification kinda done\n\r", sizeof("pub key pkg verification kinda done\n\r"), NULL);
}

//==============================================================================
//==============================================================================

void mx_generate_aes_key(CryptoKey *my_private_key, CryptoKey *peer_pub_key, CryptoKey *shared_secret, CryptoKey *symetric_key) {
    int_fast16_t rslt;

    ECDH_Handle handle_ecdh;
    ECDH_Params ecdh_params;
    ECDH_OperationComputeSharedSecret operationComputeSharedSecret;

    uint8_t buf_sha_digest[SHA2_DIGEST_LENGTH_BYTES_256];

    int i = 0;
    char status[64];
    memset(status, 0, 64);

    /* Since we are using default ECDH_Params, we just pass in NULL for that parameter. */
    ECDH_Params_init(&ecdh_params);
    ecdh_params.returnBehavior = ECDH_RETURN_BEHAVIOR_BLOCKING;
    handle_ecdh = ECDH_open(CONFIG_ECDH_0, &ecdh_params);
    if (!handle_ecdh) {
        mx_say_err("ECDH_open");
    }

    /* The ECC_NISTP256 struct is provided in ti/drivers/types/EccParams.h and the corresponding device-specific implementation. */
    ECDH_OperationComputeSharedSecret_init(&operationComputeSharedSecret);
    operationComputeSharedSecret.curve = &ECCParams_NISTP256;
    operationComputeSharedSecret.myPrivateKey       = my_private_key;
    operationComputeSharedSecret.theirPublicKey     = peer_pub_key;
    operationComputeSharedSecret.sharedSecret       = shared_secret;

    UART2_write(uart, "secret BEF:\n", sizeof("secret BEF:\n"), NULL);
    for (i = 0; i < shared_secret->u.plaintext.keyLength; i++) {
        memset(status, 0, sizeof(status));
        sprintf(status, " %d", shared_secret->u.plaintext.keyMaterial[i]);
        UART2_write(uart, status, sizeof(status), NULL);
    }
    UART2_write(uart, "\n\r", 2, NULL);

    rslt = ECDH_computeSharedSecret(handle_ecdh, &operationComputeSharedSecret);

    switch (rslt) {
        case ECDH_STATUS_ERROR:
            mx_say_err("ECDH_STATUS_ERROR");
            break;
        case ECDH_STATUS_RESOURCE_UNAVAILABLE:
            mx_say_err("ECDH_STATUS_RESOURCE_UNAVAILABLE");
            break;

        case ECDH_STATUS_CANCELED:
            mx_say_err("ECDH_STATUS_CANCELED");
            break;

        case ECDH_STATUS_POINT_AT_INFINITY:
            mx_say_err("ECDH_STATUS_POINT_AT_INFINITY");
            break;

        case ECDH_STATUS_PRIVATE_KEY_ZERO:
            mx_say_err("ECDH_STATUS_PRIVATE_KEY_ZERO");
            break;
        case ECDH_STATUS_SUCCESS:
                UART2_write(uart, "secret AFT:\n", sizeof("secret AFT:\n"), NULL);
                for (i = 0; i < shared_secret->u.plaintext.keyLength; i++) {
                    memset(status, 0, sizeof(status));
                    sprintf(status, " %d", shared_secret->u.plaintext.keyMaterial[i]);
                    UART2_write(uart, status, sizeof(status), NULL);
                }
                UART2_write(uart, "\n\r", 2, NULL);
            break;
        default:
            memset(status, 0, sizeof(status));
            sprintf(status, "!! status %d !!\n\r", rslt);
            UART2_write(uart, status, sizeof(status), NULL);
            mx_say_err("fckn ecdh");
            break;
    }


    ECDH_close(handle_ecdh);

    /* Hash the sharedSecret to a 256-bit buffer */
    /* As the Y-coordinate is derived from the X-coordinate, hashing only the X component (i.e. keyLength/2 bytes)
     * is a relatively common way of deriving a symmetric key from a shared secret if you are not using a dedicated key derivation function. */
    do_sha(shared_secret->u.plaintext.keyMaterial, shared_secret->u.plaintext.keyLength, buf_sha_digest, 0);

    /* AES keys are 128-bit long, so truncate the generated hash */
    memcpy(symetric_key->u.plaintext.keyMaterial, buf_sha_digest, symetric_key->u.plaintext.keyLength);
}
