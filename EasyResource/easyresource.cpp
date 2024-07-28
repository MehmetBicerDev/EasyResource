#include "easyresource.h"
#include "file_util.h"
#include "hash_util.h"

namespace easyresource
{
	std::shared_ptr<skeleton::c_mainresource> mainresource;
	std::string main_resource_path = "";
	bool initialize(const std::string& p_mainresourcefilepath)
	{
		mainresource = std::make_shared<skeleton::c_mainresource>();

		main_resource_path = p_mainresourcefilepath;


		if (!std::filesystem::exists(p_mainresourcefilepath + "\\resource.ms"))
			return false;


		auto file_buffer = file_util::read_file_to_buffer(p_mainresourcefilepath + "\\resource.ms");

		if (!mainresource->read(file_buffer)) return false;

		load_resources();

		return true;
	}

	void delete_resource(uint32_t index)
	{
		if (mainresource->m_resources.size() > index) 
		{
			mainresource->m_resources.erase(easyresource::mainresource->m_resources.begin() + index);
			reload_mainbuffer();
			save_resources();
		}
	}

	void reload_mainbuffer()
	{
		mainresource->main_buffer.clear();

		for (auto m_resource : mainresource->m_resources)
		{
			mainresource->main_buffer.write_bytes(m_resource.m_buffer);
		}
	}

	void load_resources ( )
	{
		mainresource->main_buffer.clear();

		for(auto m_file : mainresource->m_files)
		{
			if(std::filesystem::exists(main_resource_path + "\\resources\\" + m_file.first))
			{
				mainresource->main_buffer.write_bytes(file_util::read_file_to_buffer(main_resource_path + "\\resources\\" + m_file.first), false);
			}
		}

		for(auto m_resource : mainresource->m_resources)
		{
			mainresource->main_buffer.read_bytes(m_resource.m_buffer, m_resource.m_size);
		}
	}

	void extract_resources()
	{
		if (!std::filesystem::exists(main_resource_path + "\\Extract"))
			std::filesystem::create_directory(main_resource_path + "\\Extract");
		mainresource->main_buffer.reset_position();
		for (auto m_resource : mainresource->m_resources)
		{
			data::c_buffer buffer;
			if (mainresource->main_buffer.read_bytes(buffer, m_resource.m_size)) 
			{
				printf("Extracting Resource Size %d | %d\n", m_resource.m_size, buffer.size());

				//file_util::create_file(main_resource_path + "\\Extract\\" + m_resource.m_name);
				file_util::write_buffer_to_file(main_resource_path + "\\Extract\\" + m_resource.m_name, buffer);
			}
			else printf("failed to read %d | %d\n", mainresource->main_buffer.size(), m_resource.m_size);
		}
	}

	void add_file_to_resources ( const std::string& file_path, const std::string& resource_name )
	{
		if (!std::filesystem::exists(file_path))
			printf("File %s does not exist \n", file_path);
		auto buffer = file_util::read_file_to_buffer(file_path);
		skeleton::c_resource resource(resource_name, file_util::get_file_size(file_path), buffer);
		mainresource->m_resources.push_back(resource);

		mainresource->main_buffer.write_bytes(buffer, false);
	}

	void save_resources ( )
	{

		mainresource->m_files.clear();

		size_t file_count = mainresource->main_buffer.size() / 10000;
		size_t extra_bytes = mainresource->main_buffer.size() % 10000;

		printf("Creating Resources directory \n");
		if (!std::filesystem::exists(main_resource_path + "\\resources"))
			std::filesystem::create_directory(main_resource_path + "\\resources");

		for(uint32_t i = 0; i < file_count;i++)
		{
			std::string file_name = hash_util::hash_string(std::to_string ( i ));
			std::string file_path = main_resource_path + "\\resources\\" + file_name;
			if (std::filesystem::exists(file_path))
				std::filesystem::remove(file_path);
			file_util::create_file(file_path);

			data::c_buffer file_buffer;
			mainresource->main_buffer.read_bytes(file_buffer, file_count > 1 ? 10000 : mainresource->main_buffer.size (  ));
			file_util::write_buffer_to_file(file_path, file_buffer);

			mainresource->m_files.push_back({file_name, 0});

		}

		if(extra_bytes)
		{
			std::string file_name = hash_util::hash_string(std::to_string(file_count + 1));

			file_util::create_file(main_resource_path + "\\resources\\" + file_name);

			data::c_buffer file_buffer;
			mainresource->main_buffer.read_bytes(file_buffer, extra_bytes);
			file_util::write_buffer_to_file(main_resource_path + "\\resources\\" + file_name, file_buffer);
			mainresource->m_files.push_back({ file_name, 0 });

		}
		if (std::filesystem::exists(main_resource_path + "\\resource.ms"))
			std::filesystem::remove(main_resource_path + "\\resource.ms");

		auto mainres_buffer = mainresource->serialize();

		file_util::write_buffer_to_file(main_resource_path + "\\resource.ms", mainres_buffer);
		
	}
}
