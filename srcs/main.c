#include <elf.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void panic(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc != 2) panic("Usage: %s <input_binary>\n", argv[0]);

    // Open the ELF file
    FILE* f = fopen(argv[1], "rb+");
    if (!f) panic("Cannot open file %s\n", argv[1]);

    // Get the file size
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Read the entire file into memory
    char* buffer = malloc(filesize);
    if (!buffer) panic("Memory allocation failed\n");

    if (fread(buffer, 1, filesize, f) != filesize) panic("Failed to read file\n");
    fclose(f);

    // Get the ELF header
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;

    // Verify it's an ELF64 file
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) panic("Not an ELF file\n");
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) panic("Not an ELF64 file\n");

    // Find the last PT_LOAD segment
    Elf64_Phdr* phdrs = (Elf64_Phdr*)(buffer + ehdr->e_phoff);
    Elf64_Phdr* last_load = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD && (phdrs[i].p_flags & PF_X)) {
            last_load = &phdrs[i];
        }
    }
    if (!last_load) panic("No PT_LOAD segment found\n");

    // Calculate where to place the new code
    uint64_t code_offset = last_load->p_offset + last_load->p_filesz;
    uint64_t code_vaddr = last_load->p_vaddr + last_load->p_filesz;

    // Define the injected code
    uint8_t code[] = {
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        0x90,
        // mov rax, 1
        0x48,
        0xc7,
        0xc0,
        0x01,
        0x00,
        0x00,
        0x00,
        // mov rdi, 1
        0x48,
        0xc7,
        0xc7,
        0x01,
        0x00,
        0x00,
        0x00,
        // lea rsi, [rip+22] (jump to after HACKED)
        0x48,
        0x8d,
        0x35,
        0x16,
        0x00,
        0x00,
        0x00,
        // mov rdx, 7
        0x48,
        0xc7,
        0xc2,
        0x07,
        0x00,
        0x00,
        0x00,
        // syscall
        0x0f,
        0x05,
        // mov rdx, 0
        0x48,
        0xc7,
        0xc2,
        0x00,
        0x00,
        0x00,
        0x00,
        // jmp [rip+7]
        0xff,
        0x25,
        0x07,
        0x00,
        0x00,
        0x00,
        // "HACKED\n"
        'H',
        'A',
        'C',
        'K',
        'E',
        'D',
        '\n',
        // Placeholder for original entry point (8 bytes)
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };

    size_t code_size = sizeof(code);

    // Adjust the code to include the original entry point
    memcpy(&code[code_size - sizeof(size_t)], &ehdr->e_entry, sizeof(size_t));

    // Adjust segment sizes
    last_load->p_filesz += code_size;
    last_load->p_memsz += code_size;

    // Update the entry point
    ehdr->e_entry = code_vaddr;

    // Resize the buffer to accommodate the new code
    buffer = realloc(buffer, filesize + code_size);
    if (!buffer) panic("Memory allocation failed\n");

    // Zero out the new space
    memset(buffer + filesize, 0, code_size);

    // Copy the code into the buffer
    memcpy(buffer + code_offset, code, code_size);

    // Write the modified buffer back to the file
    f = fopen("woody", "wb");
    if (!f) panic("Cannot open file for writing\n");

    if (fwrite(buffer, 1, filesize + code_size, f) != filesize + code_size)
        panic("Failed to write to file\n");

    fclose(f);
    free(buffer);
    return 0;
}
