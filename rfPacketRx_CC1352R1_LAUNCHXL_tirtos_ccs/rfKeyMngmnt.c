/***** Includes *****/
/* Standard C Libraries */
#include "rx_includes.h"

/***** Defines *****/

#include "rx_defines.h"

/***** Prototypes *****/
#include "rx_glob_vars.h"

/***** Function definitions *****/
#include "rx_functions.h"


inline void do_sha (uint8_t* src, uint32_t src_len, uint8_t *rslt_buf) {
    SHA2_Handle handle_sha2;
    int_fast16_t rslt;

    /* Hash the sharedSecret to a 256-bit buffer */
    handle_sha2 = SHA2_open(CONFIG_SHA2_0, NULL);
    if (!handle_sha2) {
        mx_say_err("SHA2_open @ aes_key");
    }

    /* As the Y-coordinate is derived from the X-coordinate, hashing only the X component (i.e. keyLength/2 bytes)
     * is a relatively common way of deriving a symmetric key from a shared secret if you are not using a dedicated key derivation function. */

    rslt = SHA2_hashData(handle_sha2, src, src_len, rslt_buf);

    if (rslt != SHA2_STATUS_SUCCESS) {
        mx_say_err("SHA2_hashData @ aes_key");
    }

    SHA2_close(handle_sha2);
}

//===============================================================================================================


/* Generate random bytes in the provided buffer up to size using the TRNG */
void mx_generate_random_bytes(CryptoKey *entropy_key) {
    TRNG_Handle handle;
    int_fast16_t rslt;
//    int i = 0;

    /*    call driver init funtion */
    TRNG_init();

/*    open a TRNG_Handle with the provided buffer */
    handle = TRNG_open(CONFIG_TRNG_0, NULL);

    if (!handle) {
        mx_say_err("TRNG_open()");
    }

    rslt = TRNG_generateEntropy(handle, entropy_key);

/*    err handling */
    if (rslt != TRNG_STATUS_SUCCESS) {
        mx_say_err("TRNG_generateEntropy()");
    }

//    for (i = 0; i < 10; i++) {
//            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
//            usleep(250000);
//        }

    TRNG_close(handle);
}


//===============================================================================================================


void mx_generate_public_key(CryptoKey *private_key, CryptoKey *public_key) {
    int_fast16_t rslt;
    ECDH_Handle handle;
    ECDH_Params params;
    ECDH_OperationGeneratePublicKey operation;

    mx_generate_random_bytes(private_key);

    ECDH_init();
    ECDH_Params_init(&params);
    params.returnBehavior = ECDH_RETURN_BEHAVIOR_BLOCKING;
    handle = ECDH_open(CONFIG_ECDH_0, &params);

    if (!handle) {
        mx_say_err("ECDH_open()");
    }

    ECDH_OperationGeneratePublicKey_init(&operation);
    operation.curve = &ECCParams_NISTP256;
    operation.myPrivateKey = private_key;
    operation.myPublicKey = public_key;

    rslt = ECDH_generatePublicKey(handle, &operation);

    if (rslt != ECDH_STATUS_SUCCESS) {
        mx_say_err("ECDH_generatePublicKey()");
    }

    ECDH_close(handle);
}

//===============================================================================================================

void mx_create_publick_key_pkg(uint8_t *key_pkg, CryptoKey *private_key, CryptoKey *public_key) {
    int_fast16_t rslt;

    ECDSA_Handle handle_ecdsa;
    ECDSA_OperationSign operation_sign;

    uint8_t hashed_data_buf[PRIVATE_KEY_LEN];

    char check_msg[5];
    int i;

    SHA2_init();
    ECDSA_init();

//load pkg metadata n' publick key to pkg
    memset(key_pkg, 0, sizeof(key_pkg));
    key_pkg[PKG_ID_BYTE] = KEY_PKG;
//    key_pkg[PKG_LEN_BYTE] = KEY_PKG_LEN;
    memcpy(&key_pkg[HEADER_LEN], public_key->u.plaintext.keyMaterial, public_key->u.plaintext.keyLength);

/* Perform SHA-2 computation on the data to be signed */
    do_sha(key_pkg, HEADER_LEN + PRIVATE_KEY_LEN, hashed_data_buf);


UART2_write(uart, "PUB KEY GEN hash\n", sizeof("PUB KEY GEN hash\n"), NULL);
    for (i = 0; i < SHA2_DIGEST_LENGTH_BYTES_256; i++) {
        memset(check_msg, 0, sizeof(check_msg));
        sprintf(check_msg, " %d", hashed_data_buf[i]);
        UART2_write(uart, check_msg, sizeof(check_msg), NULL);
    }

    UART2_write(uart, "\n\r", sizeof("\n\r"), NULL);

    /* Sign some key data to verify to the receiver that this unit has the corresponding private key to the transmit public key pair */

    ECDSA_OperationSign_init(&operation_sign);
    operation_sign.curve = &ECCParams_NISTP256;
    operation_sign.myPrivateKey = private_key;
    operation_sign.hash= hashed_data_buf;
    operation_sign.r = &key_pkg[HEADER_LEN + PUBLIC_KEY_LEN];
    operation_sign.s = &key_pkg[HEADER_LEN + PUBLIC_KEY_LEN + PRIVATE_KEY_LEN];

    handle_ecdsa = ECDSA_open(CONFIG_ECDSA_0, NULL);
    if (!handle_ecdsa) {
        mx_say_err("ECDSA_open()");
    }

    rslt = ECDSA_sign(handle_ecdsa, &operation_sign);
    if (rslt != ECDSA_STATUS_SUCCESS) {
        mx_say_err("ECDSA_sign");
    }

    ECDSA_close(handle_ecdsa);
}

