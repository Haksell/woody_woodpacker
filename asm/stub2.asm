BITS 64
global main

main:
    jmp get_string

print:
    mov rax, 0x1
    mov rdi, 0x1
    pop rsi
    mov rdx, 0xe
    syscall
    jmp end

get_string:
    call print
    db `....WOODY....\n`

end:
    xor rax, rax
    xor rdi, rdi
    xor rdx, rdx
    xor rsi, rsi

    jmp $-3191
