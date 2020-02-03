#pragma once
#include "module_getter.hpp"

/* thanks to: DefCon42 */
namespace impl
{
	constexpr std::uint8_t char_to_hex( char c )
	{
		//if c is less than or equal to ASCII value 9 it's either a number or something else that isn't a letter, assume number.
		//otherwise lowercase letter ( | ' '), subtract ASCII value 'a', add 10
		return c <= '9' ? c - '0' : ( c | ' ' ) - 'a' + 10;
	}

	template<typename T, size_t N, size_t... Idx>
	constexpr std::array<T, N + 1> add_to_front( std::array<T, N> arr, T val, std::index_sequence<Idx...> )
	{
		return std::array<T, N + 1> { val, arr[ Idx ]... };
	}

	template<std::uint32_t C, std::uint32_t N,
		const char* Pattern,
		class Enable = void>
		struct ida_mask_helper_t
	{
		using next_mask_helper = ida_mask_helper_t<C + 2, N, Pattern>;
		constexpr static auto length = next_mask_helper::length + 1;

		constexpr static auto pattern = add_to_front<std::uint8_t>( next_mask_helper::pattern, char_to_hex( Pattern[ C ] ) * 16 + char_to_hex( Pattern[ C + 1 ] ), std::make_index_sequence<length - 1>( ) );
		constexpr static auto wildcard = add_to_front( next_mask_helper::wildcard, false, std::make_index_sequence<length - 1>( ) );
	};

	template<
		std::uint32_t C, std::uint32_t N,
		const char* Pattern>
		struct ida_mask_helper_t<C, N, Pattern, std::enable_if_t<C >= N>>
	{
		constexpr static auto length = 1;

		constexpr static std::array<std::uint8_t, 1> pattern{ 0 };
		constexpr static std::array<bool, 1> wildcard{ true };
	};

	template<
		std::uint32_t C, std::uint32_t N,
		const char* Pattern>
		struct ida_mask_helper_t<C, N, Pattern, std::enable_if_t<Pattern[ C ] == '?'>>
	{
		using next_mask_helper = ida_mask_helper_t<C + 1, N, Pattern>;
		constexpr static auto length = next_mask_helper::length + 1;

		constexpr static auto pattern = add_to_front( next_mask_helper::pattern, static_cast< std::uint8_t >( 0 ), std::make_index_sequence<length - 1>( ) );
		constexpr static auto wildcard = add_to_front( next_mask_helper::wildcard, true, std::make_index_sequence<length - 1>( ) );
	};

	template<
		std::uint32_t C, std::uint32_t N,
		const char* Pattern>
		struct ida_mask_helper_t<C, N, Pattern, std::enable_if_t<Pattern[ C ] == ' '>>
	{
		using next_mask_helper = ida_mask_helper_t<C + 1, N, Pattern>;
		constexpr static auto length = next_mask_helper::length;

		constexpr static auto pattern = next_mask_helper::pattern;
		constexpr static auto wildcard = next_mask_helper::wildcard;
	};

	template<std::uint32_t N, const char Pattern[ N ]>
	struct ida_mask_t
	{
		using value = ida_mask_helper_t<0, N - 1, Pattern>;
	};

	template<typename mask>
	constexpr uint8_t* scan_for_pattern_ida( nt::rtl_module_info* module_info )
	{
		if ( !module_info )
			return nullptr;

		const auto base = reinterpret_cast< uint8_t* >( module_info->image_base );
		const auto size = module_info->image_size;

		if ( !base || !size )
			return nullptr;

		auto pattern = mask::pattern;
		auto wildcard = mask::wildcard;
		auto length = mask::length;

		for ( size_t i{ 0 }; i < size - length; i++ )
		{
			auto found = true;
			for ( size_t j{ 0 }; j < length; j++ )
			{
				if ( !wildcard[ j ] && pattern[ j ] != base[ i + j ] )
				{
					found = false;
					break;
				}
			}

			if ( found )
				return base + i;
		}

		return nullptr;
	}

#define CONSTRUCT_IDA_MASK(str) ida_mask_t<sizeof(str), str>::value
#define SCAN_PATTERN_IDA(base, str) scan_for_pattern_ida<CONSTRUCT_IDA_MASK(str)>( base )
}