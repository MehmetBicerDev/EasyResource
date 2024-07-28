#include "hash_util.h"

#pragma warning(disable : 4996)

std::string hash_util::hash_string(const std::string& input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);
    return to_hex_string(hash, SHA256_DIGEST_LENGTH);
}

std::string hash_util::hash_bytes ( easyresource::data::c_buffer& input )
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.buffer(), input.size());
    SHA256_Final(hash, &sha256);
    return to_hex_string(hash, SHA256_DIGEST_LENGTH);
}

std::string hash_util::to_hex_string ( const unsigned char* hash, size_t length )
{
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}
