/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/

#include "tx_defines.h"

/***** Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

static inline void mx_generate_random_bytes(CryptoKey *entropy_key);
static void mx_generate_public_key(CryptoKey *private_key, CryptoKey *public_key);

void mx_do_my_keys(void) {
    CryptoKeyPlaintext_initBlankKey(&private_key, private_key_material, PRIVATE_KEY_LEN);
    CryptoKeyPlaintext_initBlankKey(&public_key, public_key_material, PUBLIC_KEY_LEN);

    CryptoKeyPlaintext_initBlankKey(&shared_secret, shared_secret_material, PUBLIC_KEY_LEN);
    CryptoKeyPlaintext_initBlankKey(&symmetric_key, symmetric_key_material, AES_KEY_LEN);

    mx_generate_random_bytes(&private_key);
    mx_generate_public_key(&private_key, &public_key);
}

//===============================================================================================================
//===============================================================================================================
/* Generate random bytes in the provided buffer up to size using the TRNG */
static inline void mx_generate_random_bytes(CryptoKey *entropy_key) {
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
//===============================================================================================================
static void mx_generate_public_key(CryptoKey *private_key, CryptoKey *public_key) {
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
