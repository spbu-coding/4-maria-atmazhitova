#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma once

struct color_t {
	unsigned char red, green, blue;
};

struct coord_t {
	int by_width, by_height;
};

struct color_t convert_color_to_negative(struct color_t color);

bool check_if_colors_are_equal(struct color_t color1, struct color_t color2);

uint32_t convert_uchar_array_to_uint32(unsigned char *array, bool uses_little_endian);

int32_t convert_uchar_array_to_int32(unsigned char *array, bool uses_little_endian);

uint16_t convert_uchar_array_to_uint16(unsigned char *array, bool uses_little_endian);

void convert_uint16_to_uchar_array(uint16_t num, unsigned char *array, bool uses_little_endian);

void convert_uint32_to_uchar_array(uint32_t num, unsigned char *array, bool uses_little_endian);

void convert_int32_to_uchar_array(int32_t num, unsigned char *array, bool uses_little_endian);

void get_bytes_from_buffer(unsigned char *buffer, unsigned char *destination, unsigned int amount_of_bytes);

void put_bytes_to_buffer(unsigned char *buffer, unsigned char *source, unsigned int amount_of_bytes);

unsigned int get_file_size(FILE *file);

struct BMP_t {
	bool uses_little_endian;
	uint32_t f_size;
	uint32_t off_bits;
	uint32_t size;
	int32_t width;
	int32_t height;
	uint16_t bit_count;
	uint32_t size_image;
	int32_t x_pels_per_meter;
	int32_t y_pels_per_meter;
	uint32_t clr_used;
	uint32_t clr_important;
	struct color_t *color_table;
	struct color_t **pixel_array_24bpp;
	unsigned char **pixel_array_8bpp;
};

struct color_t get_pixel_color(struct BMP_t *picture, int32_t coord_by_width, int32_t coord_by_height);

void read_BMP_header(FILE *file, struct BMP_t *picture);

void read_BMP_palette(FILE *file, struct BMP_t *picture);

void read_BMP_pixel_array_8bpp(FILE *file, struct BMP_t *picture);

void read_BMP_pixel_array_24bpp(FILE *file, struct BMP_t *picture);

void read_BMP(char *filename, struct BMP_t *picture);

void make_BMP_negative(struct BMP_t *picture);

void save_BMP_header(FILE *file, struct BMP_t *picture);

void save_BMP_pixel_array_24bpp(FILE *file, struct BMP_t *picture);

void save_BMP_palette(FILE *file, struct BMP_t *picture);

void save_BMP_pixel_array_8bpp(FILE *file, struct BMP_t *picture);

void save_BMP(char *filename, struct BMP_t *picture);

void free_BMP(struct BMP_t *picture);

int compare_BMP(struct BMP_t *picture1, struct BMP_t *picture2, struct coord_t *coords_of_mismatching_pixels);