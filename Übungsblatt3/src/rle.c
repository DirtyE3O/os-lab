#include "../include/rle.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct RLENode {
    uint64_t count;
    struct RLENode* next;
} RLENode;

struct RLE {
    RLENode* head;
    RLENode* tail;
    uint64_t size;
};

static void append_to_rle(RLE* rle, uint64_t count) {
    RLENode* node = malloc(sizeof(RLENode));
    node->count = count;
    node->next = NULL;

    if (rle->tail) {
        rle->tail->next = node;
    }
    else {
        rle->head = node;
    }

    rle->tail = node;
    rle->size += 1;
}

/**
 * Create a new RLE data structure. The RLE data structure is a linked
 * list of RLENodes. Each RLENode contains a count of the number of
 * consecutive bits that are the same. The RLE data structure is
 * initialized with an entry.
 * @return a pointer to the RLE data structure
 */
RLE* create_rle() {
    RLE* rle = malloc(sizeof(RLE));
    rle->head = NULL;
    rle->tail = NULL;
    rle->size = 0;

    append_to_rle(rle, 0); // Start with a count of 0 bits

    return rle;
}

/**
 * Delete the RLE data structure and all of its nodes. This function
 * should be called when the RLE data structure is no longer needed.
 * @param rle the RLE data structure to delete
 */
void delete_rle(RLE* rle) {
    RLENode* node = rle->head;
    while (node) {
        RLENode* next = node->next;
        free(node);
        node = next;
    }
    free(rle);
}

static bool pop_head_rle(RLE* rle, uint64_t* count) {
    if (!rle->head) {
        return false;
    }

    RLENode* node = rle->head;
    *count = node->count;

    rle->head = node->next;
    if (rle->tail == node) {
        rle->tail = NULL;
    }

    free(node);
    rle->size -= 1;

    return true;
}

static uint64_t get_rle_total_count(RLE* rle) {
    uint64_t total = 0;
    RLENode* node = rle->head;
    while (node) {
        total += node->count;
        node = node->next;
    }
    return total;
}

/**
 * Fill rle counts with the provided data. The data should be treated as
 * binary data, not as a string, so the data is not null-terminated.
 *
 * This function counts the number of consecutive bits that are the
 * same and appends that count to the rle. The first entry of the rle
 * should always be the number of consecutive 0s at the beginning of
 * the data.
 *
 * For example, if the start of data is "00001111", then the rle should contain
 * two entries 4 and 4.
 *
 * If the start of data is "11110000", then the rle should contain three entries,
 * 0, 4, and 4
 * @param rle Will be filled with counts
 * @param data Source data, treated as binary data
 * @param size Size of the source data
 */
void encode_rle(RLE* rle, const char* data, size_t size) {
    uint8_t counting_bit = (rle->size & 1) ^ 1;

    for (size_t i = 0; i < size; i++) {
        for (int8_t j = 7; j >= 0; j--) {
            uint8_t current_bit = (data[i] >> j) & 1;
            if (current_bit == counting_bit) {
                rle->tail->count++;;
            }
            else {
                append_to_rle(rle, 1);
                counting_bit ^= 1; // Switch between 0 and 1
            }
        }
    }
}

/**
 * Decodes the rle to the appropriate binary data. The returned data
 * should be treated as binary data, not as a string, so the data is
 * not null-terminated.
 * @param rle assumed to be filled with counts
 * @param size will be set by this function and is the size of the returned data
 * @return binary data
 */
char* decode_rle(RLE* rle, size_t* size) {
    uint64_t total_bits = get_rle_total_count(rle);
    *size = (total_bits + 7) >> 3; // Round up to the nearest byte

    char* output = calloc(*size, sizeof(char));
    if (!output) {
        return NULL;
    }

    uint64_t count;
    size_t byte_index = 0;

    uint8_t bit = 0;
    uint8_t bit_index = 7;

    while (byte_index < *size && pop_head_rle(rle, &count)) {
        while (count > 0) {
            output[byte_index] |= bit << bit_index;
            if (bit_index == 0) {
                byte_index++;
                bit_index = 7;
            }
            else {
                bit_index--;
            }
            count--;
        }
        bit ^= 1; // Switch between 0 and 1
    }

    return output;
}

void print_rle(RLE* rle, uint8_t counts_per_line) {
    RLENode* node = rle->head;
    printf("{\n");
    int counter = 0;
    while (node) {
        printf("  %lu", node->count);
        if (node->next) printf(", "); // print comma only if this isn't the last node
        if (counts_per_line > 0 && ++counter >= counts_per_line) {
            printf("\n");
            counter = 0;
        }

        node = node->next;
    }
    printf(" }");
    printf("\n");
}

/**
 * Serializes the RLE structure into binary format.
 *
 * A = Art bit (0=zeros, 1=ones)
 * M = Mode bit (0=short, 1=long)
 *
 * @param rle The RLE structure to serialize
 * @param out_size Output size will be written here
 * @return Serialized binary data (must be freed by caller)
 */
