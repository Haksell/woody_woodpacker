#ifndef FT_ELF_H
#define FT_ELF_H

#include "nm_errors.h"
#include "../libft/includes/libft.h"
#include <elf.h>

#define NEXT_PHDR(current_phdr, elf_header) \
((Elf64_Phdr *)((char *)(current_phdr) + (elf_header)->e_phentsize))


typedef struct {
    void *buffer;
    Elf64_Ehdr *ehdr;
    size_t file_size;
} t_elf_ctx;

int sum(int a, int b);
void append_segment_to_file_end(t_elf_ctx *ctx, Elf64_Phdr *phdr, size_t guaranteed_size);

#endif //FT_ELF_H


