#pragma once
#include "module_getter.hpp"

namespace impl
{
	uint8_t* scan_for_pattern_code( const nt::rtl_module_info* module, const char* signature, const char* signature_mask )
	{
		if ( !module )
			return nullptr;

		const auto module_start = reinterpret_cast< uint8_t* >( module->image_base );
		const auto module_size = module_start + module->image_size;

		/* iterate the entire module */
		for ( auto segment = module_start; segment < module_size; segment++ )
		{
			if ( [ & ]( const uint8_t* bytes ) -> bool
				 {
					 auto sig_as_bytes = reinterpret_cast< uint8_t* >( const_cast< char* >( signature ) );

						 /* iterate through validity of the mask, mask sz is essentially equal to the byte sequence specified in signature */
						 for ( ; *signature_mask; ++signature_mask, ++bytes, ++sig_as_bytes )
						 {
							 /* if the signature mask is 'x' ( a valid byte, not an always match / wildcard ), and the current byte is not equal to the byte in the sig, then break */
							 if ( *signature_mask == 'x' && *bytes != *sig_as_bytes )
								 return false;
						 }

					 return ( *signature_mask ) == 0;
				 }( segment )
					 )
				return segment;
		}

		return nullptr;
	}
}