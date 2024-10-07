#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void panic(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static void error(const char* name) {
    panic("woody_woodpacker: %s: %s\n", name, strerror(errno));
}

static Elf64_Ehdr* read_elf_file(const char* input_file) {
    FILE* f = fopen(input_file, "rb");
    if (!f) error("fopen");

    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = malloc(filesize);
    if (!buffer) {
        fclose(f);
        error("malloc");
    }

    if (fread(buffer, 1, filesize, f) != filesize) {
        fclose(f);
        free(buffer);
        error("fread");
    }

    fclose(f);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        free(buffer);
        panic("File architecture not suported. x86_64 only\n");
    }

    return ehdr;
}

static Elf64_Phdr* find_code_header(Elf64_Ehdr* ehdr) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)ehdr + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X)) {
            return phdr + i;
        }
    }
    free(ehdr);
    panic("No executable segment found\n");
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) panic("Usage: %s <input_binary>\n", argv[0]);
    Elf64_Ehdr* ehdr = read_elf_file(argv[1]);
    Elf64_Phdr* code_phdr = find_code_header(ehdr);
    printf("code_phdr->p_paddr=%zu\n", code_phdr->p_paddr);
    return EXIT_SUCCESS;
}