char* serialize_rle(RLE* rle, size_t* out_size) {
    // Null check
    if (!rle || rle->size == 0) {
        *out_size = 0;
        return NULL;
    }

    // Allocate memory
    uint64_t total_bits = get_rle_total_count(rle);
    size_t max_bytes = (size_t)((total_bits + 62) / 63) + 1;
    char* data = malloc(max_bytes);

    // State variables
    uint8_t current_bit = 0;  // Current bit type (0 or 1)
    size_t byte_i = 0;        // Output byte index
    bool half = false;        // Is there a half byte ?
    uint8_t byte = 0;         // Current byte

    RLENode* node = rle->head;

    // Process each node
    while (node) {
        uint64_t remaining = node->count;

        // Split large counts into chunks (max 63)
        do {
            // Get count (max 63)
            uint8_t count;
            if (remaining > 63) {
                count = 63;
            }
            else {
                count = (uint8_t)remaining;
            }

            if (count <= 3) {

               uint8_t nibble = (current_bit << 3) | (0 << 2) | (count & 0x3);

                // Place nibble into byte
                if (!half) {
                    byte = nibble << 4;  // Upper 4 bits
                    half = true;
                }
                else {
                    byte |= nibble;      // Lower 4 bits
                    data[byte_i++] = byte;
                    byte = 0;
                    half = false;
                }
            }
            else {

                uint8_t nib1 = (current_bit << 3) | (1 << 2) | ((count >> 4) & 0x3);
                uint8_t nib2 = count & 0xF;

                // Write nibble 1
                if (!half) {
                    byte = nib1 << 4;
                    half = true;
                }
                else {
                    byte |= nib1;
                    data[byte_i++] = byte;
                    byte = 0;
                    half = false;
                }

                // Write nibble 2
                if (!half) {
                    byte = nib2 << 4;
                    half = true;
                }
                else {
                    byte |= nib2;
                    data[byte_i++] = byte;
                    byte = 0;
                    half = false;
                }
            }

            remaining -= count;
        }
        while (remaining > 0);

        // Toggle bit type for next node
        current_bit ^= 1;
        node = node->next;
    }

    // Write remaining half byte (with padding)
    if (half) {
        data[byte_i++] = byte;
    }

    *out_size = byte_i;
    return data;
}

/**
 * Deserializes binary data into an RLE structure.
 *
 * Performs the reverse operation of serialize. Merges consecutive
 * nibbles with the same A bit (large counts may have been split).
 *
 * @param rle The RLE structure to fill
 * @param data Binary data to deserialize
 * @param size Size of the data
 */
void deserialize_rle(RLE* rle, const char* data, size_t size) {
    if (size == 0) return;

    // Remove initial [0] node (comes from create_rle)
    if (rle->size == 1 && rle->head->count == 0) {
        uint64_t dummy;
        pop_head_rle(rle, &dummy);
    }

    // If first bit is 1, prepend [0] (RLE always starts with zeros)
    if (data[0] & 0x80) {
        append_to_rle(rle, 0);
    }

    // Track last processed bit type (for merging)
    uint8_t last_bit;
    if (data[0] & 0x80) {
        last_bit = 1;
    }
    else {
        last_bit = 0;
    }

    bool jump = false;  // Skip next nibble in 6-bit mode

    // Process each byte
    for (size_t i = 0; i < size; i++) {
        // Split byte into two nibbles
        uint8_t byte = (uint8_t)data[i];
        uint8_t nibbles[2] = {(byte >> 4) & 0x0F, byte & 0x0F};

        // Process each nibble
        for (int n = 0; n < 2; n++) {
            if (jump) {
                jump = false;
                continue;
            }

            uint8_t nibble = nibbles[n];

            if (i == size - 1 && nibble == 0) {
                break;
            }

            // Extract A and M bits
            uint8_t A = (nibble >> 3) & 0x1;  // Art bit
            uint8_t M = (nibble >> 2) & 0x1;  // Mode bit
            uint8_t count;

            if (M == 0) {
                count = nibble & 0x3;
            }
            else {
                // Get next nibble
                uint8_t next_nibble;
                if (n == 0) {
                    next_nibble = nibbles[1];  // Same byte
                }
                else if (i + 1 < size) {
                    next_nibble = ((uint8_t)data[i + 1] >> 4) & 0x0F;  // Next byte
                }
                else {
                    next_nibble = 0;
                }

                count = ((nibble & 0x3) << 4) | (next_nibble & 0x0F);

                if (n == 0) {
                    n = 1;  // Skip nibble in same byte
                }
                else {
                    jump = true;  // Skip first nibble of next byte
                }
            }

            // Add to RLE (merge if same A, new node if different)
            if (rle->size > 0 && A == last_bit) {
                // Same bit type - add to current node (merge)
                rle->tail->count += count;
            }
            else {
                // Different bit type - create new node
                append_to_rle(rle, count);
                last_bit = A;
            }
        }
    }

    if (rle->size % 2 == 1) {
        append_to_rle(rle, 0);
    }
}
