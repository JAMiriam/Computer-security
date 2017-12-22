//
// Created by miriam on 19.12.17.
//

#include "timing_attack.h"
#define ITERS 50000

using namespace std;

timing_attack::timing_attack(char *priv_key, char *pub_key) {
    ctx = BN_CTX_new();
    N = BN_new();
    d = BN_new();
    e = BN_new();
    r = BN_new();

    string key;
    key_length = get_key(priv_key, N, d, &real_key);
    get_key(pub_key, N, e, &key);

    attack();
}

int timing_attack::get_key(char *path, BIGNUM *x, BIGNUM *y, string *key) {
    ifstream file;
    file.open(path);
    string line;

    auto *keys = new string[2];
    int i = 0;
    while(getline(file, line)) {
        keys[i] = line;
        i++;
    }
    int length = keys[1].size()*4;

    *key = hex_str_to_bin_str(keys[1]);

    const char *c = keys[0].c_str();
    BN_hex2bn(&x, c);
    c = keys[1].c_str();
    BN_hex2bn(&y, c);

    return length;
}

const char* timing_attack::hex_char_to_bin(char c) {
    switch(toupper(c)) {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
        default : return "0000";
    }
}

string timing_attack::hex_str_to_bin_str(const std::string& hex) {
    string bin;
    for(unsigned i = 0; i != hex.length(); ++i)
        bin += hex_char_to_bin(hex[i]);
    return bin;
}

vector<string> timing_attack::split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}



void timing_attack::attack() {
    vector<int> known_zeros;
    vector<int> known_ones;
    string known_bits_str;
    cout << "Private key: " << real_key << endl;
    cout << "Do you know some bits? Support inserting no:x-no:x-no:x" << endl;
    cin >> known_bits_str;

    vector<string> temp = split(known_bits_str, '-');
    vector<string> small_temp;
    for (string el : temp) {
        small_temp = split(el, ':');
        if(small_temp[1]=="1")
            known_ones.push_back(stoi(small_temp[0]));
        else
            known_zeros.push_back(stoi(small_temp[0]));
        small_temp.clear();
        small_temp.shrink_to_fit();
    }

    BIGNUM *private_key = BN_new();
    BIGNUM *d_real = BN_new();
    BIGNUM *z = BN_new();
    BIGNUM *o = BN_new();
    BIGNUM *power_2 = BN_new();
    BIGNUM *two = BN_new();
    string final_key = "1";

    int counts = 0;

    vector<double> zeros;
    vector<double> ones;

    BN_set_word(d_real, 1);
    BN_set_word(private_key, 1);
    BN_set_word(power_2, 2);
    BN_set_word(two, 2);

    double server_time0, server_time1, local_time0, local_time1, diff0, diff1;
    double var0, var1;

    for(int i = 2; i < key_length+1; i++) {

        if(real_key[key_length-i] == '1')
            BN_add(d_real, private_key, power_2);

        if(count(known_zeros.begin(), known_zeros.end(), i)) {
            cout << "Bit number "<< i << endl;
            cout << "Zero known" << endl << endl << endl;
            final_key = final_key + '0';
            continue;
        }
        else if(count(known_ones.begin(), known_ones.end(), i)) {
            BN_add(private_key, private_key, power_2);
            cout << "Bit number "<< i << endl;
            cout << "One known" << endl << endl << endl;
            final_key = final_key + '1';
            continue;
        }
        else {
            BN_copy(z, private_key);
            BN_add(o, private_key, power_2);
            cout << "Bit number "<< i << endl;
        }

        for(int j = 0; j < ITERS; j++) {
            BIGNUM *rand_msg = gen_message();
            usleep(1);
            server_time0 = sgn_message(rand_msg, d_real, N);
            usleep(1);
            local_time0 = sgn_message(rand_msg, z, N);

            diff0 = server_time0 - local_time0;
            zeros.push_back(abs(diff0));
            BN_free(rand_msg);
        }
        for(int j = 0; j < ITERS; j++) {
            BIGNUM *rand_msg = gen_message();
            usleep(1);
            server_time1 = sgn_message(rand_msg, d_real, N);
            usleep(1);
            local_time1 = sgn_message(rand_msg, o, N);

            diff1 = server_time1 - local_time1;
            ones.push_back(abs(diff1));
            BN_free(rand_msg);
        }

        var0 = calc_variance(zeros);
        var1 = calc_variance(ones);

        printf("Variance when zero added: %e\n", var0);
        printf("Variance when one added: %e\n", var1);

        if(counts < 10) {
            compare_var(var0, var1);
            i--;
            cout << "Repeating..." << endl;
            counts++;
            ones.clear();
            ones.shrink_to_fit();
            zeros.clear();
            zeros.shrink_to_fit();
            continue;
        }
        else {
            long int res = count(results.begin(), results.end(), 0);
            if(res < 5) {
                BN_add(private_key, private_key, power_2);
                cout << "One wins" << endl;
                final_key = final_key + '1';
            }
            else {
                cout << "Zero wins" << endl;
                final_key = final_key + '0';
            }
        }

        BN_CTX_start(ctx);
        BN_mul(power_2, power_2, two, ctx);
        BN_CTX_end(ctx);

        cout << "Guessed key by now: ";
        for(int i = final_key.length()-1; i >= 0; i--) {
            cout << final_key[i];
        }
        cout << endl << endl << endl;

        counts = 0;
        ones.clear();
        ones.shrink_to_fit();
        zeros.clear();
        zeros.shrink_to_fit();
        results.clear();
        results.shrink_to_fit();
    }

    BN_free(private_key);
    BN_free(d_real);
    BN_free(z);
    BN_free(o);
    BN_free(power_2);
    BN_free(two);
}

