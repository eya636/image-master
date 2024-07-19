#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "image_io.h"

#define NUM_ITERATIONS_PER_PIXEL 300

int main() {
    const char *input_filename = "input.jpg";
    const char *output_filename = "output_sequential_extended.jpg";
    
    int width, height, num_components;
    unsigned char *image_data = read_jpeg_file(input_filename, &width, &height, &num_components);
    
    clock_t start = clock();

    for (int iteration = 0; iteration < 10000; iteration++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = (y * width + x) * num_components;
                unsigned char gray = 0.299 * image_data[index] + 0.587 * image_data[index + 1] + 0.114 * image_data[index + 2];
                
                // Boucle supplémentaire pour augmenter le temps de calcul sans changer le résultat
                for (int i = 0; i < NUM_ITERATIONS_PER_PIXEL; i++) {
                    gray = (gray + i) % 256;
                }
                
                // Restaurer la valeur du gris calculée initialement
                gray = 0.299 * image_data[index] + 0.587 * image_data[index + 1] + 0.114 * image_data[index + 2];
                image_data[index] = gray;
                image_data[index + 1] = gray;
                image_data[index + 2] = gray;
            }
        }
    }

    clock_t end = clock();

    write_jpeg_file(output_filename, image_data, width, height, num_components);
    free(image_data);
    
    double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Temps d'exécution : %.2f secondes\n", elapsed_time);

    return 0;
}
