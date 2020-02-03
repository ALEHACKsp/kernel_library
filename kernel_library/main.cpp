#include "export_getter.hpp"
#include "process_getter.hpp"
#include "general_util.hpp"

NTSTATUS DriverEntry( )
{
	INVOKE_FN( DbgPrintEx )( 77, 0, "[!] initialized\n[!] csrss.exe: 0x%p\n[!] %wZ\n", impl::nt_find_process( "csrss.exe" ), &impl::make_unicode( L"test string" ) );

	return STATUS_SUCCESS;
}