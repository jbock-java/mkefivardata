#include <efi.h>

int str_to_guid(const char *str, EFI_GUID *guid)
{
	int args;
	args = sscanf(str,
		      "%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
		      &guid->Data1, &guid->Data2, &guid->Data3,
		      guid->Data4, guid->Data4 + 1, guid->Data4 + 2,
		      guid->Data4 + 3, guid->Data4 + 4, guid->Data4 + 5,
		      guid->Data4 + 6, guid->Data4 + 7);

	return args != 11;
}