//===============================================================================================================

void mx_send_key(void) {
    UART2_write(uart, "ama tuta\n\r", sizeof("ama tuta\n\r"), NULL);

    terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
    UART2_write(uart, "ama tuta\n\r", sizeof("ama tuta\n\r"), NULL);

    switch(terminationReason) {
        case RF_EventLastCmdDone:
    UART2_write(uart, "Sending key pkg done\n\r", sizeof("Sending key pkg done\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
            GPIO_toggle(CONFIG_GPIO_LED_RED);

            sleep(1);

            GPIO_toggle(CONFIG_GPIO_LED_GREEN);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            break;
        case RF_EventCmdCancelled:
    UART2_write(uart, "Sending key pkg ERR RF_EventCmdCancelled\n\r", sizeof("Sending key pkg ERR RF_EventCmdCancelled\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Command cancelled before it was started; it can be caused
        // by RF_cancelCmd() or RF_flushCmd().
            break;
        case RF_EventCmdAborted:
    UART2_write(uart, "Sending key pkg ERR RF_EventCmdAborted\n\r", sizeof("Sending key pkg ERR RF_EventCmdAborted\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Abrupt command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        case RF_EventCmdStopped:
    UART2_write(uart, "Sending key pkg ERR RF_EventCmdStopped\n\r", sizeof("Sending key pkg ERR RF_EventCmdStopped\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Graceful command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        default:
    UART2_write(uart, "Sending key pkg Uncaught ERR\n\r", sizeof("Sending key pkg Uncaught ERR\n\r"), NULL);
            GPIO_toggle(CONFIG_GPIO_LED_RED);
            // Uncaught error event
            while(1);
    }
}


//===============================================================================================================

//void COM_generateAesKey(CryptoKey *myPrivateKey, CryptoKey *theirPublicKey, CryptoKey *sharedSecret, CryptoKey *symmetricKey) {
//    int_fast16_t operationResult;
//
//    ECDH_Handle ecdhHandle;
//    ECDH_OperationComputeSharedSecret operationComputeSharedSecret;
//
//    uint8_t entropyBuffer[SHA2_DIGEST_LENGTH_BYTES_256];
//
//    char status[128];
//    int i;
//
//    /* Since we are using default ECDH_Params, we just pass in NULL for that parameter. */
//    ecdhHandle = ECDH_open(CONFIG_ECDH_0, NULL);
//
//    /* The ECC_NISTP256 struct is provided in ti/drivers/types/EccParams.h and the corresponding device-specific implementation. */
//    ECDH_OperationComputeSharedSecret_init(&operationComputeSharedSecret);
//    operationComputeSharedSecret.curve = &ECCParams_NISTP256;
//    operationComputeSharedSecret.myPrivateKey = myPrivateKey;
//    operationComputeSharedSecret.theirPublicKey = theirPublicKey;
//    operationComputeSharedSecret.sharedSecret = sharedSecret;
//
////    chck lengths
//    memset(status, 0, sizeof(status));
//    sprintf(status, "lengths: curve %d; my_priv: %d; their_pub: %d; secret: %d\n\r",
//            operationComputeSharedSecret.curve->length,
//            operationComputeSharedSecret.myPrivateKey->u.plaintext.keyLength,
//            operationComputeSharedSecret.theirPublicKey->u.plaintext.keyLength,
//            operationComputeSharedSecret.sharedSecret->u.plaintext.keyLength);
//    UART2_write(uart, status, sizeof(status), NULL);
//
//
//    UART2_write(uart, "secret BEF:\n", sizeof("secret BEF:\n"), NULL);
//    for (i = 0; i < sharedSecret->u.plaintext.keyLength; i++) {
//        memset(status, 0, sizeof(status));
//        sprintf(status, " %d", sharedSecret->u.plaintext.keyMaterial[i]);
//        UART2_write(uart, status, sizeof(status), NULL);
//    }
//    UART2_write(uart, "\n\r", 2, NULL);
//
//    /* Compute the shared secret and copy it to sharedSecretKeyingMaterial */
//    operationResult = ECDH_computeSharedSecret(ecdhHandle, &operationComputeSharedSecret);
//    if (operationResult != ECDH_STATUS_SUCCESS) {
//                    memset(status, 0, sizeof(status));
//                    sprintf(status, "!! status %d !!\n\r", operationResult);
//                    UART2_write(uart, status, sizeof(status), NULL);
//mx_say_err("FCKN ECDH");
//            while (1)
//            ;
//    }
//    ECDH_close(ecdhHandle);
//
//    UART2_write(uart, "secret AFT:\n", sizeof("secret AFT:\n"), NULL);
//    for (i = 0; i < sharedSecret->u.plaintext.keyLength; i++) {
//        memset(status, 0, sizeof(status));
//        sprintf(status, " %d", sharedSecret->u.plaintext.keyMaterial[i]);
//        UART2_write(uart, status, sizeof(status), NULL);
//    }
//    UART2_write(uart, "\n\r", 2, NULL);
//
//
//    /* Hash the sharedSecret to a 256-bit buffer */
////    handleHash = SHA2_open(CONFIG_SHA2_0, NULL);
//
//    /* As the Y-coordinate is derived from the X-coordinate, hashing only the X component (i.e. keyLength/2 bytes)
//     * is a relatively common way of deriving a symmetric key from a shared secret if you are not using a dedicated key derivation function. */
////    operationResult = SHA2_hashData(handleHash, sharedSecret->u.plaintext.keyMaterial, sharedSecret->u.plaintext.keyLength/2, entropyBuffer);
////
////    if (operationResult != SHA2_STATUS_SUCCESS) {
////        while (1)
////            ;
////    }
////
////
////    SHA2_close(handleHash);
//    do_sha(sharedSecret->u.plaintext.keyMaterial, sharedSecret->u.plaintext.keyLength/2, entropyBuffer);
//
//    /* AES keys are 128-bit long, so truncate the generated hash */
//    memcpy(symmetricKey->u.plaintext.keyMaterial, entropyBuffer, symmetricKey->u.plaintext.keyLength);
//
//    UART2_write(uart, "SYMETRIC:\n", sizeof("SYMETRIC:\n"), NULL);
//    for (i = 0; i < symmetricKey->u.plaintext.keyLength; i++) {
//        memset(status, 0, sizeof(status));
//        sprintf(status, " %d", symmetricKey->u.plaintext.keyMaterial[i]);
//        UART2_write(uart, status, sizeof(status), NULL);
//    }
//    UART2_write(uart, "\n\r", 2, NULL);
//}

void mx_generate_aes_key(CryptoKey *my_private_key, CryptoKey *peer_pub_key, CryptoKey *shared_secret, CryptoKey *symetric_key) {
    int_fast16_t rslt;

    ECDH_Handle handle_ecdh;
    ECDH_Params ecdh_params;
    ECDH_OperationComputeSharedSecret operationComputeSharedSecret;

    uint8_t buf_sha_digest[SHA2_DIGEST_LENGTH_BYTES_256];

    int i = 0;
    char status[64];
    memset(status, 0, 64);

    ECDH_init();

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
    do_sha(shared_secret->u.plaintext.keyMaterial, shared_secret->u.plaintext.keyLength, buf_sha_digest);

    /* AES keys are 128-bit long, so truncate the generated hash */
    memcpy(symetric_key->u.plaintext.keyMaterial, buf_sha_digest, symetric_key->u.plaintext.keyLength);

}

//===============================================================================================================


void mx_check_keys(CryptoKey *private_key, CryptoKey *public_key, CryptoKey *peer_pub_key) {
    int i = 0;
    char check_msg[32];

    UART2_write(uart, "PRIVATE:\n", sizeof("PRIVATE:\n"), NULL);
    for (i  = 0; i < PRIVATE_KEY_LEN; i++) {
        memset(check_msg, 0, sizeof(check_msg));
        sprintf(check_msg, " %d", private_key->u.plaintext.keyMaterial[i]);
        UART2_write(uart, check_msg, sizeof(check_msg), NULL);
    }

    UART2_write(uart, "\n\r PUBLIC:\n", sizeof("\n\r PUBLIC:\n"), NULL);
    for (i  = 0; i < PUBLIC_KEY_LEN; i++) {
        memset(check_msg, 0, sizeof(check_msg));
        sprintf(check_msg, " %d", public_key->u.plaintext.keyMaterial[i]);
        UART2_write(uart, check_msg, sizeof(check_msg), NULL);
    }

    UART2_write(uart, "\n\r   PEER:\n", sizeof("\n\r   PEER:\n"), NULL);
    for (i  = 0; i < PUBLIC_KEY_LEN; i++) {
        memset(check_msg, 0, sizeof(check_msg));
        sprintf(check_msg, " %d", peer_pub_key->u.plaintext.keyMaterial[i]);
        UART2_write(uart, check_msg, sizeof(check_msg), NULL);
    }

    UART2_write(uart, "\n\r----- ----- -----\n\r", sizeof("\n\r----- ----- -----\n\r"), NULL);
}

//===============================================================================================================

void mx_handle_keypkg(uint8_t *packet, CryptoKey *peer_pub_key) {
    ECDSA_Handle handle_verify;
    ECDSA_OperationVerify operation_verify;

    int_fast16_t rslt;
    uint8_t hash_buf[PRIVATE_KEY_LEN];

    /* Copy the public keys from the packet into the parameters */
    memcpy(peer_pub_key_material, &packet[HEADER_LEN], PUBLIC_KEY_LEN);
    CryptoKeyPlaintext_initKey(peer_pub_key, peer_pub_key_material, PUBLIC_KEY_LEN);



    /* Hash the header and public key component of the message. Pass NULL to use the default parameters */
    do_sha(packet, HEADER_LEN + PUBLIC_KEY_LEN, hash_buf);
UART2_write(uart, "sha done\n\r", sizeof("sha done\n\r"), NULL);

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

//===============================================================================================================

void mx_do_keys(void) {
    uint8_t private_key_material[PRIVATE_KEY_LEN];
    uint8_t public_key_material[PUBLIC_KEY_LEN];

    uint8_t peer_priv_key_material[PRIVATE_KEY_LEN] = {0};
//    uint8_t peer_pub_key_material[PUBLIC_KEY_LEN] = {0};

    uint8_t shared_secret_material[PUBLIC_KEY_LEN];
    uint8_t symmetric_key_material[AES_KEY_LEN] = {0};

    CryptoKey private_key;
    CryptoKey public_key;

    CryptoKey peer_priv_key;
//    CryptoKey peer_pub_key;

    CryptoKey shared_secret;
    CryptoKey symmetric_key;

    CryptoKeyPlaintext_initBlankKey(&private_key, private_key_material, PRIVATE_KEY_LEN);
    CryptoKeyPlaintext_initBlankKey(&public_key, public_key_material, PUBLIC_KEY_LEN);

    CryptoKeyPlaintext_initBlankKey(&peer_priv_key, peer_priv_key_material, PRIVATE_KEY_LEN);
//    CryptoKeyPlaintext_initBlankKey(&peer_pub_key, peer_pub_key_material, PUBLIC_KEY_LEN);

    CryptoKeyPlaintext_initBlankKey(&shared_secret, shared_secret_material, PUBLIC_KEY_LEN);
    CryptoKeyPlaintext_initBlankKey(&symmetric_key, symmetric_key_material, AES_KEY_LEN);

    mx_generate_random_bytes(&private_key);
    mx_generate_public_key(&private_key, &public_key);

    mx_check_keys(&private_key, &public_key, &peer_pub_key);
    mx_create_publick_key_pkg(packet, &private_key, &public_key);


    //   ===========================  !!! JUST FOR TEST !!!  ==============================
//        mx_generate_random_bytes(&peer_priv_key);
//        mx_generate_public_key(&peer_priv_key, &peer_pub_key);

    //   ========================  ===========================  ===========================

//        COM_generateAesKey(&private_key, &peer_pub_key, &shared_secret, &symmetric_key);
//        mx_generate_aes_key(&private_key, &peer_pub_key, &shared_secret, &symmetric_key);

//UART2_write(uart, "now have to send key pkg \n\r", sizeof("now have to send key pkg \n\r"), NULL);
//    mx_send_key();
}
