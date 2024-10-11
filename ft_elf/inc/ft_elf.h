#ifndef FT_ELF_H
#define FT_ELF_H

#include "nm_errors.h"
#include "../libft/includes/libft.h"
#include <elf.h>

typedef struct {
    void *buffer;
    Elf64_Ehdr *ehdr;
    size_t file_size;
} t_elf_ctx;

int sum(int a, int b);
void add_program_header(t_elf_ctx *ctx/*, Elf64_Phdr *phdr*/);

#endif //FT_ELF_H


