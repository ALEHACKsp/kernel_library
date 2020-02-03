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
		auto current_cr0 = __readcr0( );

		static const auto original_cr0 = current_cr0;

		/* flip bit 16 (write protect) */
		current_cr0 &= ~( 1u << 16u );

		KIRQL previous_irql_level = 0;
		KeRaiseIrql( DISPATCH_LEVEL, &previous_irql_level );

		__writecr0( current_cr0 );

		memcpy( address, data, size );

		__writecr0( original_cr0 );

		KeLowerIrql( previous_irql_level );
	}
}