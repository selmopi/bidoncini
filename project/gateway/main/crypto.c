#define WOLFSSL_ESPIDF
#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/curve25519.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "include/crypto.h"

#define LEN 128
char *TAG_CRYPTO = "CRYPTO_MODULE";

ecc_key my_key;
ecc_key client_pub;
ecEncCtx* ctx;      //ECC context object

byte client_public_key_bytes[LEN] = {0x04, 0xBE, 0x19, 0xDF, 0x55, 0x0C, 0x70, 0x90, 0xA6, 0x41, 0x8C, 0xFA, 0xEC, 0x52, 0xB3, 0xD7, 0x47, 0x98, 0xE3, 0xAF, 0x88, 0xC2, 0x7E, 0x10,
                                0xCB, 0x6F, 0xC8, 0xBF, 0x39, 0x10, 0xE6, 0xA4, 0xC4, 0x11, 0x78, 0xAC, 0xA2, 0x8C, 0x52, 0xF0, 0xC9, 0x9F, 0x74, 0xA0, 0x24, 0xA2, 0x89, 0xD8,
                                0x27, 0xA9, 0xEF, 0xFD, 0x16, 0x62, 0x60, 0xEC, 0xA2, 0xEC, 0x79, 0x54, 0xE3, 0xB8, 0x1A, 0xBD, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//"0x04BE19DF550C7090A6418CFAEC52B3D74798E3AF88C27E10CB6FC8BF3910E6A4C41178ACA28C52F0C99F74A024A289D827A9EFFD166260ECA2EC7954E3B81ABD10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

byte my_public_key_bytes[LEN] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xE8, 0x03, 0x3C, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0x01, 0x00,
0x00, 0x00, 0x60, 0x39, 0xCF, 0x3F, 0x43, 0x00, 0x00, 0x00, 0x60, 0x39, 0xCF, 0x3F, 0x5C, 0x30, 0x38, 0x80, 0x90, 0x3A, 0xCF, 0x3F, 0x28, 0x5F, 0x02, 0x42, 0x8C, 0x3F, 0x03, 0x3C,
0xF0, 0x3A, 0xCF, 0x3F, 0xD0, 0x3A, 0xCF, 0x3F, 0x0C, 0x00, 0x00, 0x00, 0xA5, 0xA5, 0xA5, 0xA5, 0xD0, 0x3A, 0xCF, 0x3F, 0x0C, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x21, 0x66, 0x00, 0x82, 0xC0, 0x3A, 0xCF, 0x3F, 0x03, 0x00, 0x00, 0x00};

void import_keys(){
    WC_RNG rng;
    //import the keys
    wc_ecc_init(&client_pub);
    wc_ecc_init(&my_key);
    wc_ecc_import_x963(client_public_key_bytes, sizeof(client_public_key_bytes), &client_pub);
    wc_ecc_import_x963(my_public_key_bytes, sizeof(my_public_key_bytes), &my_key);
    //initialize ecEncCtx object for the decryption
    wc_InitRng(&rng);
    ctx = wc_ecc_ctx_new(REQ_RESP_SERVER, &rng);
    wc_ecc_ctx_set_algo(ctx, ecAES_128_CTR, ecHKDF_SHA256, ecHMAC_SHA256);
}

void decrypt_message(u_int8_t* buf, byte* out, word32* outSz){
    word32 bufSz = strlen((char*)buf);
    wc_ecc_decrypt(&my_key, &client_pub, buf, bufSz, out, outSz, ctx);
}