#include <stdio.h>
#include <stdlib.h>
#include "../include/rle.h"

int main() {
    // Beispiel: Serialisierte RLE-Daten direkt (bereits im RLE-Format)
    //char serialized_data[] = { 0x3A, 0x30}; // Beispielwert
    //char serialized_data[] = {0x46,0xC4,0x1D,0x50}; // Beispielwert
    char serialized_data[] = { 0x3F, 0x01};
    size_t serialized_size = sizeof(serialized_data);

    // Neues RLE erzeugen und deserialisieren
    RLE* rle = create_rle();
    deserialize_rle(rle, serialized_data, serialized_size);

    // Ausgabe der deserialisierten Counts
    printf("Deserialized RLE counts:\n");
    print_rle(rle,7);

    // Aufräumen
    delete_rle(rle);

    return 0;
}
