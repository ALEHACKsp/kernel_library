#pragma once
#include "memory_scanner_code.hpp"
#include "memory_utility.hpp"

namespace impl
{
	struct threads_t
	{
		std::uint64_t start_address;
		HANDLE thread_id;
	};

	using threads_array_t = std::pair<threads_t*, uint32_t>;

	threads_array_t nt_find_threads( uint64_t process_id )
	{
		PEPROCESS process_out{};

		if ( !NT_SUCCESS( PsLookupProcessByProcessId( reinterpret_cast< HANDLE >( process_id ), &process_out ) ) )
			return {};

		const smart::object<PEPROCESS> process( process_out );

		const auto temporary_buffer = static_cast< threads_t* >( ExAllocatePoolWithTag( PagedPool, 0x10000u, 'erhT' ) );

		if ( !temporary_buffer )
			return {};

		static const auto relative_sig = scan_for_pattern_code( nt_find_module( "ntoskrnl.exe" ), "\xeb\xd0\x48\x8b\x5c\x24", "xxxxxx" );

		if ( !relative_sig )
			return {};

		static const auto call_instruction = resolve_jxx( relative_sig );

		if ( !call_instruction )
			return {};

		static const auto PsGetNextProcessThread = resolve_call< PETHREAD( * )( PEPROCESS, PETHREAD )>( call_instruction );

		if ( !PsGetNextProcessThread )
			return {};

		auto previous_thread = PsGetNextProcessThread( process.get( ), nullptr );

		if ( !previous_thread )
		{
			ExFreePool( temporary_buffer );
			return {};
		}

		auto thread_count = 0u;

		while ( previous_thread )
		{
			/* these differ per windows versions. this is for 1909. */
			temporary_buffer[ thread_count ].start_address = *reinterpret_cast< std::uint64_t* >( std::uintptr_t( previous_thread ) + 0x628 );
			temporary_buffer[ thread_count ].thread_id = ( *reinterpret_cast< CLIENT_ID* >( std::uintptr_t( previous_thread ) + 0x648 ) ).UniqueThread;

			previous_thread = PsGetNextProcessThread( process.get( ), previous_thread );
			thread_count++;
		}

		if ( previous_thread )
			ObfDereferenceObject( previous_thread );

		const auto appropriate_buffer = static_cast< threads_t* >( ExAllocatePoolWithTag( PagedPool, thread_count * sizeof( threads_t ), 'erhT' ) );
		memcpy( appropriate_buffer, temporary_buffer, thread_count * sizeof( threads_t ) );

		ExFreePool( temporary_buffer );

		return { appropriate_buffer, thread_count };
	}
}