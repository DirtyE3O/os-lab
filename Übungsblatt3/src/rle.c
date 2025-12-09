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
    } else {
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
            } else {
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
            } else {
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

char* serialize_rle(RLE *rle, size_t* out_size) {
    
  }
void deserialize_rle(RLE *rle, const char *data, size_t size) {
    if (rle->size == 1 && rle->head->count == 0) {
        uint64_t dummy;
        pop_head_rle(rle, &dummy);
    }
    // prüfen, ob erstes Count-Bit 1 →  0 einfügen
    if (data[0] & 0x80) {
        append_to_rle(rle, 0);
    }
    uint8_t expected_bit = 0; // Anfang immer 0 in rle
    bool jump = false;

    for (size_t i = 0; i < size; i++) {
        uint8_t byte = (uint8_t)data[i]; // sicheres Casten
        uint8_t nibbles[2];
        nibbles[0] = (byte >> 4) & 0x0F; // löscht dann führende 0, also mehr als 8 bit
        nibbles[1] = byte & 0x0F;
        
        for (int n = 0; n < 2; n++) {
            if (jump == true){
              n++;
            }
            
            uint8_t nibble = nibbles[n];
            if (i == size - 1 && nibble == 0) { // sonst 0 am ende zu viel
                break;
            }
            uint8_t B = (nibble >> 3) & 0x1; // Bittyp (0 oder 1) – wird ignoriert, da alternierend
            uint8_t M = (nibble >> 2) & 0x1; // Mode: 0 = 2-Bit-Länge, 1 = 6-Bit-Länge
            uint8_t count = 0;

            if (M == 0) {
                count = nibble & 0x3; // 2 bits berücksichtigt
            } else {
                // 6 bits
                uint8_t next_nibble;
                if (n == 0) {
                    next_nibble = nibbles[1]; // nibble ist im selben byte
                } else if (i + 1 < size) {
                    next_nibble = ((uint8_t)data[i + 1] >> 4) & 0x0F;
                } else {
                    next_nibble = 0;
                }

                count = ((nibble & 0x3) << 4) | (next_nibble & 0x0F);

                
                if (n == 0) {
                  n = 1;
                  jump = false;
                  
                } // nibble übersprungen da wir es schon genommen haben n->2 am ende 
                else {jump = true;}
            }
            
            append_to_rle(rle, count);

            expected_bit = expected_bit ^ 1;
        }
    }
    if ((rle->size)%2 == 1){
      append_to_rle(rle, 0);
    }
}
