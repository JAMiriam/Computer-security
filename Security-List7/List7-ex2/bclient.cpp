//
// Created by miriam on 04.12.17.
//

#include "bclient.h"

bclient::bclient(int port, char *pub_key_path, char *to_sign) {
    N = BN_new();
    e = BN_new();
    r = BN_new();
    ctx = BN_CTX_new();
    ctx_mont = NULL;

    get_pub_key(pub_key_path);
    BIGNUM *x = prepare_message(to_sign);
    send_to_sign(port, BN_bn2hex(x));
    BN_free(x);
}

void bclient::get_pub_key(char *path) {
    cout << "Loading public key from: " << path << endl;
    string item_name;
    ifstream nameFileout;
    nameFileout.open(path);
    string line;

    auto *temp = new string[2];

    int i = 0;
    while(getline(nameFileout, line)) {
        temp[i] = line;
        i++;
    }

    const char *c = temp[0].c_str();
    BN_hex2bn(&N, c);
    c = temp[1].c_str();
    BN_hex2bn(&e, c);
}

BIGNUM* bclient::prepare_message(char *message) {

    string hashed_msg = sha256(message);

    const char *hashed_msg_char = hashed_msg.c_str();
    BIGNUM *m = BN_new();
    BN_hex2bn(&m, hashed_msg_char);
    hashed = BN_bn2dec(m);

    BIGNUM *one = BN_new();
    BIGNUM *gcd = BN_new();
    BN_one(one);

    do {
        BN_rand_range(r, N);
        BN_gcd(gcd,r, N, ctx);
    }
    while(BN_cmp(gcd, one) != 0);

    BIGNUM *x = BN_new();

    BIGNUM *ec = BN_new();
    BN_with_flags(ec, e, BN_FLG_CONSTTIME);
    BN_mod_exp_mont_consttime(x, r, ec, N, ctx, ctx_mont);
    BN_free(ec);
    BN_mod_mul(x, m, x, N, ctx);

    BN_free(one);
    BN_free(gcd);
    BN_free(m);
    return x;
}

string bclient::sha256(const string str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

void bclient::send_to_sign(int port, char *message) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char signed_msg[BUFFER_SIZE] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }

    send(sock, message, strlen(message), 0);
    cout << "Message sent to sign" << endl;

    read(sock, signed_msg, BUFFER_SIZE);
    cout << "Received signed message" << endl;

    remove_sign(signed_msg);
//    remove_sign((char *) "cos");
}

void bclient::remove_sign(char *signed_message) {

    BIGNUM *from = BN_new();
    BIGNUM *inverse = BN_new();
    BIGNUM *s = BN_new();
    BN_hex2bn(&from, signed_message);

    BIGNUM *Nc = BN_new();
    BN_with_flags(Nc, N, BN_FLG_CONSTTIME);
    BN_mod_inverse(inverse, r, Nc, ctx);
    BN_free(Nc);
    BN_mod_mul(s, inverse, from, N, ctx);

    if(bvrfy(s))
        cout << "[BVRFY] CORRECT SIGNATURE" << endl;
    else
        cout << "[BVRFY] INCORRECT SIGNATURE" << endl;

    BN_free(from);
    BN_free(inverse);
    BN_free(s);
}

bool bclient::bvrfy(BIGNUM *message) {

    BIGNUM *h = BN_new();
    BIGNUM *ec = BN_new();
    BN_with_flags(ec, e, BN_FLG_CONSTTIME);

    auto start = chrono::high_resolution_clock::now();

    BN_mod_exp(h, message, ec, N, ctx);

    auto end = chrono::high_resolution_clock::now();
    cout << "Verified in: ";
    cout << chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms" << endl;

    BN_free(ec);

    int ret = strcmp(hashed, BN_bn2dec(h));
    BN_free(h);
    return ret == 0;
}

bclient::~bclient() {
    BN_free(N);
    BN_free(e);
    BN_free(r);
    BN_CTX_free(ctx);
    free((char*)hashed);
}

int main(int argc, char*argv[]) {
    if(argc < 4) {
        cout << "Missing arguments" << endl;
        return -1;
    }

    bclient *client = new bclient(atoi(argv[1]), argv[2], argv[3]);
    delete client;
    return 0;
}
