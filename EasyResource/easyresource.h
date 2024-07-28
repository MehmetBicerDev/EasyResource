#pragma once
#include "c_mainresource.h"


namespace easyresource
{
	extern std::shared_ptr<skeleton::c_mainresource> mainresource;
	bool initialize(const std::string& p_mainresourcefilepath);

	void delete_resource(uint32_t index);
	void reload_mainbuffer();
	void load_resources();
	void extract_resources();
	void add_file_to_resources(const std::string& file_path, const std::string& resource_name);
	void save_resources();
}