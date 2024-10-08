// TODO: free stuff

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HuffmanNode {
    uint8_t byte;
    size_t frequency;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

typedef struct Compressor {
    uint16_t starts[256];
    uint16_t sizes[256];
    uint16_t num_bits;
    uint64_t* bits;
} Compressor;

HuffmanNode* huffman_leaf(uint8_t byte, size_t frequency) {
    HuffmanNode* node = malloc(sizeof(HuffmanNode));
    node->byte = byte;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

HuffmanNode* huffman_pair(HuffmanNode* left, HuffmanNode* right) {
    HuffmanNode* node = malloc(sizeof(HuffmanNode));
    node->byte = 0;
    node->frequency = left->frequency + right->frequency;
    node->left = left;
    node->right = right;
    return node;
}

void display_huffman_tree(HuffmanNode* tree, size_t indentation) {
    if (!tree) return;
    if (tree->left) {
        display_huffman_tree(tree->left, indentation + 2);
        for (size_t i = 0; i < indentation; ++i) printf(" ");
        printf("- %zu\n", tree->frequency);
        display_huffman_tree(tree->right, indentation + 2);
    } else {
        for (size_t i = 0; i < indentation; ++i) printf(" ");
        printf("- %zu (%hhu)\n", tree->frequency, tree->byte);
    }
}

HuffmanNode* construct_tree(const uint8_t* bytes, size_t num_bytes) {
    size_t frequencies[256] = {0};
    for (size_t i = 0; i < num_bytes; ++i) ++frequencies[bytes[i]];

    HuffmanNode* nodes[256] = {NULL};
    size_t num_nodes = 0;
    for (size_t byte = 0; byte < 256; ++byte) {
        if (frequencies[byte]) {
            nodes[num_nodes] = huffman_leaf(byte, frequencies[byte]);
            ++num_nodes;
        }
    }

    while (num_nodes > 1) {
        size_t min_freq_1 = SIZE_MAX;
        size_t min_freq_2 = SIZE_MAX;
        size_t min_idx_1 = 0;
        size_t min_idx_2 = 0;
        for (size_t i = 0; i < num_nodes; ++i) {
            if (nodes[i]->frequency < min_freq_1) {
                min_freq_2 = min_freq_1;
                min_idx_2 = min_idx_1;
                min_freq_1 = nodes[i]->frequency;
                min_idx_1 = i;
            } else if (nodes[i]->frequency < min_freq_2) {
                min_freq_2 = nodes[i]->frequency;
                min_idx_2 = i;
            }
        }
        HuffmanNode* new_node = huffman_pair(nodes[min_idx_1], nodes[min_idx_2]);
        if (min_idx_1 < min_idx_2) {
            nodes[min_idx_1] = new_node;
            nodes[min_idx_2] = nodes[num_nodes - 1];
        } else {
            nodes[min_idx_1] = nodes[num_nodes - 1];
            nodes[min_idx_2] = new_node;
        }
        --num_nodes;
    }

    return nodes[0];
}

void compute_sizes(uint16_t* sizes, HuffmanNode* tree, uint16_t depth) {
    if (!tree->left) {
        sizes[tree->byte] = depth;
    } else {
        compute_sizes(sizes, tree->left, depth + 1);
        compute_sizes(sizes, tree->right, depth + 1);
    }
}

uint16_t compute_starts(Compressor* compressor) {
    uint16_t start = 0;
    for (uint16_t byte = 0; byte < 256; ++byte) {
        compressor->starts[byte] = start;
        start += compressor->sizes[byte];
    }
    return start;
}

void fill_bits(
    Compressor* compressor,
    HuffmanNode* tree,
    bool stack[256],
    uint16_t depth
) {
    if (!tree->left) {
        uint16_t start = compressor->starts[tree->byte];
        for (uint16_t i = 0; i < depth; ++i) {
            uint16_t bit_idx = start + i;
            // printf("bit_idx->%u %u\n", bit_idx, stack[i]);
            if (stack[i])
                compressor->bits[bit_idx >> 6] |= 1llu << (uint64_t)(bit_idx & 63);
        }
    } else {
        stack[depth] = 1;
        fill_bits(compressor, tree->right, stack, depth + 1);
        stack[depth] = 0;
        fill_bits(compressor, tree->left, stack, depth + 1);
    }
}

void construct_map(Compressor* compressor, HuffmanNode* tree) {
    compute_sizes(compressor->sizes, tree, 0);
    compressor->num_bits = compute_starts(compressor);
    compressor->bits = calloc((compressor->num_bits + 63) >> 6, 8);
    bool stack[256] = {0};
    fill_bits(compressor, tree, stack, 0);
    // exit(0);
}

size_t huffman_compress(
    const uint8_t* bytes,
    size_t num_bytes,
    Compressor* compressor,
    uint64_t** compressed
) {
    size_t compressed_size = 0;
    for (size_t i = 0; i < num_bytes; ++i)
        compressed_size += compressor->sizes[bytes[i]];
    *compressed = calloc((compressed_size + 63) >> 6, 8);
    size_t compressed_idx = 0;
    for (size_t i = 0; i < num_bytes; ++i) {
        uint8_t byte = bytes[i];
        uint16_t start = compressor->starts[byte];
        uint16_t end = start + compressor->sizes[byte];
        for (size_t j = start; j < end; ++j) {
            uint64_t bit = (compressor->bits[j >> 6] >> (j & 63)) & 1;
            (*compressed)[compressed_idx >> 6] |= bit << (compressed_idx & 63);
            ++compressed_idx;
        }
    }
    return compressed_size;
}

void show_compressed_form(uint64_t* compressed, size_t compressed_bits) {
    for (size_t i = 0; i < ((compressed_bits + 63) >> 6); ++i)
        printf("%#018zx ", compressed[i]);
    printf("\n");
}

int main() {
    char input[] = "bonjour je mappelle axel et jaime le chocolat";
    puts(input);
    size_t n = strlen(input);
    HuffmanNode* tree = construct_tree((uint8_t*)input, n);
    Compressor compressor = {0};
    construct_map(&compressor, tree);
    uint64_t* compressed;
    size_t compressed_bits = huffman_compress(
        (uint8_t*)input,
        n,
        &compressor,
        &compressed
    );
    show_compressed_form(compressed, compressed_bits);
    // uint64_t* decompresed_bytes;
    // size_t decompressed_bytes = huffman_decompress(
    //     tree,
    //     compressed,
    //     compressed_bits,
    //     &decompresed_bytes
    // );
}
