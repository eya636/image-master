#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <stdio.h>
#include <jpeglib.h>
#include <stddef.h>

unsigned char* read_jpeg_file(const char *filename, int *width, int *height, int *num_components);
void write_jpeg_file(const char *filename, unsigned char *image_data, int width, int height, int num_components);

#endif
