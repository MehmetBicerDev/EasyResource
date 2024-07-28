#pragma once
#include "c_resource.h"






namespace easyresource 
{
	namespace skeleton
	{
		class c_mainresource
		{
		public:
			std::vector<std::pair<std::string, uint32_t>> m_files;

			std::vector<c_resource> m_resources;

			data::c_buffer main_buffer;
			

			data::c_buffer serialize()
			{
				data::c_buffer buffer;
				buffer.write_uint(m_files.size());
				for(auto m_file : m_files)
				{
					buffer.write_uint(m_file.second);
					buffer.write_string(m_file.first);
				}
				buffer.write_uint(m_resources.size());
				for(auto m_resource : m_resources)
				{
					buffer.write_bytes(m_resource.serialize_header(), false);
				}
				return buffer;
			}

			bool read(data::c_buffer& buffer)
			{
				uint32_t file_count = 0, resource_count = 0;

				if(buffer.read_uint ( file_count ))
				{
					for(uint32_t i = 0; i < file_count;i++)
					{
						uint32_t crc_hash = 0;
						std::string file_name = "";

						if (!buffer.read_uint(crc_hash) || !buffer.read_string(file_name)) return false;

						m_files.push_back({ file_name, crc_hash });
					}

					if(buffer.read_uint ( resource_count ))
					{
						for(uint32_t i = 0; i < resource_count;i++)
						{
							c_resource resource;
							if (!resource.read_header(buffer)) return false;
							m_resources.push_back(resource);
						}
						return true;
					}
				}
				return false;
			}
		};
	}
}