#include "../includes/woody.h"

#define JMP_SIZE 5
#define REL_JMP_OPCODE 0xE9

//TODO: add mechanism to check if it is already infected

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

    return filesize;
}

static Elf64_Phdr* find_exec_phdr(Elf64_Ehdr* ehdr) {
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

//TODO: test with strip (with additional flag to delete all section headers)
static uint64_t get_text_section_offset(uint8_t* buffer) {
    Elf64_Ehdr* elf_header = (Elf64_Ehdr*) buffer;

    Elf64_Shdr* section_headers = (Elf64_Shdr*) (buffer + elf_header->e_shoff);
    uint16_t section_header_count = elf_header->e_shnum;

    Elf64_Shdr* sh_strtab = &section_headers[elf_header->e_shstrndx];
    const char* section_str_tab = (const char*) (buffer + sh_strtab->sh_offset);

    for (int i = 0; i < section_header_count; i++) {
        const char* section_name = &section_str_tab[section_headers[i].sh_name];

        if (strcmp(section_name, ".text") == 0) {
            return section_headers[i].sh_offset;
        }
    }

    free(buffer);
    panic("No .text section was found, can't perform relative jump");
    return 0;
}

static void check_text_section_has_enough_zeros(
    Elf64_Ehdr* ehdr,
    uint8_t* buffer,
    size_t stub_size
) {
    Elf64_Phdr* code_phdr = find_exec_phdr(ehdr);

    uint8_t* zero_mem = buffer + code_phdr->p_offset + code_phdr->p_filesz;

    for (size_t i = 0; i < stub_size; i++) {
        if (zero_mem[i] != 0) {
            panic(
                "There is not enough space to write the stub after the actual main "
                "code."
            );
        }
    }
}

//TODO: big endian?
void generate_jmp(int32_t offset, uint8_t *jmp_instruction) {
    offset -= JMP_SIZE;

    jmp_instruction[0] = REL_JMP_OPCODE;

    jmp_instruction[1] = offset & 0xFF;
    jmp_instruction[2] = (offset >> 8) & 0xFF;
    jmp_instruction[3] = (offset >> 16) & 0xFF;
    jmp_instruction[4] = (offset >> 24) & 0xFF;
}

static void inject_stub_dyn(uint8_t* buffer) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    Elf64_Phdr* code_phdr = find_exec_phdr(ehdr);

    uint8_t stub
        [] =
        "\xeb\x14\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x5e\xba\x0e\x00\x00\x00\x0f"
        "\x05\xeb\x13\xe8\xe7\xff\xff\xff\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e"
        "\x2e\x2e\x0a\x48\x31\xc0\x48\x31\xff\x48\x31\xd2\x48\x31\xf6\x00\x00\x00\x00\x00";
    size_t stub_size = sizeof(stub);

    check_text_section_has_enough_zeros(ehdr, buffer, stub_size);

    int32_t text_section_rel_offset = (stub_size + code_phdr->p_filesz - (get_text_section_offset(buffer) - code_phdr->p_offset) + 4) * -1;

    uint8_t jmp_instruction[5];
    generate_jmp(text_section_rel_offset, jmp_instruction);

    memcpy(&stub[stub_size - 6], jmp_instruction, 5);

    // code_phdr->p_filesz += stub_size;
    // code_phdr->p_memsz += stub_size;

    ehdr->e_entry = code_phdr->p_vaddr + code_phdr->p_filesz;
    memcpy(buffer + code_phdr->p_offset + code_phdr->p_filesz, stub, stub_size);
}

static void inject_stub_exec(uint8_t* buffer) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buffer;
    Elf64_Phdr* code_phdr = find_exec_phdr(ehdr);

    uint8_t stub
        [] = "\xeb\x14\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x5e\xba\x0e\x00\x00\x00"
             "\x0f\x05\xeb\x13\xe8\xe7\xff\xff\xff\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59"
             "\x2e\x2e\x2e\x2e\x0a\x48\x31\xc0\x48\x31\xff\x48\x31\xd2\x48\x31\xf6\xff"
             "\x25\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff";
    size_t stub_size = sizeof(stub);
    check_text_section_has_enough_zeros(ehdr, buffer, stub_size);
    memcpy(&stub[stub_size - sizeof(size_t)], &ehdr->e_entry, sizeof(size_t));

    // code_phdr->p_filesz += stub_size;
    // code_phdr->p_memsz += stub_size;

    ehdr->e_entry = code_phdr->p_vaddr + code_phdr->p_filesz;
    memcpy(buffer + code_phdr->p_offset + code_phdr->p_filesz, stub, stub_size);
}

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
    if (((Elf64_Ehdr*) buffer)->e_type == ET_EXEC) {
        inject_stub_exec(buffer);
    } else {
        inject_stub_dyn(buffer);
    }
    pack(buffer, filesize);
    free(buffer);
    return EXIT_SUCCESS;
}
