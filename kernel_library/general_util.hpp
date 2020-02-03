#pragma once
#include <ntifs.h>

namespace impl
{
	template <size_t N>
	UNICODE_STRING make_unicode( const wchar_t( &str )[ N ] )
	{
		UNICODE_STRING result{};

		result.Buffer = const_cast< wchar_t* >( str );
		result.Length = ( N - 1 ) * 2; /* size of the string, multiplied by the size of 1 wide character. */
		result.MaximumLength = result.Length + 2;

		return result;
	}

	template <size_t N>
	ANSI_STRING make_ansi( const char( &str )[ N ] )
	{
		ANSI_STRING result{};

		result.Buffer = const_cast< char* >( str );
		result.Length = ( N - 1 ); /* size of the string, don't need to multiply since 1 narrow character = 1 byte */
		result.MaximumLength = result.Length + 1;

		return result;
	}
}