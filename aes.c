#include <assert.h> // TODO: proper errors
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t* arrdup(uint8_t* arr, size_t size) {
    uint8_t* out = malloc(size);
    memcpy(out, arr, size);
    return out;
}

void feistel(uint8_t* plaintext, size_t plaintext_size, size_t num_rounds) {
    assert(plaintext_size % 2 == 0);
    size_t half = plaintext_size / 2;
    uint8_t* left = arrdup(plaintext, half);
    uint8_t* right = arrdup(plaintext + half, half);
    for (size_t i = 0; i <= num_rounds; ++i) {
        uint8_t* right_copy = arrdup(right, half);
        feistel_round()
    }
}

int main() {
    char input[] = "je mappelle axel et jaime le chocolat";
    size_t n = strlen(input);
    printf("%s %zu\n", input, n);
}
