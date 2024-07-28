#pragma once
#include "c_buffer.h"






namespace easyresource
{
	namespace skeleton
	{
		class c_resource
		{
		public:
			inline c_resource(std::string p_name = "", uint64_t p_size = 0, data::c_buffer p_buffer = data::c_buffer()) : m_name(p_name), m_size(p_size), m_buffer(p_buffer){}

			std::string m_name = "";

			uint64_t m_size = 0;
			data::c_buffer m_buffer;

			data::c_buffer serialize_header()
			{
				data::c_buffer buffer;
				buffer.write_string(m_name);
				buffer.write_uint64(m_size);

				return buffer;
			}
			data::c_buffer serialize()
			{
				data::c_buffer buffer;

				buffer.write_bytes(serialize_header(), false);
				buffer.write_bytes(m_buffer);
			}

			bool read(data::c_buffer& buffer)
			{
				return read_header(buffer ) && buffer.read_bytes(m_buffer);
			}

			bool read_header(data::c_buffer& buffer)
			{
				return (buffer.read_string(m_name) && buffer.read_uint64(m_size));
			}
		};
	}
}