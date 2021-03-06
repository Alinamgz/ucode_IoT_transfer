
/***** Includes *****/
/* Standard C Libraries */
#include "tx_includes.h"

/***** Defines *****/
#include "tx_defines.h"

/***** Prototypes *****/
#include "tx_glob_vars.h"

/***** Function definitions *****/
#include "tx_functions.h"

//  prototypes
static void mx_create_publick_key_pkg(uint8_t *key_pkg, CryptoKey *private_key, CryptoKey *public_key);

void mx_share_my_pub_key(void) {
	mx_create_publick_key_pkg(packet, &private_key, &public_key);

    terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, 0);
    // sending doesn't change cmdStatus, so I pass there PROP_DONE_OK to get rid of uncaught err msg
    mx_chck_rf_termination_n_status(terminationReason, PROP_DONE_OK);
}

static void mx_create_publick_key_pkg(uint8_t *key_pkg, CryptoKey *private_key, CryptoKey *public_key) {
    int_fast16_t rslt;

    ECDSA_Handle handle_ecdsa;
    ECDSA_OperationSign operation_sign;

    uint8_t hash_buf[PRIVATE_KEY_LEN];

//load pkg metadata n' publick key to pkg
    memset(key_pkg, 0, sizeof(key_pkg));
    key_pkg[PKG_ID_BYTE] = KEY_PKG;
    memcpy(&key_pkg[HEADER_LEN], public_key->u.plaintext.keyMaterial, public_key->u.plaintext.keyLength);

/* Perform SHA-2 computation on the data to be signed */
    memset(hash_buf, 0, sizeof(hash_buf));
    mx_do_sha(key_pkg, HEADER_LEN + PUBLIC_KEY_LEN, hash_buf, 1);

    /* Sign some key data to verify to the receiver that this unit has the corresponding private key to the transmit public key pair */
    ECDSA_OperationSign_init(&operation_sign);
    operation_sign.curve = &ECCParams_NISTP256;
    operation_sign.myPrivateKey = private_key;
    operation_sign.hash= hash_buf;
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

