#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

size_t read_elf_file(const char* input_file, uint8_t** buffer) {
    FILE* f = fopen(input_file, "rb");
    if (!f) error("fopen");

    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    *buffer = malloc(filesize);
    if (!*buffer) {
        fclose(f);
        error("malloc");
    }

    if (fread(*buffer, 1, filesize, f) != filesize) {
        fclose(f);
        free(*buffer);
        error("fread");
    }

    fclose(f);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)*buffer;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
        ehdr->e_ident[EI_CLASS] != ELFCLASS64 || ehdr->e_machine != EM_X86_64) {
        free(*buffer);
        panic("File architecture not suported. x86_64 only\n");
    }

    if (ehdr->e_type != ET_EXEC) {
        free(*buffer);
        panic("The file is not an executable.\n");
    }

    return filesize;
}

static Elf64_Phdr* find_code_header(Elf64_Ehdr* ehdr) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)ehdr + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X)) {
            return phdr + i;
        }
    }
    free(ehdr);
    panic("No executable segment found.\n");
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) panic("Usage: %s <input_binary>\n", argv[0]);

    uint8_t* buffer;
    size_t filesize = read_elf_file(argv[1], &buffer);
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    Elf64_Phdr* code_phdr = find_code_header(ehdr);

    uint8_t payload
        [] = "\x48\xc7\xc0\x01\x00\x00\x00" // mov rax, 1
             "\x48\xc7\xc7\x01\x00\x00\x00" // mov rdx, 1
             "\x48\x8d\x35\x16\x00\x00\x00" // lea rsi, [rip+22] (jump to after HACKED)
             "\x48\xc7\xc2\x07\x00\x00\x00" // mov rdx, 7
             "\x0f\x05" // syscall
             "\x48\xc7\xc2\x00\x00\x00\x00" // mov rdx, 0
             "\xff\x25\x07\x00\x00\x00" // jmp [rip+7]
             "HACKED\n"
             "\x00\x00\x00\x00\x00\x00\x00" // placeholder for jmp address
        ;
    size_t payload_size = sizeof(payload);
    memcpy(&payload[payload_size - sizeof(size_t)], &ehdr->e_entry, sizeof(size_t));

    // Adjust segment sizes
    code_phdr->p_filesz += payload_size;
    code_phdr->p_memsz += payload_size;

    // Calculate where to place the new code
    uint64_t exec_offset = code_phdr->p_offset + code_phdr->p_filesz;
    uint64_t exec_vaddr = code_phdr->p_vaddr + code_phdr->p_filesz;

    // Update the entry point
    ehdr->e_entry = exec_vaddr;

    // Resize the buffer to accommodate the new code
    memcpy(buffer + exec_offset, payload, payload_size);

    // Write the modified buffer back to the file
    FILE* f = fopen("woody", "wb");
    if (!f) error("fopen");
    if (fwrite(buffer, 1, filesize + payload_size, f) != filesize + payload_size)
        error("fwrite");

    fclose(f);
    free(buffer);
    return 0;
}
