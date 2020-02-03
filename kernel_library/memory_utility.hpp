#pragma once
#include <cstdint>
#include <intrin.h>
#include <ntifs.h>

namespace impl
{
	template <typename T = uint8_t*>
	T resolve_jxx( uint8_t* address )
	{
		return reinterpret_cast< T >( address + *reinterpret_cast< int8_t* >( address + 1 ) + 2 );
	}

	template <typename T = uint8_t*>
	T resolve_call( uint8_t* address )
	{
		return reinterpret_cast< T >( address + *reinterpret_cast< int32_t* >( address + 1 ) + 5 );
	}

	template <typename T = uint8_t*>
	T resolve_mov( uint8_t* address )
	{
		return reinterpret_cast< T >( address + *reinterpret_cast< int32_t* >( address + 3 ) + 7 );
	}

	void force_write( void* address, void* data, size_t size )
	{
		const std::unique_ptr<MDL, decltype( &IoFreeMdl )> mdl( IoAllocateMdl( address, static_cast< ULONG >( size ), FALSE, FALSE, nullptr ), &IoFreeMdl );

		if ( !mdl )
			return;

		MmProbeAndLockPages( mdl.get( ), KernelMode, IoReadAccess );

		const auto mapped_page = MmMapLockedPagesSpecifyCache( mdl.get( ), KernelMode, MmNonCached, nullptr, FALSE, NormalPagePriority );

		if ( !mapped_page )
			return;

		if ( !NT_SUCCESS( MmProtectMdlSystemAddress( mdl.get( ), PAGE_EXECUTE_READWRITE ) ) )
			return;

		memcpy( mapped_page, data, size );

		MmUnmapLockedPages( mapped_page, mdl.get( ) );
		MmUnlockPages( mdl.get( ) );
	}
}
