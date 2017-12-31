//
// Created by miriam on 04.12.17.
//

#include "bserver.h"

bserver::bserver() {
    e = BN_new();
    ctx = BN_CTX_new();
}

void bserver::setup(char* path) {
    is_ok = BN_set_word(e, RSA_F4);     //generating rsa key from known public key e
    if(is_ok != 1) {
        cout << "Error. Could not generate key." << endl;
    }

    gen_pass();
    gen_safe_key(64, path);
    gen_weak_key(64, path);
}

string bserver::to_hex(unsigned char *buff, int size) {
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < size; ++i)
        ss << std::setw(2) << std::setfill('0') << (int)buff[i];
    return ss.str();
}


void bserver::gen_pass() {
    auto *p = gen_rand_bytes(LENGTH);
    auto password = to_hex(p, LENGTH);
    cout << "Password: " << password << endl;
    auto *s = gen_rand_bytes(LENGTH);
    auto salt = to_hex(s, LENGTH);

    unsigned char out[HASH_LEN];
    memset(out, 0, sizeof out);

    if(PKCS5_PBKDF2_HMAC(password.c_str(), LENGTH, (const unsigned char *) salt.c_str(), LENGTH, ITER, EVP_sha256(), HASH_LEN, out) != 1) {
        cout << ":(" << endl;
    }

    auto key = to_hex(out, HASH_LEN);
    string out_str = key + "\n" + salt;

    ofstream passo("passo");
    passo << out_str;
}

unsigned char* bserver::gen_rand_bytes(int size) {
    auto *buff = (unsigned char*)(malloc(size + 1));

    if (!RAND_bytes(buff, size)) {
        return NULL;
    }

    return buff;
}

char* bserver::sign(BIGNUM *msg_to_sign) {

    auto start = chrono::high_resolution_clock::now();

    BIGNUM *result = BN_new();
    BN_mod_exp(result, msg_to_sign, d, N, ctx);

    auto end = chrono::high_resolution_clock::now();
    cout << "Signed in: ";
    cout << chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms" << endl;

    char *ret = BN_bn2hex(result);
    BN_free(result);
    return ret;
}

void bserver::gen_key(int key_length, char *path_to_save) {

    auto start = chrono::high_resolution_clock::now();

    rsa = RSA_new();
    is_ok = RSA_generate_key_ex(rsa, key_length, e, NULL);
    if(is_ok != 1) {
        cout << "Error. Enable to generate key" << endl;
    }

    auto end = chrono::high_resolution_clock::now();
    cout << "Key of " << key_length << " bits length generated in: ";
    cout << chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms" << endl;

    const BIGNUM *N = rsa->n;
    const BIGNUM *d = rsa->d;
    const BIGNUM *e = rsa->e;

    char p[50];
    FILE *file;
    string s = to_string(key_length);
    char const *length = s.c_str();

    memset(p, 0, sizeof p);
    strcat(p, path_to_save);
    strcat(p, length);
    strcat(p, "_pub");

    file = fopen(p , "w+");
    BN_print_fp(file, N);
    fprintf(file, "\n");
    BN_print_fp(file, e);
    fclose(file);

    memset(p, 0, sizeof p);
    strcat(p, path_to_save);
    strcat(p, length);
    strcat(p, "_priv");

    file = fopen(p , "w+");
    BN_print_fp(file, N);
    fprintf(file, "\n");
    BN_print_fp(file, d);
    fclose(file);

}

void bserver::gen_safe_key(int key_length, char *path_to_save) {
    auto start = std::chrono::high_resolution_clock::now();

    BIGNUM *p = BN_new();
    BIGNUM *q = BN_new();
    BIGNUM *d_safe = BN_new();
    BIGNUM *e_safe = BN_new();
    BIGNUM *n_safe = BN_new();
    BIGNUM *euler = BN_new();
    BN_generate_prime_ex(p, key_length/2, 1, NULL, NULL, NULL);
    BN_generate_prime_ex(q, key_length/2, 1, NULL, NULL, NULL);

    BN_mul(n_safe, p, q, ctx);

    BIGNUM *one = BN_new();
    BIGNUM *tmp1 = BN_new();
    BIGNUM *tmp2 = BN_new();

    BN_set_word(one, 1);
    BN_sub(tmp1, p, one);
    BN_sub(tmp2, q, one);

    BN_mul(euler, tmp1, tmp2, ctx);

    BIGNUM *gcd = BN_new();
    BN_one(one);

    do {
        BN_rand_range(e_safe, euler);
        BN_gcd(gcd, e_safe, euler, ctx);
    }
    while(BN_cmp(gcd, one) != 0);

    BN_mod_inverse(d_safe, e_safe, euler, ctx);

    auto end = std::chrono::high_resolution_clock::now();
    cout << "Key of " << key_length << " bits length generated in: ";
    cout << chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms" << endl;

    if(is_strong_prime(p))
        cout << "p: " << BN_bn2dec(p) << " is safe prime." << endl;
    if(is_strong_prime(q))
        cout << "q: " << BN_bn2dec(q) << " is safe prime." << endl;

    BN_free(p);
    BN_free(q);
    BN_free(euler);
    BN_free(one);
    BN_free(tmp1);
    BN_free(tmp2);
    BN_free(gcd);


    char pa[50];
    FILE *file;
    std::string s = std::to_string(key_length);
    char const *length = s.c_str();

    memset(pa, 0, sizeof p);
    strcat(pa, path_to_save);
    strcat(pa, length);
    strcat(pa, "_pub");

    file = fopen(pa , "w+");
    BN_print_fp(file, n_safe);
    fprintf(file, "\n");
    BN_print_fp(file, e_safe);
    fclose(file);

    memset(pa, 0, sizeof p);
    strcat(pa, path_to_save);
    strcat(pa, length);
    strcat(pa, "_priv");

    file = fopen(pa , "w+");
    BN_print_fp(file, n_safe);
    fprintf(file, "\n");
    BN_print_fp(file, d_safe);
    fclose(file);

    BN_free(n_safe);
    BN_free(d_safe);
    BN_free(e_safe);
}

