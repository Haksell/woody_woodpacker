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

void display_huffman_tree(HuffmanNode* root, size_t indentation) {
    if (!root) return;
    if (root->left) {
        display_huffman_tree(root->left, indentation + 2);
        for (size_t i = 0; i < indentation; ++i) printf(" ");
        printf("- %zu\n", root->frequency);
        display_huffman_tree(root->right, indentation + 2);
    } else {
        for (size_t i = 0; i < indentation; ++i) printf(" ");
        printf("- %zu (%hhu)\n", root->frequency, root->byte);
    }
}

void huffman_encode(uint8_t* bytes, size_t num_bytes) {
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

    display_huffman_tree(nodes[0], 0);
}

int main() {
    char s[] = "bonjour je mappelle axel et jaime le chocolat";
    huffman_encode((uint8_t*)s, strlen(s));
}
