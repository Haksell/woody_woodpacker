#include <stdio.h>
#include <string.h>

#include "../inc/ft_elf.h"

int sum(int a, int b) {
    ft_printf("%s: sum(%d, %d)\n", NAME2, a, b);
    return a + b;
}

#include <elf.h>

#include <stdlib.h>
#include <stdint.h>

/*

static void move_section_headers(t_elf_ctx *ctx, size_t offset) {
    size_t new_file_size = ctx->file_size + offset;

    void *new_buffer = calloc(1, new_file_size);
    if (!new_buffer) {
        fprintf(stderr, "Failed to allocate memory\n");
        return;
    }

    size_t sh_table_end_offset = ctx->ehdr->e_shoff;

    memcpy(new_buffer, ctx->buffer, sh_table_end_offset);

    memcpy((uint8_t *)new_buffer + sh_table_end_offset + offset,
           (uint8_t *)ctx->buffer + sh_table_end_offset,
           ctx->file_size - sh_table_end_offset);

    Elf64_Ehdr *new_ehdr = (Elf64_Ehdr *)new_buffer;

    new_ehdr->e_shoff += offset;

    free(ctx->buffer);
    ctx->buffer = new_buffer;
    ctx->ehdr = new_ehdr;
    ctx->file_size = new_file_size;
}

static Elf64_Phdr* find_executable_program_header(Elf64_Ehdr* ehdr) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)ehdr + ehdr->e_phoff);  // Start of program headers
    Elf64_Phdr* res = NULL;

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X)) {
            if (!res) {
                res = &phdr[i];
            } else {
                printf("Found more executable program headers at index %u\n", i);
            }
        }
    }

    return res;
}

Elf64_Phdr* get_last_phdr(Elf64_Ehdr* ehdr) {
    if (ehdr == NULL) {
        return NULL;
    }

    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)ehdr + ehdr->e_phoff);

    int phnum = ehdr->e_phnum;

    return &phdr[phnum - 1];
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>


void append_segment_to_file_end(t_elf_ctx *ctx, Elf64_Phdr *phdr, size_t guaranteed_size) {
    // Calculate new segment sizes
    size_t new_segment_filesz = phdr->p_filesz + guaranteed_size;
    size_t new_segment_memsz = phdr->p_memsz + guaranteed_size;

    // Calculate new file size
    size_t new_file_size = ctx->file_size + guaranteed_size;

    // Allocate new buffer
    void *new_buffer = calloc(1, new_file_size);
    if (!new_buffer) {
        fprintf(stderr, "Failed to allocate memory\n");
        return;
    }

    // Copy the original file into the new buffer
    memcpy(new_buffer, ctx->buffer, ctx->file_size);

    // Get pointers to the ELF header and program headers in the new buffer
    Elf64_Ehdr *new_ehdr = (Elf64_Ehdr *)new_buffer;
    Elf64_Phdr *new_phdrs = (Elf64_Phdr *)((char *)new_buffer + new_ehdr->e_phoff);

    // Calculate the index of the program header to modify
    size_t phdr_index = phdr - (Elf64_Phdr *)((char *)ctx->buffer + ctx->ehdr->e_phoff);
    if (phdr_index >= new_ehdr->e_phnum) {
        fprintf(stderr, "Invalid program header index\n");
        free(new_buffer);
        return;
    }

    // Get the program header in the new buffer
    Elf64_Phdr *new_phdr = &new_phdrs[phdr_index];

    // Store old offsets and addresses
    size_t segment_old_offset = new_phdr->p_offset;
    //Elf64_Addr segment_old_vaddr = new_phdr->p_vaddr;

    // Calculate new offset aligned to p_align
    size_t new_offset = ctx->file_size;
    if (new_phdr->p_align > 0) {
        new_offset = (new_offset + new_phdr->p_align - 1) & ~(new_phdr->p_align - 1);
    }

    // Adjust virtual address based on new offset
    // Maintain the relationship between p_vaddr and p_offset
    Elf64_Addr delta_offset = new_offset - new_phdr->p_offset;
    Elf64_Addr new_vaddr = new_phdr->p_vaddr + delta_offset;

    // Ensure the new virtual address is aligned
    if (new_phdr->p_align > 0) {
        new_vaddr = (new_vaddr + new_phdr->p_align - 1) & ~(new_phdr->p_align - 1);
    }

    // Update the program header in the new buffer
    new_phdr->p_offset = new_offset;
    new_phdr->p_vaddr = new_vaddr;
    new_phdr->p_paddr = new_vaddr;
    new_phdr->p_filesz = new_segment_filesz;
    new_phdr->p_memsz = new_segment_memsz;

    new_ehdr->e_entry = new_vaddr + 0x40;

    // Copy the segment data to the new location in the new buffer
    memcpy((char *)new_buffer + new_offset,
           (char *)ctx->buffer + segment_old_offset,
           phdr->p_filesz);

    // Zero-fill the additional space (already zeroed by calloc)
    // You can inject your code into this space if needed

    // Update the context
    free(ctx->buffer);
    ctx->buffer = new_buffer;
    ctx->file_size = new_file_size;
    ctx->ehdr = new_ehdr;
}






    /*
void add_program_header(t_elf_ctx *ctx) {
    //TODO: don't hardcode here. Some articles says that section headers are optional. test it.
    size_t offset = 4096;

    move_section_headers(ctx, offset);

    Elf64_Phdr *executable_phdr = find_executable_program_header(ctx->ehdr);

    Elf64_Phdr *next_after_executable = NEXT_PHDR(executable_phdr, ctx->ehdr);

    printf("Next after executable header: offset %lx, virtaddr %lx\n", next_after_executable->p_offset, next_after_executable->p_vaddr);

    Elf64_Phdr *last_program_header = get_last_phdr(ctx->ehdr);

    printf("Last program header: offset %lx, virtaddr %lx\n", last_program_header->p_offset, last_program_header->p_vaddr);
}

    */
