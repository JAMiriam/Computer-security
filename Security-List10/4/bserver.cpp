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

    gen_key(2048, path);
    gen_key(4096, path);
    gen_key(8192, path);
    gen_key(16384, path);
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
