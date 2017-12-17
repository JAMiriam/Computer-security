//
// Created by miriam on 04.12.17.
//

#ifndef LIST6_BSERVER_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <memory>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <chrono>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/rsa.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/bio.h>


#define BN_FLG_CONSTTIME   0x04
#define BN_FLG_SECURE 0x08
#define BUFFER_SIZE 8192
#define LENGTH 32
#define ITER 6000
#define HASH_LEN 20
#define LIST6_BSERVER_H

using namespace std;

class bserver {
private:
    RSA *rsa = NULL;
    BIGNUM *e, *N, *d;
    BN_MONT_CTX *ctx_mont;
    BN_CTX *ctx;
    int is_ok;

    void gen_pass();
    unsigned char* gen_rand_bytes(int size);
    string to_hex(unsigned char *buff, int size);
    void gen_key(int key_length, char *path_to_save);
    bool is_pass_ok(char *user_pass);
    char* sign(BIGNUM *msg);
    bool is_message_ok(BIGNUM *num);
    void get_key(char *path);
public:
    bserver();
    void setup(char* path);
    void server_start(char *password, int port, char *key_path);
    ~bserver();
};

#endif //LIST6_BSERVER_H