/*

void add_program_header2(t_elf_ctx *ctx , Elf64_Phdr *new_phdr ) {



    if (new_ehdr->e_shoff >= ph_table_end_offset) {
        new_ehdr->e_shoff += phdr_size;
    }

    Elf64_Addr insertion_vaddr = 0;
    Elf64_Phdr *program_headers = (Elf64_Phdr *)((uint8_t *)new_buffer + new_ehdr->e_phoff);
    for (int i = 0; i < new_ehdr->e_phnum; i++) {
        if (program_headers[i].p_type == PT_LOAD) {
            Elf64_Off segment_offset = program_headers[i].p_offset;
            Elf64_Xword segment_filesz = program_headers[i].p_filesz;
            if (ph_table_end_offset >= segment_offset &&
                ph_table_end_offset <= segment_offset + segment_filesz) {
                insertion_vaddr = program_headers[i].p_vaddr + (ph_table_end_offset - segment_offset);
                break;
            }
        }
    }
    if (insertion_vaddr == 0) {
        fprintf(stderr, "Failed to find insertion virtual address\n");
        free(new_buffer);
        return;
    }

    if (new_ehdr->e_entry >= insertion_vaddr) {
        new_ehdr->e_entry += phdr_size;
    }

    for (int i = 0; i < new_ehdr->e_phnum; i++) {
        if (program_headers[i].p_offset >= ph_table_end_offset) {
            program_headers[i].p_offset += phdr_size;
        }
        if (program_headers[i].p_vaddr >= insertion_vaddr) {
            program_headers[i].p_vaddr += phdr_size;
            program_headers[i].p_paddr += phdr_size;
        }
        if (program_headers[i].p_type == PT_LOAD) {
            Elf64_Off segment_end = program_headers[i].p_offset + program_headers[i].p_filesz;
            // If the segment includes data after the insertion point, increase its size
            if (segment_end > ph_table_end_offset) {
                program_headers[i].p_filesz += phdr_size;
                program_headers[i].p_memsz += phdr_size;
            }
        }
    }

    Elf64_Shdr *section_headers = (Elf64_Shdr *)((uint8_t *)new_buffer + new_ehdr->e_shoff);
    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        if (section_headers[i].sh_offset >= ph_table_end_offset) {
            section_headers[i].sh_offset += phdr_size;
        }
        if (section_headers[i].sh_addr >= insertion_vaddr) {
            section_headers[i].sh_addr += phdr_size;
        }
    }

    Elf64_Shdr *sh_strtab = &section_headers[new_ehdr->e_shstrndx];
    const char *sh_strs = (const char *)new_buffer + sh_strtab->sh_offset;

    Elf64_Phdr *dynamic_phdr = NULL;
    for (int i = 0; i < new_ehdr->e_phnum; i++) {
        if (program_headers[i].p_type == PT_DYNAMIC) {
            dynamic_phdr = &program_headers[i];
            break;
        }
    }
    if (dynamic_phdr) {
        Elf64_Dyn *dyn_entries = (Elf64_Dyn *)((uint8_t *)new_buffer + dynamic_phdr->p_offset);
        size_t num_dyn_entries = dynamic_phdr->p_filesz / sizeof(Elf64_Dyn);

        for (size_t i = 0; i < num_dyn_entries; i++) {
            uint64_t tag = dyn_entries[i].d_tag;
            switch (tag) {
                case DT_PLTGOT:
                case DT_HASH:
                case DT_GNU_HASH:
                case DT_STRTAB:
                case DT_SYMTAB:
                case DT_RELA:
                case DT_REL:
                case DT_JMPREL:
                case DT_INIT:
                case DT_FINI:
                case DT_INIT_ARRAY:
                case DT_FINI_ARRAY:
                case DT_VERNEED:
                case DT_VERSYM:
                    if (dyn_entries[i].d_un.d_ptr >= insertion_vaddr) {
                        dyn_entries[i].d_un.d_ptr += phdr_size;
                    }
                    break;
                case DT_PLTRELSZ:
                case DT_RELASZ:
                case DT_RELSZ:
                case DT_INIT_ARRAYSZ:
                case DT_FINI_ARRAYSZ:
                case DT_STRSZ:
                case DT_SYMENT:
                case DT_RELAENT:
                case DT_RELENT:
                    break;
                default:
                    break;
            }
        }
    }

    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        const char *section_name = sh_strs + section_headers[i].sh_name;

        if (strcmp(section_name, ".rela.dyn") == 0 || strcmp(section_name, ".rela.plt") == 0) {
            Elf64_Rela *rel_entries = (Elf64_Rela *)((uint8_t *)new_buffer + section_headers[i].sh_offset);
            size_t num_entries = section_headers[i].sh_size / sizeof(Elf64_Rela);

            for (size_t j = 0; j < num_entries; j++) {
                if (rel_entries[j].r_offset >= insertion_vaddr) {
                    rel_entries[j].r_offset += phdr_size;
                }
            }
        }
    }

    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        const char *section_name = sh_strs + section_headers[i].sh_name;
        if (strcmp(section_name, ".init_array") == 0 || strcmp(section_name, ".fini_array") == 0) {
            Elf64_Shdr *shdr = &section_headers[i];
            uint64_t *array = (uint64_t *)((uint8_t *)new_buffer + shdr->sh_offset);
            size_t num_entries = shdr->sh_size / sizeof(uint64_t);

            for (size_t j = 0; j < num_entries; j++) {
                if (array[j] >= insertion_vaddr) {
                    array[j] += phdr_size;
                }
            }
        }
    }

    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        const char *section_name = sh_strs + section_headers[i].sh_name;

        if (strcmp(section_name, ".got") == 0 || strcmp(section_name, ".got.plt") == 0) {
            Elf64_Shdr *shdr = &section_headers[i];
            uint64_t *got_entries = (uint64_t *)((uint8_t *)new_buffer + shdr->sh_offset);
            size_t num_entries = shdr->sh_size / sizeof(uint64_t);

            for (size_t j = 0; j < num_entries; j++) {
                if (got_entries[j] >= insertion_vaddr) {
                    got_entries[j] += phdr_size;
                }
            }
        }
    }

    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        const char *section_name = sh_strs + section_headers[i].sh_name;

        if (strcmp(section_name, ".eh_frame") == 0 || strcmp(section_name, ".eh_frame_hdr") == 0) {
            Elf64_Shdr *shdr = &section_headers[i];
            if (shdr->sh_offset >= ph_table_end_offset) {
                shdr->sh_offset += phdr_size;
            }
            if (shdr->sh_addr >= insertion_vaddr) {
                shdr->sh_addr += phdr_size;
            }

        }
    }

    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        const char *section_name = sh_strs + section_headers[i].sh_name;

        if (strcmp(section_name, ".symtab") == 0) {
            Elf64_Shdr *symtab_shdr = &section_headers[i];
            Elf64_Sym *symtab = (Elf64_Sym *)((uint8_t *)new_buffer + symtab_shdr->sh_offset);
            size_t num_symbols = symtab_shdr->sh_size / sizeof(Elf64_Sym);

            for (size_t j = 0; j < num_symbols; j++) {
                if (symtab[j].st_value >= insertion_vaddr && symtab[j].st_value != 0) {
                    symtab[j].st_value += phdr_size;
                }
            }
        }
    }


}


*/




