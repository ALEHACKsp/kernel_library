#pragma once
#include "raii_defs.hpp"
#include "module_defs.hpp"

namespace impl
{
	extern "C" NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation( ULONG, PVOID, ULONG, PULONG );

	/* summary: query the current system module list, walk through it to find the module with the supplied name */
	nt::rtl_module_info* nt_find_module( const char* wanted_module_name )
	{
		uint32_t buffer_bytes_sz = 8192;
		smart::alloc buffer_bytes( ExAllocatePoolWithTag( PagedPool, buffer_bytes_sz, 'kciD' ) );

		if ( !buffer_bytes )
			return nullptr;

		auto last_status = ZwQuerySystemInformation( 11 /* SystemModuleInformation */, buffer_bytes.get( ), buffer_bytes_sz, reinterpret_cast< PULONG >( &buffer_bytes_sz ) /* :) */ );

		while ( last_status == STATUS_INFO_LENGTH_MISMATCH )
		{
			buffer_bytes.reset( ExAllocatePoolWithTag( PagedPool, buffer_bytes_sz, 'kciD' ) );

			if ( !buffer_bytes )
				return nullptr;

			last_status = ZwQuerySystemInformation( 11 /* SystemModuleInformation */, buffer_bytes.get( ), buffer_bytes_sz, reinterpret_cast< PULONG >( &buffer_bytes_sz ) /* :) */ );
		}

		if ( !NT_SUCCESS( last_status ) )
			return nullptr;

		const auto module_list = reinterpret_cast< nt::rtl_modules* >( buffer_bytes.get( ) );

		for ( auto i = 0u; i < module_list->count; i++ )
		{
			const auto curr_module = &module_list->modules[ i ];
			
			/* full_path is the complete system path of the module, file_name_offset is the offset from the start of the array till the last slash to the module's name. */
			const auto curr_module_name = reinterpret_cast< char* >( curr_module->full_path ) + curr_module->file_name_offset;

			/* return value 0: strings are both equal, else return occurence of mismatch */
			if ( std::strcmp( curr_module_name, wanted_module_name ) != 0 )
				continue;

			return curr_module;
		}

		return nullptr;
	}
}