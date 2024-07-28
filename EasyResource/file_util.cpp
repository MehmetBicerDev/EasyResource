#include "file_util.h"



namespace file_util
{
	std::string read_file_to_string(const std::string& path)
	{
		std::ifstream file(path, std::ios::in);
		if (!file) {
			return "";
		}

		std::ostringstream contents;
		contents << file.rdbuf();
		file.close();

		return contents.str();
	}
	std::vector<char> read_file_to_vector(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			return std::vector<char>();
		}

		std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		return buffer;
	}
	easyresource::data::c_buffer read_file_to_buffer(const std::string& path) 
	{
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			return easyresource::data::c_buffer();
		}
		std::vector<int8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		return easyresource::data::c_buffer(buffer.data (  ), buffer.size (  ));
	}
	void write_string_to_file(const std::string& path, const std::string& content)
	{
		std::ofstream file(path, std::ios::out | std::ios::trunc);
		if (file) {
			file << content;
			file.close();
		}
	}
	void write_vector_to_file(const std::string& path, const std::vector<char>& content)
	{
		std::ofstream file(path, std::ios::binary);
		if (file) {
			file.write(content.data(), content.size());
			file.close();
		}
	}
	void write_buffer_to_file(const std::string& path, easyresource::data::c_buffer& content)
	{
		std::ofstream file(path, std::ios::binary);
		if (file) {
			file.write((char*)content.buffer (  ), content.size());
			file.close();
		}
	}

	size_t get_file_size ( const std::string& path )
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file) {
			return -1;
		}

		std::size_t fileSize = file.tellg();
		file.close();

		return fileSize;
	}

	void create_file ( const std::string& path )
	{
		std::ofstream file(path, std::ios::out);
		if (file) 
		{
			file.close();
		}
	}
}

