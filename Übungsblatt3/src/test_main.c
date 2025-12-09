#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/rle.h"

int main() {
    // Beispiel: Serialisierte RLE-Daten direkt (bereits im RLE-Format)
    //char serialized_data[] = {0x41}; // 6 bit im 1 byte
    //char serialized_data[] = { 0x3A, 0x30}; //bsp 1 // 0x38 statt 30
    //char serialized_data[] = { 0x3F, 0x01};  // zu viel 0x80 am ende
    char serialized_data[] = {0x46,0xC4,0x1D,0x50};
    size_t serialized_size = sizeof(serialized_data); 

    RLE* rle = create_rle();
    deserialize_rle(rle, serialized_data, serialized_size);

    printf("Deserialized counts:\n");
    print_rle(rle, 7);

    size_t reserialized_size = 0;
    char* reserialized_data = serialize_rle(rle, &reserialized_size);

    printf("Reserialized bytes:\n");
    for (size_t i = 0; i < reserialized_size; i++) {
        printf("0x%02X ", (unsigned char)reserialized_data[i]);
    }
    printf("\n");

    RLE* rle2 = create_rle();
    deserialize_rle(rle2, reserialized_data, reserialized_size);

    printf("RLE counts after test \n");
    print_rle(rle2, 7);

    free(reserialized_data);
    delete_rle(rle);
    delete_rle(rle2);

    return 0;
}
