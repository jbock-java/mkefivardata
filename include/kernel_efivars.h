#ifndef KERNEL_EFIVARS_H
#define KERNEL_EFIVARS_H

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define NC "\x1b[0m"

void kernel_variable_init();
int write_vardata(const char* outfile, uint32_t attributes, uint32_t size, void* buf);
extern char* kernel_efi_path;

#endif /* KERNEL_EFIVARS_H */