//this works rather poorly
void bserver::gen_weak_key(int key_length, char *path_to_save) {
    auto start = std::chrono::high_resolution_clock::now();

    BIGNUM *p = BN_new();
    BIGNUM *q = BN_new();
    BIGNUM *d_weak = BN_new();
    BIGNUM *e_weak = BN_new();
    BIGNUM *n_weak = BN_new();
    BIGNUM *euler = BN_new();
    BIGNUM *max = BN_new();
    BIGNUM *max_4 = BN_new();
    BIGNUM *zero = BN_new();
    BIGNUM *div = BN_new();
    BIGNUM *factor = BN_new();

    BN_set_word(zero, 0);
    do {
        BN_generate_prime_ex(p, key_length / 2, 0, NULL, NULL, NULL);
        BN_generate_prime_ex(q, key_length / 2, 0, NULL, NULL, NULL);
        BN_mul(n_weak, p, q, ctx);

        max = max_factor(p);
        BN_mul(max_4, max, max, ctx);
        BN_mul(max_4, max_4, max_4, ctx);
        BN_div(div, NULL, max_4, n_weak, ctx);
    }
    while(BN_cmp(div, zero) > 0);

    cout << "n: " << BN_bn2dec(n_weak) << endl;
    cout << "max factor: " << BN_bn2dec(max_4) << endl;
    cout << "p: " << BN_bn2dec(p) << " is weak prime." << endl;
    cout << "q: " << BN_bn2dec(q) << endl;

    BIGNUM *one = BN_new();
    BIGNUM *tmp1 = BN_new();
    BIGNUM *tmp2 = BN_new();
    BN_set_word(one, 1);

    BN_sub(tmp1, p, one);
    BN_sub(tmp2, q, one);
    BN_mul(euler, tmp1, tmp2, ctx);

    BIGNUM *gcd = BN_new();
    BN_one(one);

    do {
        BN_rand_range(e_weak, euler);
        BN_gcd(gcd, e_weak, euler, ctx);
    }
    while(BN_cmp(gcd, one) != 0);

    BN_mod_inverse(d_weak, e_weak, euler, ctx);

    auto end = std::chrono::high_resolution_clock::now();
    cout << "Key of " << key_length << " bits length generated in: ";
    cout << chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms" << endl;

    BN_free(gcd);
    BN_free(one);
    BN_free(tmp1);
    BN_free(tmp2);
    BN_free(euler);
    BN_free(max);
    BN_free(max_4);
    BN_free(div);
    BN_free(zero);

    char pa[50];
    FILE *file;
    std::string s = std::to_string(key_length);
    char const *length = s.c_str();

    memset(pa, 0, sizeof p);
    strcat(pa, path_to_save);
    strcat(pa, length);
    strcat(pa, "_pub");

    file = fopen(pa , "w+");
    BN_print_fp(file, n_weak);
    fprintf(file, "\n");
    BN_print_fp(file, e_weak);
    fclose(file);

    memset(pa, 0, sizeof p);
    strcat(pa, path_to_save);
    strcat(pa, length);
    strcat(pa, "_priv");

    file = fopen(pa , "w+");
    BN_print_fp(file, n_weak);
    fprintf(file, "\n");
    BN_print_fp(file, d_weak);
    fclose(file);

    BN_free(n_weak);
    BN_free(e_weak);
    BN_free(d_weak);
}

bool bserver::is_strong_prime(BIGNUM *prime) {

    BIGNUM *one = BN_new();
    BN_set_word(one, 1);

    BN_sub(prime, prime, one);
    BN_add(one, one, one);
    BN_div(prime, NULL, prime, one, ctx);

    int ret = BN_is_prime_fasttest_ex(prime, BN_prime_checks, ctx, 1, NULL);

    BN_free(one);
    if(ret == 0)
        return false;
    return true;
}

