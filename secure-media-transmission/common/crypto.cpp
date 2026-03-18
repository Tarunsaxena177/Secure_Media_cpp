#include "crypto.h"
#include <iomanip>
#include <sstream>
#include <cstring>

namespace Crypto {

    void handleErrors() {
        ERR_print_errors_fp(stderr);
        abort();
    }

    std::string sha256(const std::string& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.c_str(), data.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    std::vector<unsigned char> encrypt(const std::string& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv) {
        EVP_CIPHER_CTX *ctx;
        int len;
        int ciphertext_len;
        std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);

        if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

        if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()))
            handleErrors();

        if(1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (unsigned char*)plaintext.c_str(), plaintext.size()))
            handleErrors();
        ciphertext_len = len;

        if(1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) handleErrors();
        ciphertext_len += len;

        ciphertext.resize(ciphertext_len);
        EVP_CIPHER_CTX_free(ctx);

        return ciphertext;
    }

    std::string decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv) {
        EVP_CIPHER_CTX *ctx;
        int len;
        int plaintext_len;
        std::vector<unsigned char> plaintext(ciphertext.size());

        if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

        if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()))
            handleErrors();

        if(1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()))
            handleErrors();
        plaintext_len = len;

        if(1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) handleErrors();
        plaintext_len += len;

        plaintext.resize(plaintext_len);
        EVP_CIPHER_CTX_free(ctx);

        return std::string(plaintext.begin(), plaintext.end());
    }
}
