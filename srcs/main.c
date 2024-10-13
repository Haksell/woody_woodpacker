#include "../includes/woody.h"

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

    if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
        free(*buffer);
        panic("The file is not an executable.\n");
    }

    if (ehdr->e_type == ET_DYN)
        fprintf(stderr, "Warning: woody_woodpacker does not handle DYN files yet.\n");

    return filesize;
}

static Elf64_Phdr* find_code_header(Elf64_Ehdr* ehdr) {
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)ehdr + ehdr->e_phoff);
    // }
    for (int i = 0; i < ehdr->e_phnum; i++) {
        printf("@@ %d\n", i);
        if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & PF_X)) {
            return phdr + i;
        }
    }
    free(ehdr);
    panic("No executable segment found.\n");
    return NULL;
}

/*
static void check_text_section_has_enough_zeros(Elf64_Ehdr* ehdr, uint8_t* buffer, size_t stub_size) {
    Elf64_Phdr* code_phdr = find_code_header(ehdr);

    uint8_t* zero_mem = buffer + code_phdr->p_offset + code_phdr->p_filesz;

    for (size_t i = 0; i < stub_size; i++) {
        if (zero_mem[i] != 0) {
            printf("i = %lu\n",i);
            //panic("There is not enough space to write the stub after the actual main code.");
        }
    }
}

static void inject_stub(uint8_t* buffer) {
returnentry: 401040
➜  woody_woodpacker git:(shit_but_not_completely) ✗ bingrep woody
ELF EXEC X86_64-little-endian @ 0x401040:

e_phoff: 0x40 e_shoff: 0x3158 e_flags: 0x0 e_ehsize: 64 e_phentsize: 56 e_phnum: 13 e_shentsize: 64 e_shnum: 28 e_shstrndx: 27

ProgramHeaders(13):
  Idx   Type              Flags   Offset    Vaddr       Paddr       Filesz   Memsz    Align
  0     PT_PHDR           R       0x40      0x400040    0x400040    0x2d8    0x2d8    0x8
  1     PT_INTERP         R       0x318     0x400318    0x400318    0x1c     0x1c     0x1
  2     PT_LOAD           R       0x0       0x400000    0x400000    0x5a8    0x5a8    0x1000
  3     PT_LOAD           R+X     0x3858    0x401000    0x401000    0x14d    0x14d    0x1000
  4     PT_LOAD           R       0x2000    0x402000    0x402000    0xcc     0xcc     0x1000
  5     PT_LOAD           RW      0x2de8    0x403de8    0x403de8    0x230    0x238    0x1000
  6     PT_DYNAMIC        RW      0x2df8    0x403df8    0x403df8    0x1d0    0x1d0    0x8
  7     PT_NOTE           R       0x338     0x400338    0x400338    0x40     0x40     0x8
  8     PT_NOTE           R       0x378     0x400378    0x400378    0x44     0x44     0x4
  9     PT_GNU_PROPERTY   R       0x338     0x400338    0x400338    0x40     0x40     0x8
  10    PT_GNU_EH_FRAME   R       0x2014    0x402014    0x402014    0x2c     0x2c     0x4
  11    PT_GNU_STACK      RW      0x0       0x0         0x0         0x0      0x0      0x10
  12    PT_GNU_RELRO      R       0x2de8    0x403de8    0x403de8    0x218    0x218    0x1  ;
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    Elf64_Phdr* code_phdr = find_code_header(ehdr);

    uint8_t stub
        [] = "\xeb\x14\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x5e\xba\x0e\x00\x00\x00"
             "\x0f\x05\xeb\x13\xe8\xe7\xff\xff\xff\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59"
             "\x2e\x2e\x2e\x2e\x0a\x48\x31\xc0\x48\x31\xff\x48\x31\xd2\x48\x31\xf6\xff"
             "\x25\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff";
    size_t stub_size = sizeof(stub);
    check_text_section_has_enough_zeros(ehdr, buffer, stub_size);
    memcpy(&stub[stub_size - sizeof(size_t)], &ehdr->e_entry, sizeof(size_t));

    code_phdr->p_filesz += stub_size;
    code_phdr->p_memsz += stub_size;

    ehdr->e_entry = code_phdr->p_vaddr + code_phdr->p_filesz;
    memcpy(buffer + code_phdr->p_offset + code_phdr->p_filesz, stub, stub_size);
}
*/

static void pack(uint8_t* buffer, size_t filesize) {
    FILE* f = fopen("woody", "wb");
    if (!f) {
        free(buffer);
        error("fopen");
    }
    if (fwrite(buffer, 1, filesize, f) != filesize) {
        fclose(f);
        free(buffer);
        error("fwrite");
    }
    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc != 2) panic("Usage: %s <input_binary>\n", argv[0]);
    uint8_t* buffer;
    size_t filesize = read_elf_file(argv[1], &buffer);
    t_elf_ctx elf_ctx = {
.buffer = buffer,
        .ehdr = (Elf64_Ehdr*)buffer,
        filesize
    };
    //add_program_header(&elf_ctx /*,find_code_header((Elf64_Ehdr*)buffer)*/);
    append_segment_to_file_end(&elf_ctx, find_code_header(elf_ctx.ehdr), 4096);
    //inject_stub(elf_ctx.buffer);
    pack(elf_ctx.buffer, elf_ctx.file_size);
    free(elf_ctx.buffer);
    return EXIT_SUCCESS;
}
