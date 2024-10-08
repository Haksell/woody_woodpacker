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
    FILE* f = fopen(argv[1], "rb");
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
    Elf64_Phdr* exec_phdr = NULL;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type == PT_LOAD && (phdrs[i].p_flags & PF_X)) {
            exec_phdr = &phdrs[i];
        }
    }
    if (!exec_phdr) panic("No PT_LOAD segment found\n");

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
    exec_phdr->p_filesz += payload_size;
    exec_phdr->p_memsz += payload_size;

    // Calculate where to place the new code
    uint64_t exec_offset = exec_phdr->p_offset + exec_phdr->p_filesz;
    uint64_t exec_vaddr = exec_phdr->p_vaddr + exec_phdr->p_filesz;

    // Update the entry point
    ehdr->e_entry = exec_vaddr;

    // Resize the buffer to accommodate the new code
    buffer = realloc(buffer, filesize + payload_size);
    if (!buffer) panic("Memory allocation failed\n");

    // Zero out the new space
    memset(buffer + filesize, 0, payload_size);

    // Copy the code into the buffer
    memcpy(buffer + exec_offset, payload, payload_size);

    // Write the modified buffer back to the file
    f = fopen("woody", "wb");
    if (!f) panic("Cannot open file for writing\n");

    if (fwrite(buffer, 1, filesize + payload_size, f) != filesize + payload_size)
        panic("Failed to write to file\n");

    fclose(f);
    free(buffer);
    return 0;
}
