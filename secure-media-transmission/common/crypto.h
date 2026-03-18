#include <iostream>
#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#ifndef CRYPTO_H
#define CRYPTO_H

namespace Crypto {
    // Constants for AES-256
    const int KEY_SIZE = 32;
    const int IV_SIZE = 16;

    // SHA-256 Hashing
    std::string sha256(const std::string& data);

    // AES-256-CBC Encryption/Decryption
    std::vector<unsigned char> encrypt(const std::string& plaintext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);
    std::string decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);

    void handleErrors();
}

#endif
