//
// Created by miriam on 04.12.17.
//

#ifndef LIST6_BCLIENT_H

#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BN_FLG_CONSTTIME   0x04
#define BN_FLG_SECURE 0x08
#define BUFFER_SIZE 8192
#define LIST6_BCLIENT_H

using namespace std;

class bclient {
private:
    BIGNUM *N, *e, *r;
    BN_CTX *ctx;
    BN_MONT_CTX *ctx_mont;
    const char* hashed;

    void get_pub_key(char *path);
    BIGNUM *prepare_message(char *message);
    string sha256(string str);
    void send_to_sign(int port, char *message);
    void remove_sign(char *signed_message);
    bool bvrfy(BIGNUM *message);
public:
    bclient(int port, char* pub_key_path, char* to_sign);
    ~bclient();
};

#endif //LIST6_BCLIENT_H
