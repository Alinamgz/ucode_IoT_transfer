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
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void mx_recv_n_process_peer_key(void) {
	uint8_t is_rf_ok;

	terminationReason = RF_runCmd(rfHandle,
								  (RF_Op*)&RF_cmdPropRx,
								  RF_PriorityNormal,
								  &callback,
								  RF_EventRxEntryDone);

	is_rf_ok = mx_chck_rf_termination_n_status(terminationReason,
	                                           ((volatile RF_Op*)&RF_cmdPropRx)->status);

	if (is_rf_ok == 1) {
		mx_handle_keypkg(packet, &peer_pub_key);
		mx_generate_aes_key(&private_key,
							&peer_pub_key,
							&shared_secret,
							&symmetric_key);
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

            if (*packetDataPointer != KEY_PKG) {
                mx_say_err("unknown pkg type");
            }

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

    /* Copy the public keys from the packet into the parameters */
    memcpy(peer_pub_key_material, &packet[HEADER_LEN], PUBLIC_KEY_LEN);
    CryptoKeyPlaintext_initKey(peer_pub_key, peer_pub_key_material, PUBLIC_KEY_LEN);

    /* Hash the header and public key component of the message. Pass NULL to use the default parameters */
    memset(hash_buf, 0, sizeof(hash_buf));
    mx_do_sha(packet, HEADER_LEN + PUBLIC_KEY_LEN, hash_buf, 1);

    /* Verify signature of public key */
    ECDSA_OperationVerify_init(&operation_verify);
    operation_verify.curve = &ECCParams_NISTP256;
    operation_verify.theirPublicKey = peer_pub_key;
    operation_verify.hash = hash_buf;
    operation_verify.r = &packet[HEADER_LEN + PUBLIC_KEY_LEN];
    operation_verify.s = &packet[HEADER_LEN + PUBLIC_KEY_LEN + PRIVATE_KEY_LEN];

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
}

//==============================================================================
//==============================================================================

void mx_generate_aes_key(CryptoKey *my_private_key, CryptoKey *peer_pub_key, CryptoKey *shared_secret, CryptoKey *symetric_key) {
    int_fast16_t rslt;

    ECDH_Handle handle_ecdh;
    ECDH_Params ecdh_params;
    ECDH_OperationComputeSharedSecret operationComputeSharedSecret;

    uint8_t buf_sha_digest[SHA2_DIGEST_LENGTH_BYTES_256];

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
            break;
        default:
            mx_say_err("ECDH_computeSharedSecret uncaught");
            break;
    }

    ECDH_close(handle_ecdh);

    /* Hash the sharedSecret to a 256-bit buffer */
    /* As the Y-coordinate is derived from the X-coordinate, hashing only the X component (i.e. keyLength/2 bytes)
     * is a relatively common way of deriving a symmetric key from a shared secret if you are not using a dedicated key derivation function. */
    mx_do_sha(shared_secret->u.plaintext.keyMaterial, shared_secret->u.plaintext.keyLength, buf_sha_digest, 0);

    /* AES keys are 128-bit long, so truncate the generated hash */
    memcpy(symetric_key->u.plaintext.keyMaterial, buf_sha_digest, symetric_key->u.plaintext.keyLength);
}
