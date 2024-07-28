#pragma once
#include "c_buffer.h"
#include <openssl/sha.h>






namespace hash_util
{
	std::string hash_string(const std::string& input);

	std::string hash_bytes(easyresource::data::c_buffer& input);

	std::string to_hex_string(const unsigned char* hash, size_t length);
}