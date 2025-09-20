#pragma once

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define NC "\x1b[0m"

void kernel_variable_init();
int write_vardata(const char* outfile, uint32_t attributes, uint32_t size, void* buf);
char* kernel_efi_path;
