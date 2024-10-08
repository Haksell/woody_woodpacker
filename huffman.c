// TODO: free_huffman_tree

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

typedef struct HuffmanMap {
    uint8_t pos[256];
    uint8_t size[256];
    size_t num_bits;
    uint8_t* bits;
} HuffmanMap;

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

size_t count_bits_in_map(HuffmanNode* tree, size_t depth) {
    if (!tree || !tree->left) return depth;
    return count_bits_in_map(tree->left, depth + 1) +
           count_bits_in_map(tree->right, depth + 1);
}

void construct_map(HuffmanMap* map, HuffmanNode* tree) {
    map->num_bits = count_bits_in_map(tree, 0);
    size_t bytes_in_map = (map->num_bits + 7) >> 3;
    map->bits = malloc(bytes_in_map); // TODO: check
    size_t bits_idx;
    for (uint16_t byte = 0; byte < 256; ++byte) {
        bits_idx += add_byte_to_map(byte, tree);
    }
    printf("%zu\n", map->num_bits);
}

// size_t count_compressed_bits_in_byte(uint8_t byte, HuffmanNode* tree) {
//     while (tree) {
//     }
// }

// size_t count_compressed_bits_in_bytes(
//     const uint8_t* bytes,
//     size_t num_bytes,
//     HuffmanNode* tree
// ) {
//     size_t compressed_bits = 0;
//     for (size_t i = 0; i < num_bytes; ++i)
//         compressed_bits += count_compressed_bits_in_byte(bytes[i], tree);
//     return compressed_bits;
// }

// size_t huffman_compress(
//     const uint8_t* bytes,
//     size_t num_bytes,
//     HuffmanNode* tree,
//     uint8_t** compressed
// ) {
//     size_t compressed_bits = count_compressed_bits_in_bytes(bytes, num_bytes, tree);
//     return 0;
// }

int main() {
    char s[] = "bonjour je mappelle axel et jaime le chocolat";
    size_t n = strlen(s);
    HuffmanNode* tree = construct_tree((uint8_t*)s, n);
    HuffmanMap map;
    construct_map(&map, tree);
    // uint8_t* compressed;
    // size_t compressed_bits = huffman_compress((uint8_t*)s, n, tree, &compressed);
}
