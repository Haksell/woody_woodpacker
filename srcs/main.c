#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void panic_syscall(const char* name) {
    perror(name);
    exit(EXIT_FAILURE);
}

static Elf64_Ehdr* read_elf_file(const char* input_file) {
    FILE* f = fopen(input_file, "rb");
    if (!f) panic_syscall("fopen");

    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = malloc(filesize);
    if (!buffer) {
        fclose(f);
        panic_syscall("malloc");
    }

    if (fread(buffer, 1, filesize, f) != filesize) {
        fclose(f);
        free(buffer);
        panic_syscall("fread");
    }

    fclose(f);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "File architecture not suported. x86_64 only\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    return ehdr;
}

static Elf64_Phdr* find_code_header(Elf64_Ehdr* ehdr) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)ehdr + ehdr->e_phoff);
    Elf64_Phdr* code_phdr = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X)) {
            code_phdr = &phdr[i];
            return code_phdr;
        }
    }
    fprintf(stderr, "No executable segment found\n");
    free(ehdr);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_binary>\n", argv[0]);
        return EXIT_FAILURE;
    }
    Elf64_Ehdr* ehdr = read_elf_file(argv[1]);
    Elf64_Phdr* code_phdr = find_code_header(ehdr);
    printf("code_phdr->p_paddr=%zu\n", code_phdr->p_paddr);
    return EXIT_SUCCESS;
}
