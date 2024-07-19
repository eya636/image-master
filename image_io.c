#include "image_io.h"
#include <stdlib.h>

unsigned char* read_jpeg_file(const char *filename, int *width, int *height, int *num_components) {
    FILE *input_file = fopen(filename, "rb");
    if (input_file == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", filename);
        exit(EXIT_FAILURE);
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, input_file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *num_components = cinfo.output_components;

    unsigned char *image_data = (unsigned char *)malloc(*width * *height * *num_components);
    while (cinfo.output_scanline < *height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = image_data + (cinfo.output_scanline) * (*width) * (*num_components);
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(input_file);

    return image_data;
}

void write_jpeg_file(const char *filename, unsigned char *image_data, int width, int height, int num_components) {
    FILE *output_file = fopen(filename, "wb");
    if (output_file == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", filename);
        exit(EXIT_FAILURE);
    }

    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr_out;
    cinfo_out.err = jpeg_std_error(&jerr_out);
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, output_file);

    cinfo_out.image_width = width;
    cinfo_out.image_height = height;
    cinfo_out.input_components = num_components;
    cinfo_out.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo_out);
    jpeg_start_compress(&cinfo_out, TRUE);

    while (cinfo_out.next_scanline < height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = image_data + (cinfo_out.next_scanline) * width * num_components;
        jpeg_write_scanlines(&cinfo_out, buffer_array, 1);
    }

    jpeg_finish_compress(&cinfo_out);
    jpeg_destroy_compress(&cinfo_out);
    fclose(output_file);
}
