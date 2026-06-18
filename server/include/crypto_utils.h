#pragma once
// crypto_utils.h — SHA-256 hashing utility (OpenSSL EVP API)
//
// Extracted from main.cpp for testability.
// Uses the modern EVP API (OpenSSL 1.1/3.x compatible).

#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

/// SHA-256 hash -> lowercase hex string (64 characters).
/// Returns empty string on OpenSSL internal error.
inline std::string sha256_hex(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return "";
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, input.data(), input.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}