/*
static void add_program_header1(t_elf_ctx *ctx ,Elf64_Phdr *new_phdr) {
    size_t phdr_size = sizeof(Elf64_Phdr);
    size_t new_file_size = ctx->file_size + phdr_size;

    void *new_buffer = calloc(new_file_size, 1);
    if (!new_buffer) {
        ft_printf("malloc\n");
        return;
    }

    size_t ph_table_end_offset = ctx->ehdr->e_phoff + ctx->ehdr->e_phnum * phdr_size;

    memcpy(new_buffer, ctx->buffer, ph_table_end_offset);

    //memcpy((uint8_t *)new_buffer + ph_table_end_offset, new_phdr, phdr_size);

    memcpy(new_buffer + ph_table_end_offset + phdr_size,
           ctx->buffer + ph_table_end_offset,
           ctx->file_size - ph_table_end_offset);

    Elf64_Ehdr *new_ehdr = new_buffer;
    //new_ehdr->e_phnum += 1;

    if (new_ehdr->e_shoff >= ph_table_end_offset) {
        new_ehdr->e_shoff += phdr_size;
    }

    Elf64_Shdr *section_headers = (Elf64_Shdr *)((uint8_t *)new_buffer + new_ehdr->e_shoff);
    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        if (section_headers[i].sh_offset >= ph_table_end_offset) {
            section_headers[i].sh_offset += phdr_size;
            section_headers[i].sh_addr += phdr_size;
        }
        //if (section_headers[i].sh_addr >= new_phdr->p_vaddr) {
        //}
    }

    Elf64_Phdr *program_headers = (Elf64_Phdr *)((uint8_t *)new_buffer + new_ehdr->e_phoff);
    for (int i = 0; i < new_ehdr->e_phnum; i++) {
        if (program_headers[i].p_offset >= ph_table_end_offset) {
            program_headers[i].p_offset += phdr_size;
            program_headers[i].p_vaddr += phdr_size;
            program_headers[i].p_paddr += phdr_size;
        }
        //if (program_headers[i].p_vaddr >= new_phdr->p_vaddr) {
        //}
    }

    //program_headers[new_ehdr->e_phnum - 1] = *new_phdr;

    Elf64_Shdr *sh_table = section_headers;

    Elf64_Shdr *sh_strtab = &sh_table[new_ehdr->e_shstrndx];
    const char *sh_strs = (const char *)new_buffer + sh_strtab->sh_offset;

    for (int i = 0; i < new_ehdr->e_shnum; i++) {
        const char *section_name = sh_strs + sh_table[i].sh_name;

        if (strcmp(section_name, ".rela.dyn") == 0 || strcmp(section_name, ".rela.plt") == 0) {
            Elf64_Rela *rel_entries = (Elf64_Rela *)((uint8_t *)new_buffer + sh_table[i].sh_offset);
            size_t num_entries = sh_table[i].sh_size / sizeof(Elf64_Rela);

            for (size_t j = 0; j < num_entries; j++) {
                if (rel_entries[j].r_offset >= ph_table_end_offset) {
                    rel_entries[j].r_offset += phdr_size;
                }
            }
        }
    }

    Elf64_Phdr *dynamic_phdr = NULL;
    for (int i = 0; i < new_ehdr->e_phnum; i++) {
        if (program_headers[i].p_type == PT_DYNAMIC) {
            dynamic_phdr = &program_headers[i];
            break;
        }
    }

    if (dynamic_phdr) {
        Elf64_Dyn *dyn_entries = (Elf64_Dyn *)((uint8_t *)new_buffer + dynamic_phdr->p_offset);
        size_t num_dyn_entries = dynamic_phdr->p_filesz / sizeof(Elf64_Dyn);

        for (size_t i = 0; i < num_dyn_entries; i++) {
            uint64_t tag = dyn_entries[i].d_tag;
            if (tag == DT_STRTAB || tag == DT_SYMTAB || tag == DT_GNU_HASH ||
                tag == DT_RELA || tag == DT_INIT || tag == DT_FINI ||
                tag == DT_INIT_ARRAY || tag == DT_FINI_ARRAY || tag == DT_JMPREL ||
                tag == DT_VERNEED || tag == DT_VERSYM) {

                if (dyn_entries[i].d_un.d_ptr >= ph_table_end_offset) {
                    dyn_entries[i].d_un.d_ptr += phdr_size;
                }

                } else if (tag == DT_RELASZ || tag == DT_STRSZ || tag == DT_PLTRELSZ ||
                           tag == DT_VERNEEDNUM || tag == DT_RELACOUNT) {
                    // These entries contain sizes, which may not need adjustment
                    // unless the size of the sections they refer to has changed.
                    // In this case, sizes remain the same, so we may not need to adjust them.
                    // This block is included for completeness.
                    continue;
                           } else {
                               // Handle other tags if necessary
                               continue;
                           }
        }
    }

    new_ehdr->e_entry += phdr_size;

    free(ctx->buffer);
    ctx->buffer = new_buffer;
    ctx->ehdr = new_ehdr;
    ctx->file_size = new_file_size;
}


*/