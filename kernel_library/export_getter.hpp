#pragma once
#include "module_getter.hpp"

namespace impl
{
	struct fnv1a_t
	{
		static constexpr inline uint32_t fnv_prime = 0x1000193;
		static constexpr inline uint32_t fnv_basis = 0x811c9dc5;

		__forceinline constexpr static uint32_t single_hash( char c )
		{
			return static_cast< uint32_t >( ( fnv_basis ^ c ) * static_cast< uint64_t >( fnv_prime ) );
		}
	};

	constexpr uint32_t fnv_hash( const char* str, uint32_t val = fnv1a_t::fnv_basis )
	{
		return ( *str ? fnv_hash( str + 1, fnv1a_t::single_hash( *str ) ) : val );
	}

	/* temporarily only works for ntoskrnl.exe, NO caching */
	template <size_t Hash, typename T>
	struct export_fn
	{
		/* summary: walk the export directory of the module, return the export hash that matches the supplied hash */
		T get_fn( )
		{
			static const auto kernel_module_data = nt_find_module( "ntoskrnl.exe" );

			if ( !kernel_module_data )
				return T( );

			static const auto kernel_module_start = kernel_module_data->image_base;

			static const auto export_directory = reinterpret_cast< nt::image_export_dir* >( kernel_module_start + reinterpret_cast< nt::image_nt_headers* >( kernel_module_start + 0x3c )->optional_header.data_directories[ 0 ].virtual_address );

			static const auto address_of_functions = reinterpret_cast< uint32_t* >( kernel_module_start + export_directory->address_of_fn );
			static const auto address_of_names = reinterpret_cast< uint32_t* >( kernel_module_start + export_directory->address_of_name );
			static const auto address_of_ordinals = reinterpret_cast< uint16_t* >( kernel_module_start + export_directory->address_of_ordinals );

			for ( auto i = 0u; i < export_directory->number_of_name; i++ )
			{
				const auto export_name_hash = fnv_hash( reinterpret_cast< const char* >( kernel_module_start + address_of_names[ i ] ) );

				if ( export_name_hash != Hash )
					continue;

				return reinterpret_cast< T >( kernel_module_start + address_of_functions[ address_of_ordinals[ i ] ] );
			}

			return T( );
		}

		/* summary: call the function */
		template <typename... Args>
		decltype( auto ) operator( )( Args&& ...Arguments )
		{
			return get_fn( )( std::forward< Args >( Arguments )... );
		}
	};

#define INVOKE_FN( x ) ::impl::export_fn<::impl::fnv_hash(#x), decltype(&x)>{}

	extern "C" NTSYSAPI PVOID NTAPI RtlFindExportedRoutineByName( PVOID, PCCH );

	/* summary: invoke export UN stealthy, this will call RtlFindExportRoutineByName on every call to it. this is LOUD invoking ! */
	__forceinline void* export_fn_loud( const char* module_name, const char* export_name )
	{
		return RtlFindExportedRoutineByName(
				reinterpret_cast< PVOID >( nt_find_module( module_name )->image_base ),
				export_name
			);
	}
}