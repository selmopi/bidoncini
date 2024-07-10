/* WOLFSSL includes. */
#define WOLFSSL_ESPIDF
#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/curve25519.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"

#include "headers/crypto.h"

#define LEN 128      //length of ECC key
#define BYTES_PER_LINE 6
byte my_key_public[LEN];
byte my_key_private[LEN];
RTC_DATA_ATTR bool key_generated = false;
RTC_DATA_ATTR ecc_key my_key;
RTC_DATA_ATTR ecc_key server_pub;

byte server_public_key_bytes[LEN] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xE8, 0x03, 0x3C, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0x01, 0x00,
0x00, 0x00, 0x60, 0x39, 0xCF, 0x3F, 0x43, 0x00, 0x00, 0x00, 0x60, 0x39, 0xCF, 0x3F, 0x5C, 0x30, 0x38, 0x80, 0x90, 0x3A, 0xCF, 0x3F, 0x28, 0x5F, 0x02, 0x42, 0x8C, 0x3F, 0x03, 0x3C,
0xF0, 0x3A, 0xCF, 0x3F, 0xD0, 0x3A, 0xCF, 0x3F, 0x0C, 0x00, 0x00, 0x00, 0xA5, 0xA5, 0xA5, 0xA5, 0xD0, 0x3A, 0xCF, 0x3F, 0x0C, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x21, 0x66, 0x00, 0x82, 0xC0, 0x3A, 0xCF, 0x3F, 0x03, 0x00, 0x00, 0x00};
//"0x0100000001000000000000000000000060E8033C00000000000000000000000000000000A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5010000006039CF3F430000006039CF3F5C303880903ACF3F285F02428C3F033CF03ACF3FD03ACF3F0C000000A5A5A5A5D03ACF3F0C0000001F0000000000000021660082C03ACF3F03000000";
byte my_public_key_bytes[LEN] = {0x04, 0xBE, 0x19, 0xDF, 0x55, 0x0C, 0x70, 0x90, 0xA6, 0x41, 0x8C, 0xFA, 0xEC, 0x52, 0xB3, 0xD7, 0x47, 0x98, 0xE3, 0xAF, 0x88, 0xC2, 0x7E, 0x10,
                                0xCB, 0x6F, 0xC8, 0xBF, 0x39, 0x10, 0xE6, 0xA4, 0xC4, 0x11, 0x78, 0xAC, 0xA2, 0x8C, 0x52, 0xF0, 0xC9, 0x9F, 0x74, 0xA0, 0x24, 0xA2, 0x89, 0xD8,
                                0x27, 0xA9, 0xEF, 0xFD, 0x16, 0x62, 0x60, 0xEC, 0xA2, 0xEC, 0x79, 0x54, 0xE3, 0xB8, 0x1A, 0xBD, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char *TAG_CRYPTO = "CRYPTO_MODULE";


void print_bytes(const uint8_t *bytes, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (i != 0 && i != len - 1 && i % BYTES_PER_LINE == 0)
        {
            printf("\n");
        }

        printf("0x%02X\t", *(bytes + i));
    }
    printf("\n");
}

void generate_keys(){
    //check if my key has already been produced
    if(key_generated){
        return;
    }
    /* GENERATION OF THE  KEYS
    ecc_key key;
    WC_RNG rng;
    word32 len = LEN;

    //initialize ECC key and RNG
    ESP_LOGI(TAG_CRYPTO, "Initializing ECC key and RNG");
    wc_ecc_init(&key);
    wc_InitRng(&rng);
    //generatig ECC key
    ESP_LOGI(TAG_CRYPTO, "Generating ECC key");
    wc_ecc_make_key(&rng, 32, &key);
    my_key = key;
    key_generated = true;
    //export ECC key
    ESP_LOGI(TAG_CRYPTO, "Exporting public and private key");
    wc_ecc_export_x963(&key, my_key_public, &len);
    wc_ecc_export_private_only(&key, my_key_private, &len);

    ESP_LOGI(TAG_CRYPTO, "Printing public key");    
    print_bytes(my_key_public, LEN);
    printf("\n");

    ecc_key key2;
    wc_ecc_init(&key2);
    byte server_key_public[LEN];
    byte server_key_private[LEN]; 
    //generatig ECC key
    wc_ecc_make_key(&rng, 32, &key2);
    //export ECC key
    wc_ecc_export_x963(&key2, server_key_public, &len);
    wc_ecc_export_private_only(&key2, server_key_private, &len);

    print_bytes(server_key_public, LEN);
    printf("\n");

    print_bytes(server_key_private, LEN);
    printf("\n");
    */

    //import server public key
    ESP_LOGI(TAG_CRYPTO, "Importing the server public key");    
    wc_ecc_init(&server_pub);
    wc_ecc_import_x963(server_public_key_bytes, sizeof(server_public_key_bytes), &server_pub);
    wc_ecc_init(&my_key);
    wc_ecc_import_x963(my_public_key_bytes, sizeof(my_public_key_bytes), &my_key);

    WC_RNG rng;
    ecEncCtx* ctx;      //ECC context object
    const byte* salt1;
    ESP_LOGI(TAG_CRYPTO, "Initializing ecEncCtx");
    wc_InitRng(&rng);
    ctx = wc_ecc_ctx_new(REQ_RESP_CLIENT, &rng);
    salt1 = wc_ecc_ctx_get_own_salt( ctx);
    print_bytes(salt1, sizeof(salt1));
    ESP_LOGI(TAG_CRYPTO, "fine primmo sale");
}

void encrypt_value(char* message, word32 msgLen, unsigned char* encrypted, word32* encryptedSz){
    const unsigned char* msg; 
    WC_RNG rng;
    ecEncCtx* ctx;      //ECC context object
    ESP_LOGI(TAG_CRYPTO, "Initializing ecEncCtx");
    wc_InitRng(&rng);
    ctx = wc_ecc_ctx_new(REQ_RESP_CLIENT, &rng);
    wc_ecc_ctx_set_algo(ctx, ecAES_128_CTR, ecHKDF_SHA256, ecHMAC_SHA256);
    msg = (unsigned char*)message;
    wc_ecc_encrypt(&my_key, &server_pub, msg, msgLen, encrypted, encryptedSz, ctx);
}