void timing_attack::compare_var(double var0, double var1) {
    if(var0 < var1)
        results.push_back(0);
    else
        results.push_back(1);
}

BIGNUM* timing_attack::gen_message() {
    int length = rand() % 64 + 16;

    static const string::value_type allowed_chars[] {"123456789BCDFGHJKLMNPQRSTVWXZbcdfghjklmnpqrstvwxz"};
    static thread_local default_random_engine randomEngine(random_device{}());
    static thread_local uniform_int_distribution<int> randomDistribution(0, sizeof(allowed_chars) - 1);
    string id(length, '\0');
    for (string::value_type& c : id)
        c = allowed_chars[randomDistribution(randomEngine)];

    const char *message = id.c_str();
    string hashed_msg = sha256(message);
    const char *hashed_msg_char = hashed_msg.c_str();
    BIGNUM *m = BN_new();
    BN_hex2bn(&m, hashed_msg_char);
    BN_hex2bn(&m, message);


    BIGNUM *one = BN_new();
    BIGNUM *gcd = BN_new();
    BN_one(one);
    BN_CTX_start(ctx);

    do {
        BN_rand_range(r, N);
        BN_gcd(gcd,r, N, ctx);
    }
    while(BN_cmp(gcd, one) != 0);

    BIGNUM *x = BN_new();
    BN_mod_exp(x, r, e, N, ctx);
    BN_mod_mul(x, m, x, N, ctx);

    BN_free(one);
    BN_free(gcd);
    BN_free(m);
    BN_CTX_end(ctx);
    return x;
}

string timing_attack::sha256(const string str) {
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

double timing_attack::sgn_message(BIGNUM *msg_to_sign, BIGNUM *expo, BIGNUM *modulus) {
    uint64_t cycles;
    cycles = cpu_time(msg_to_sign, expo, modulus);

    return (double)cycles;
}

uint64_t timing_attack::cpu_time(BIGNUM *msg, BIGNUM *expo, BIGNUM *modulus) {
    uint64_t started, finished;

    started = rdtsc();
    mod_exp(msg, expo, modulus);
    finished = rdtsc();

    return finished - started;
}

uint64_t timing_attack::rdtsc() {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtscp" : "=a" (lo), "=d" (hi));
    return ( ((uint64_t)hi) << (uint32_t)32 )
           | ( ((uint64_t)lo) );
}

void timing_attack::mod_exp(BIGNUM *x, BIGNUM *y, BIGNUM *m) {
    BIGNUM *result = BN_new();
    BIGNUM *zero = BN_new();
    BN_set_word(result, 1);
    BN_set_word(zero, 0);

    while(BN_cmp(y, zero) == 1) {
        if(BN_is_odd(y) == 1) {
            BN_CTX_start(ctx);
            BN_mod_mul(result, result, x, m, ctx);
            BN_CTX_end(ctx);
        }
        BN_rshift1(y, y);
        BN_CTX_start(ctx);
        BN_mod_mul(x, x, x, m, ctx);
        BN_CTX_end(ctx);
    }
    BN_free(zero);
    BN_free(result);
}

double timing_attack::calc_variance(vector<double> date) {
    unsigned long length = date.size();
    unsigned long to_out = length/16;
    double avg = 0;
    sort(date.begin(), date.end());

    for(unsigned long i=to_out; i<length-to_out; i++) {
        avg += date[i];
    }

    avg = avg/(length-(2*to_out));

    double var = 0;
    for(unsigned long i=to_out; i<length-to_out; i++) {
        var += (date[i] - avg)*(date[i] - avg);
    }

    var /= (length-(2*to_out));
    return var;
}


int main(int argc, char* argv[]) {
    if(argc < 3) {
        cout << "Not enough arguments. Insert private and public key paths." << endl;
        return -1;
    }

    auto *attack = new timing_attack(argv[1], argv[2]);

    return 0;
}
