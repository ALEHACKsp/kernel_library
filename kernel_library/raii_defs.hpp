#pragma once
#include <ntifs.h>
#include <memory>

namespace impl
{
	namespace smart
	{
		struct object_deleter
		{
			void operator( )( void* arg ) const
			{
				if ( arg )
					ObfDereferenceObject( arg );
			}
		};

		template <typename T>
		using object = std::unique_ptr<std::remove_pointer_t<T>, object_deleter>;

		struct alloc_deleter
		{
			void operator( )( void* arg ) const
			{
				if ( arg )
					ExFreePoolWithTag( arg, 0 );
			}
		};

		using alloc = std::unique_ptr<void, alloc_deleter>;
	}
}