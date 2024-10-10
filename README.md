# TODO

- [ ] use `mmap`/`munmap`/`mprotect`
- [ ] move shell code in another section
- [ ] work with DYN
- [ ] encryption with xor 42
- [ ] use better encryption algorithm (AES?)
- [ ] huffman encoding
- [ ] remove forbidden functions and use+compile libft

*asm to code*:
```shell
nasm -f bin asm/stub.asm -o /dev/stdout | xxd -p |  sed 's/.\{2\}/&\\x/g' | tr -d '\n' | sed 's/^/\\x/'
```