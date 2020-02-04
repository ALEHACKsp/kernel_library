#pragma once
#include "memory_scanner_code.hpp"
#include "memory_utility.hpp"

namespace impl
{
	extern "C" NTSYSAPI PCHAR NTAPI PsGetProcessImageFileName( PEPROCESS );

	PEPROCESS nt_find_process( const char* wanted_process_name )
	{
		static const auto relative_sig = scan_for_pattern_code( nt_find_module( "ntoskrnl.exe" ), "\x79\xdc\xe9", "xxx" );

		if ( !relative_sig )
			return nullptr;
		
		static const auto PsGetNextProcess = resolve_call< PEPROCESS( * )( PEPROCESS )>( resolve_jxx( relative_sig ) );

		if ( !PsGetNextProcess )
			return nullptr;

		PEPROCESS previous_process = PsGetNextProcess( nullptr );

		while ( previous_process )
		{
			if ( std::strcmp( wanted_process_name, PsGetProcessImageFileName( previous_process ) ) == 0 )
				return previous_process;

			previous_process = PsGetNextProcess( previous_process );
		}

		return nullptr;
	}
}