BIGNUM* bserver::max_factor(BIGNUM *number) {
    BIGNUM *z = BN_new();
    BIGNUM *one = BN_new();
    BIGNUM *rem = BN_new();
    BN_set_word(one, 1);
    BN_set_word(z, 2);
    vector<BIGNUM*> factorials;

    BN_sub(number, number, one);

    BIGNUM *power = BN_new();
    BN_mul(power, z, z, ctx);

    while(BN_cmp(number, power) >= 0) {
        BN_div(NULL, rem, number, z, ctx);
        if(BN_is_zero(rem)) {
            factorials.push_back(BN_dup(z));
            BN_div(number, NULL, number, z, ctx);
        }
        else {
            BN_add(z, z, one);
        }
        BN_mul(power, z, z, ctx);
    }

    if(BN_cmp(number, one) == 1) {
        factorials.push_back(number);
    }

    BIGNUM *max = BN_new();
    BN_set_word(one, 1);
    BN_set_word(max, 1);

    for(int i = 0; i < factorials.size(); i++) {
        if(BN_cmp(factorials.at(i), max) == 1) {
            max = BN_dup(factorials.at(i));
        }
    }

    BN_free(rem);
    BN_free(z);
    BN_free(power);
    BN_free(one);

    return max;
}

void bserver::server_start(char *password, int port, char *key_path) {
    if(!is_pass_ok(password)) {
        cout << "Incorrect password! :(" << endl;
        return;
    }
    cout << "Correct password. Welcome :)" << endl << endl;

    get_key(key_path);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("server_listen");
        exit(EXIT_FAILURE);
    }

    while(true) {
        cout << endl << "Waiting for message..." << endl;
        if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        read(new_socket, buffer, BUFFER_SIZE);
        BIGNUM *m = BN_new();
        BN_hex2bn(&m, buffer);
        cout << "Received message to sign" << endl;

        if (!is_message_ok(m)) {
            cout << "Message is not ok :(" << endl;
            BN_free(m);
            return;
        }
        cout << "Message is ok :)" << endl;

        cout << "Signing in process... please wait" << endl;
        char *signed_msg = sign(m);
        send(new_socket, signed_msg, strlen(signed_msg), 0);
        cout << "Signed message sent" << endl;
        BN_free(m);
        free(signed_msg);
    }
}

bool bserver::is_pass_ok(char *user_pass) {
    auto *pass_salt = new string[2];
    string pass;
    string salt;
    string el;
    ifstream passo("passo");
    if (passo.is_open()) {
        int i = 0;
        while (getline(passo, el)) {
            pass_salt[i] = el;
            i++;
        }
        passo.close();
    }
    pass = pass_salt[0];
    salt = pass_salt[1];

    unsigned char out[HASH_LEN];
    memset(out, 0, sizeof out);

    if(PKCS5_PBKDF2_HMAC(user_pass, LENGTH, reinterpret_cast<const unsigned char *>(salt.c_str()), LENGTH, ITER, EVP_sha256(), HASH_LEN, out) != 1) {
        cout << "Couldn't hash pass :(" << endl;
    }

    auto key = to_hex(out, HASH_LEN);

    return key == pass;
}

void bserver::get_key(char *path) {
    cout << "Loading private key from: " << path << endl;
    ifstream priv_key_file;
    priv_key_file.open(path);
    string el;

    auto *keys = new string[2];
    int i = 0;
    while(getline(priv_key_file, el)) {
        keys[i] = el;
        i++;
    }

    const char *c = keys[0].c_str();
    BN_hex2bn(&N, c);
    c = keys[1].c_str();
    BN_hex2bn(&d, c);
}

bool bserver::is_message_ok(BIGNUM *num) {
    BIGNUM *gcd = BN_new();
    BIGNUM *one = BN_new();
    BN_gcd(gcd, num, N, ctx);
    int ret = BN_cmp(one, gcd);
    BN_free(gcd);
    BN_free(one);

    return ret != 0;
}

bserver::~bserver() {
    RSA_free(rsa);
    BN_free(e);
    BN_free(N);
    BN_free(d);
    BN_CTX_free(ctx);
}

int main(int argc, char*argv[]) {
    if(argc < 3) {
        cout << "Error! Missing arguments." << endl;
        return -1;
    }

    bserver *server = new bserver();

    if(strcmp(argv[1], "0") == 0) {
        cout << "Setup mode..." << endl;
        server->setup(argv[2]);
        delete(server);
        return 0;
    }

    if(argc < 5) {
        cout << "Error! Missing arguments." << endl;
        delete(server);
        return -1;
    }

    if(strcmp(argv[1], "1") == 0) {
        cout << "Sign mode...\n" << endl;
        server->server_start(argv[4], atoi(argv[2]), argv[3]);
    }
    else {
        cout << "Selected mode does not exist. Choose 0 - setup or 1 - sign" << endl;
    }

    delete(server);
    return 0;
}
