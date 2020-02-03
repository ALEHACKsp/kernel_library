#pragma once
#include <cstdint>
#include <ntifs.h>
#include <stdlib.h>

namespace nt
{
	struct rtl_module_info
	{
		char pad_0[ 0x10 ];
		uint64_t image_base;
		uint32_t image_size;
		uint32_t flags;
		uint16_t load_order_idx;
		uint16_t init_order_idx;
		uint16_t load_count;
		uint16_t file_name_offset;
		uint8_t full_path[ _MAX_PATH - 4 ];
	};

	struct rtl_modules
	{
		uint32_t count;
		rtl_module_info modules[ 1 ];
	};

	struct image_file_header
	{
		uint16_t machine;
		uint16_t number_of_sections;
	};

	struct image_export_dir
	{
		char pad_0[ 0xc ];
		uint32_t name;
		uint32_t base;
		uint32_t number_of_fn;
		uint32_t number_of_name;
		uint32_t address_of_fn;
		uint32_t address_of_name;
		uint32_t address_of_ordinals;
	};

	struct image_section_header
	{
		uint8_t name[ 8 ];

		union
		{
			uint32_t physical_address;
			uint32_t virtual_size;
		} misc;

		uint32_t virtual_address;
		uint32_t size_of_raw_data;
		uint32_t pointer_to_raw_data;
		uint32_t pointer_to_relocations;
		uint32_t pointer_to_line_numbers;
		uint16_t number_of_relocations;
		uint16_t number_of_line_numbers;
		uint32_t characteristics;
	};

	struct image_data_dir
	{
		uint32_t virtual_address;
		uint32_t virtual_size;
	};

	struct image_optional_header
	{
		char pad_0[ 0x70 ];
		image_data_dir data_directories[ 16 ];
	};

	struct image_nt_headers
	{
		uint32_t signature;
		image_file_header file_header;
		image_optional_header optional_header;
	};
}