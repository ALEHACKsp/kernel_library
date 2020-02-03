#pragma once
#include <string>
#include "module_defs.hpp"

namespace impl
{
	/* summary: find section by name */
	nt::image_section_header* nt_find_section( const nt::rtl_module_info* module_info, const char* wanted_section_name )
	{
		if ( !module_info )
			return nullptr;

		/* get the file (nt) header, stored at image base + e_lfanew, usually fixed at 0x3c*/
		const auto image_file_header = reinterpret_cast< nt::image_nt_headers* >( kernel_module_start + *reinterpret_cast< uint32_t* >( kernel_module_start + 0x3c ) );
		const auto image_section_header = reinterpret_cast< nt::image_section_header* >( image_file_header + 1 );

		for ( auto i = 0; i < image_file_header->file_header.number_of_sections; i++ )
		{
			const auto curr_section = &image_section_header[ i ];

			if ( std::strcmp( reinterpret_cast< const char* >( curr_section->name ), wanted_section_name ) != 0 )
				 continue;

			return curr_section;
		}

		return nullptr;
	}
}