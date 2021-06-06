/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/

#include "tx_defines.h"

/***** Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

//===============================================================================================================

/* Generate random bytes in the provided buffer up to size using the TRNG */
void mx_generate_random_bytes(CryptoKey *entropy_key) {
    TRNG_Handle handle;
    int_fast16_t rslt;

/*    open a TRNG_Handle with the provided buffer */
    handle = TRNG_open(CONFIG_TRNG_0, NULL);
    if (!handle) {
        mx_say_err("TRNG_open()");
    }

    rslt = TRNG_generateEntropy(handle, entropy_key);
    if (rslt != TRNG_STATUS_SUCCESS) {
        mx_say_err("TRNG_generateEntropy()");
    }

    TRNG_close(handle);
}

//===============================================================================================================

void mx_generate_public_key(CryptoKey *private_key, CryptoKey *public_key) {
    int_fast16_t rslt;
    ECDH_Handle handle;
    ECDH_Params params;
    ECDH_OperationGeneratePublicKey operation;

    mx_generate_random_bytes(private_key);

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

    memset(key_pkg, 0, sizeof(key_pkg));
    memset(hashed_data_buf, 0, sizeof(hashed_data_buf));

    /* load pkg metadata n' publick key to pkg */
    key_pkg[PKG_ID_BYTE] = KEY_PKG;
    memcpy(&key_pkg[HEADER_LEN], public_key->u.plaintext.keyMaterial, public_key->u.plaintext.keyLength);

    /* Perform SHA-2 computation on the data to be signed */
   mx_do_sha(key_pkg, HEADER_LEN + PUBLIC_KEY_LEN, hashed_data_buf, 1);

   mx_print_pkg(hashed_data_buf, sizeof(hashed_data_buf), "PUB KEY GEN hash\n", sizeof("PUB KEY GEN hash\n"));

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

void mx_do_my_keys(void) {
    CryptoKeyPlaintext_initBlankKey(&private_key, private_key_material, PRIVATE_KEY_LEN);
    CryptoKeyPlaintext_initBlankKey(&public_key, public_key_material, PUBLIC_KEY_LEN);

    CryptoKeyPlaintext_initBlankKey(&shared_secret, shared_secret_material, PUBLIC_KEY_LEN);
    CryptoKeyPlaintext_initBlankKey(&symmetric_key, symmetric_key_material, AES_KEY_LEN);

    mx_generate_random_bytes(&private_key);
    mx_generate_public_key(&private_key, &public_key);
}
