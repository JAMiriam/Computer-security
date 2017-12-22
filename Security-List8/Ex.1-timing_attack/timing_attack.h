//
// Created by miriam on 19.12.17.
//

#ifndef TIMINGATTACK_TIMING_ATTACK_H
#include <iostream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <memory>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <unistd.h>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <vector>
#include <chrono>
#include <arpa/inet.h>
#include <netdb.h>
#include <random>
#include <algorithm>
#include <sstream>

#define BUFFER_SIZE 8192
#define TIMINGATTACK_TIMING_ATTACK_H

class timing_attack {
public:
    BIGNUM *N, *d, *e, *r;
    BN_CTX *ctx;
    int key_length;
    std::string real_key;
    std::vector<int> results;

    timing_attack(char *priv_key, char *pub_key);
    std::string hex_str_to_bin_str(const std::string& hex);
    const char* hex_char_to_bin(char c);
    int get_key(char *path, BIGNUM *x, BIGNUM *y, std::string* key);
    double sgn_message(BIGNUM *msg_to_sign, BIGNUM *expo, BIGNUM *modulus);
    std::vector<std::string> split(const std::string& s, char delimiter);

    void attack();
    BIGNUM *gen_message();
    std::string sha256(std::string str);
    uint64_t rdtsc();
    uint64_t cpu_time(BIGNUM *msg, BIGNUM *expo, BIGNUM *modulus);
    void mod_exp(BIGNUM *x, BIGNUM *y, BIGNUM *m);
    double calc_variance(std::vector<double> date);
    void compare_var(double var0, double var1);
};

#endif //TIMINGATTACK_TIMING_ATTACK_H
