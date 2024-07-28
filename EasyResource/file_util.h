#pragma once
#include "c_buffer.h"






namespace file_util
{
	std::string read_file_to_string(const std::string& path);
	std::vector<char> read_file_to_vector(const std::string& path);
	easyresource::data::c_buffer read_file_to_buffer(const std::string& path);

	void write_string_to_file(const std::string& path, const std::string& content);
	void write_vector_to_file(const std::string& path, const std::vector<char>& content);
	void write_buffer_to_file(const std::string& path, easyresource::data::c_buffer& content);

	size_t get_file_size(const std::string& path);

	void create_file(const std::string& path);

}