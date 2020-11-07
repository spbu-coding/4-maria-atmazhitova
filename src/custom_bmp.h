#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// TODO: заставить реализацию из qdbmp работать

const unsigned int BITMAPFILEHEADER_SIZE = 14;
const unsigned int BITMAPINFOHEADER_SIZE = 40;
const unsigned int BITMAPHEADER_SIZE = 54;
const unsigned int BFTYPE_SIZE = 2; 
const unsigned int BFSIZE_SIZE = 4;
const unsigned int BFRESERVED_SIZE = 4;
const unsigned int BFOFFBITS_SIZE = 4;
const unsigned int SIZE_SIZE = 4;
const unsigned int WIDTH_SIZE = 4;
const unsigned int HEIGHT_SIZE = 4;
const unsigned int PLANES_SIZE = 2;
const unsigned int BITCOUNT_SIZE = 2;
const unsigned int COMPRESSION_SIZE = 4;
const unsigned int SIZEIMAGE_SIZE = 4;
const unsigned int XPELSPERMETER_SIZE = 4;
const unsigned int YPELSPERMETER_SIZE = 4;
const unsigned int CLRUSED_SIZE = 4;
const unsigned int CLRIMPORTANT_SIZE = 4;
const unsigned int BYTES_PER_COLOR_IN_TABLE = 4;
const unsigned int BYTES_PER_PIXEL_8BPP = 1;
const unsigned int BYTES_PER_PIXEL_24BPP = 3;
const unsigned int BITS_PER_PIXEL_8BPP = 8;
const unsigned int BITS_PER_PIXEL_24BPP = 24;
const unsigned int MAX_COLORS_IN_PALETTE = 256;
const unsigned int MAX_EXTRA_BYTES = 3;

const char *FORMAT_STRING[] = {"%0s", "%1s", "%2s", "%3s", "%4s", "%5s"};

struct color_t {
	char red, green, blue;
};

struct coord_t {
	int by_width, by_height;
};

struct color_t convert_color_to_negative(struct color_t color) {
	struct color_t result = { .red = ~(color.red), .green = ~(color.green), .blue = ~(color.blue) };
	return result;
}

bool check_if_colors_are_equal(struct color_t color1, struct color_t color2) {
	return (color1.red == color2.red && color1.green == color2.green && color1.blue == color2.blue);
}

uint32_t convert_uchar_array_to_uint32(char *array, bool uses_little_endian) {
	const uint32_t char_size = 8;
	if (uses_little_endian) {
		return (uint32_t)(array[0]) + (((uint32_t)(array[1])) << char_size) + (((uint32_t)(array[2])) << (char_size * 2)) + (((uint32_t)(array[3])) << (char_size * 3));
	}
	else {
		return (uint32_t)(array[3]) + (((uint32_t)(array[2])) << char_size) + (((uint32_t)(array[1])) << (char_size * 2)) + (((uint32_t)(array[0])) << (char_size * 3));
	}
}

int32_t convert_uchar_array_to_int32(char *array, bool uses_little_endian) {
	const int32_t char_size = 8;
	if (uses_little_endian) {
		return (int32_t)(array[0]) + (((int32_t)(array[1])) << char_size) + (((int32_t)(array[2])) << (char_size * 2)) + (((int32_t)(array[3])) << (char_size * 3));
	}
	else {
		return (int32_t)(array[3]) + (((int32_t)(array[2])) << char_size) + (((int32_t)(array[1])) << (char_size * 2)) + (((int32_t)(array[0])) << (char_size * 3));
	}
}

uint16_t convert_uchar_array_to_uint16(char *array, bool uses_little_endian) {
	const uint16_t char_size = 8;
	if (uses_little_endian) {
		return (uint16_t)(array[0]) + (((uint16_t)(array[1])) << char_size);
	}
	else {
		return (uint16_t)(array[1]) + (((uint16_t)(array[0])) << char_size);
	}
}

void convert_uint16_to_uchar_array(uint16_t num, char *array, bool uses_little_endian) {
	const uint16_t max_char = 255, char_size = 8;
	if (uses_little_endian) {
		array[0] = num & max_char;
		array[1] = num >> char_size;
	}
	else {
		array[1] = num & max_char;
		array[0] = num >> char_size;
	}
}

void convert_uint32_to_uchar_array(uint32_t num, char *array, bool uses_little_endian) {
	const uint32_t char_size = 8, max_char = 255;
	if (uses_little_endian) {
		for (int i = 0; i < 4; i++) {
			array[i] = num & max_char;
			num >>= char_size;
		}
	}
	else {
		for (int i = 0; i < 4; i++) {
			array[3 - i] = num & max_char;
			num >>= char_size;
		}
	}
}

void convert_int32_to_uchar_array(int32_t num, char *array, bool uses_little_endian) {
	const int32_t char_size = 8, max_char = 255;
	if (uses_little_endian) {
		for (int i = 0; i < 4; i++) {
			array[i] = num & max_char;
			num >>= char_size;
		}
	}
	else {
		for (int i = 0; i < 4; i++) {
			array[3 - i] = num & max_char;
			num >>= char_size;
		}
	}
}

struct BMP_t {
	bool uses_little_endian;
	uint32_t f_size;
	uint32_t off_bits;
	uint32_t size;
	int32_t width;
	int32_t height;
	uint16_t bit_count;
	uint32_t compression;
	uint32_t size_image;
	int32_t x_pels_per_meter;
	int32_t y_pels_per_meter;
	uint32_t clr_used;
	uint32_t clr_important;
	struct color_t *color_table;
	struct color_t **pixel_array_24bpp;
	char **pixel_array_8bpp;
	uint32_t real_file_size;
};


void read_BMP_header(FILE *file, struct BMP_t *picture) {
	picture -> real_file_size = BITMAPHEADER_SIZE;
	char buffer[BITMAPHEADER_SIZE];
	int reading_result = fread(buffer, sizeof(char), BITMAPHEADER_SIZE, file);
	if (reading_result != (int)(BITMAPHEADER_SIZE)) {
		fprintf(stderr, "Could not read file header\n");
		exit(-2);
	}
	char *current_buffer_pos = buffer;

	char f_type[BFTYPE_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[BFTYPE_SIZE], f_type);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read file format\n");
		exit(-2);
	}
	current_buffer_pos += BFTYPE_SIZE;

	if (f_type[0] == 0x4D && f_type[1] == 0x42) {
		picture->uses_little_endian = 0;
	}
	else if (f_type[0] == 0x42 && f_type[1] == 0x4D) {
		picture->uses_little_endian = 1;
	}
	else {
		fprintf(stderr, "The file format is not BMP: expected BM as first two bytes, got %c%c\n", f_type[0], f_type[1]);
		exit(-1);
	}

	char f_size[BFSIZE_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[BFSIZE_SIZE], f_size);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read file size\n");
		exit(-2);
	}
	picture->f_size = convert_uchar_array_to_uint32(f_size, picture->uses_little_endian);
	current_buffer_pos += BFSIZE_SIZE;

	for (unsigned int i = 0; i < BFRESERVED_SIZE; i++) {
		char reserved;
		reading_result = sscanf(current_buffer_pos, "%c", &reserved);
		if (reading_result != 1) {
			fprintf(stderr, "Could not read the reserved byte #%d\n", i + 1);
			exit(-2);
		}
		current_buffer_pos += reading_result;
		if (reserved != 0) {
			fprintf(stderr, "File is broken: reserved byte #%d is not equal to zero\n", i + 1);
			exit(-2);
		}
	}

	char off_bits[BFOFFBITS_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[BFOFFBITS_SIZE], off_bits);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the offset\n");
		exit(-2);
	}
	picture->off_bits = convert_uchar_array_to_uint32(off_bits, picture->uses_little_endian);
	if (picture->off_bits < BITMAPHEADER_SIZE) {
		fprintf(stderr, "The offset should be at least 54, but it is equal to %u\n", picture->off_bits);
		exit(-1);
	}
	current_buffer_pos += BFOFFBITS_SIZE;

	char size[SIZE_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[SIZE_SIZE], size);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the bitmap header size\n");
		exit(-2);
	}
	picture->size = convert_uchar_array_to_uint32(size, picture->uses_little_endian);
	if (picture->size != BITMAPINFOHEADER_SIZE) {
		fprintf(stderr, "This is not a BMP version 3 file: expected 40 as info header size, got %d\n", picture->size);
		exit(-1);
	}
	current_buffer_pos += SIZE_SIZE;

	char width[WIDTH_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[WIDTH_SIZE], width);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the width\n");
		exit(-2);
	}
	picture->width = convert_uchar_array_to_int32(width, picture->uses_little_endian);
	if (picture->width <= 0) {
		fprintf(stderr, "The width is not positive\n");
		exit(-1);
	}
	current_buffer_pos += WIDTH_SIZE;

	char height[HEIGHT_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[HEIGHT_SIZE], height);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the height\n");
		exit(-2);
	}
	picture->height = convert_uchar_array_to_int32(height, picture->uses_little_endian);
	if (picture->height == 0) {
		fprintf(stderr, "The height is equal to zero\n");
		exit(-1);
	}
	current_buffer_pos += HEIGHT_SIZE;

	char planes[PLANES_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[PLANES_SIZE], planes);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the planes\n");
		exit(-2);
	}
	if (((planes[0] != 0 || planes[1] != 1) && !picture->uses_little_endian) ||
		((planes[0] != 1 || planes[1] != 0) && picture->uses_little_endian)) {
		fprintf(stderr, "Planes are not equal to 1\n");
		exit(-1);
	}
	current_buffer_pos += PLANES_SIZE;

	char bit_count[BITCOUNT_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[BITCOUNT_SIZE], bit_count);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the amount of bits per pixel\n");
		exit(-2);
	}
	picture->bit_count = convert_uchar_array_to_uint16(bit_count, picture->uses_little_endian);
	if (picture->bit_count != BITS_PER_PIXEL_8BPP && picture->bit_count != BITS_PER_PIXEL_24BPP) {
		fprintf(stderr, "The amount of bits per pixel is not equal to 8 or 24\n");
		exit(-1);
	}
	if (picture->bit_count == BITS_PER_PIXEL_24BPP && picture->off_bits != BITMAPHEADER_SIZE) {
		fprintf(stderr, "The file is broken: bit offset should be equal to 54, but it is equal to %u\n", picture->off_bits);
		exit(-1);
	}
	current_buffer_pos += BITCOUNT_SIZE;

	char compression[COMPRESSION_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[COMPRESSION_SIZE], compression);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read the file compression method\n");
		exit(-2);
	}
	picture->compression = convert_uchar_array_to_uint32(compression, picture->uses_little_endian);
	if (picture->compression != 0) {
		fprintf(stderr, "File uses compression to store pixels, this is not supported\n");
		exit(-1);
	}
	current_buffer_pos += COMPRESSION_SIZE;

	char size_image[SIZEIMAGE_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[SIZEIMAGE_SIZE], size_image);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read amount of pixel data\n");
		exit(-2);
	}
	picture->size_image = convert_uchar_array_to_uint32(size_image, picture->uses_little_endian);
	current_buffer_pos += SIZEIMAGE_SIZE;

	char x_pels_per_meter[XPELSPERMETER_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[XPELSPERMETER_SIZE], x_pels_per_meter);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read horizontal resolution\n");
		exit(-2);
	}
	picture->x_pels_per_meter = convert_uchar_array_to_int32(x_pels_per_meter, picture->uses_little_endian);
	current_buffer_pos += XPELSPERMETER_SIZE;

	char y_pels_per_meter[YPELSPERMETER_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[YPELSPERMETER_SIZE], y_pels_per_meter);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read vertical resolution\n");
		exit(-2);
	}
	picture->y_pels_per_meter = convert_uchar_array_to_int32(y_pels_per_meter, picture->uses_little_endian);
	current_buffer_pos += YPELSPERMETER_SIZE;

	char clr_used[CLRUSED_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[CLRUSED_SIZE], clr_used);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read size of color table\n");
		exit(-2);
	}
	picture->clr_used = convert_uchar_array_to_uint32(clr_used, picture->uses_little_endian);
	if ((picture->clr_used != 0 && picture->bit_count == BITS_PER_PIXEL_24BPP)) {
		fprintf(stderr, "Size of color table doesn't match the amount of bits per pixel\n");
		exit(-1);
	}
	current_buffer_pos += CLRUSED_SIZE;

	char clr_important[CLRIMPORTANT_SIZE + 1];
	reading_result = sscanf(current_buffer_pos, FORMAT_STRING[CLRIMPORTANT_SIZE], clr_important);
	if (reading_result != 1) {
		fprintf(stderr, "Could not read important size of color table\n");
		exit(-2);
	}
	picture->clr_important = convert_uchar_array_to_uint32(clr_important, picture->uses_little_endian);
	if ((picture->clr_used != 0 && picture->clr_important > picture->clr_used) || picture->clr_used > 256) {
		fprintf(stderr, "Important size of color table is greater than size of color table\n");
		exit(-2);
	}
}

void read_BMP_palette(FILE *file, struct BMP_t *picture) {
	uint32_t clr_used_num = picture->clr_used;
	if (picture->clr_used == 0) {
		clr_used_num = MAX_COLORS_IN_PALETTE;
	}
	picture->color_table = (struct color_t *)(malloc(clr_used_num * sizeof(struct color_t)));
	if (picture->color_table == NULL) {
		fprintf(stderr, "Could not allocate memory for color table\n");
		exit(-2);
	}

	picture->real_file_size += clr_used_num;

	unsigned int color_table_size = clr_used_num * BYTES_PER_COLOR_IN_TABLE;
	char *buffer = (char*)(malloc(color_table_size)), *current_buffer_pos = buffer;
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory for color table\n");
		exit(-2);
	}
	int reading_result = fread(buffer, sizeof(char), color_table_size, file);
	if (reading_result != (int)(color_table_size)) {
		fprintf(stderr, "Could not read the color table\n");
		exit(-2);
	}

	for (uint32_t i = 0; i < clr_used_num; i++) {
		char color[BYTES_PER_COLOR_IN_TABLE + 1];
		reading_result = sscanf(current_buffer_pos, FORMAT_STRING[BYTES_PER_COLOR_IN_TABLE], color);
		if (reading_result != 1) {
			fprintf(stderr, "Could not read color #%d of color table\n", i + 1);
			exit(-2);
		}
		picture->color_table[i] = (struct color_t){ .blue = color[0], .green = color[1], .red = color[2] };
		if (color[3] != 0) {
			fprintf(stderr, "Fourth (reserved) byte of color #%d in color table is not equal to zero\n", i + 1);
			exit(-1);
		}
		current_buffer_pos += BYTES_PER_COLOR_IN_TABLE;
	}

	free(buffer);
}

void read_BMP_pixel_array_8bpp(FILE *file, struct BMP_t *picture) {
	int32_t abs_of_height = picture->height;
	if (abs_of_height < 0) {
		abs_of_height *= -1;
	}
	picture->pixel_array_8bpp = (char **)(malloc(abs_of_height * sizeof(char *)));
	if (picture->pixel_array_8bpp == NULL) {
		fprintf(stderr, "Could not allocate memory for pixel array\n");
		exit(-2);
	}

	unsigned int buffer_size = abs_of_height * picture->width * sizeof(char);
	char *buffer = (char *)(malloc(buffer_size));
	char *current_buffer_pos = buffer;
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory for pixel array\n");
		exit(-2);
	}
	int reading_result = fread(buffer, sizeof(char), buffer_size, file);
	if (reading_result != (int)(buffer_size)) {
		if (feof(file) > 0) {
			fprintf(stderr, "Height and/or width do not match real file size\n");
			exit(-1);
		}
		fprintf(stderr, "Could not read pixel array\n");
		exit(-2);
	}

	for (int i = 0; i < abs_of_height; i++) {
		picture->pixel_array_8bpp[i] = (char *)(malloc(picture->width * sizeof(char)));
		if (picture->pixel_array_8bpp[i] == NULL) {
			fprintf(stderr, "Could not allocate memory for row %d of pixel array\n", i);
			exit(-2);
		}
		for (int j = 0; j < picture->width; j++) {
			unsigned char color_number;
			reading_result = sscanf(current_buffer_pos, "%c", &color_number);
			if (reading_result != (int)(BYTES_PER_PIXEL_8BPP)) {
				fprintf(stderr, "Could not read pixel with coords (%d, %d)\n", j, i);
				exit(-2);
			}
			picture->pixel_array_8bpp[i][j] = color_number;
			current_buffer_pos += BYTES_PER_PIXEL_8BPP;
		}
		if (picture->width % 4 != 0) { // in this case there are extra null bytes in each row to make each row length possible to divide by 4
			char extra_bytes[MAX_EXTRA_BYTES + 1];
			uint32_t amount_of_extra_bytes = 4 - picture->width % 4;
			reading_result = sscanf(current_buffer_pos, FORMAT_STRING[amount_of_extra_bytes], extra_bytes);
			if (reading_result != amount_of_extra_bytes) {
				fprintf(stderr, "Could not read extra bytes in row %d\n", i);
				exit(-2);
			}
			if (extra_bytes[0] != 0 || amount_of_extra_bytes > 1 && extra_bytes[1] != 0 || amount_of_extra_bytes > 2 && extra_bytes[2] != 0) {
				fprintf(stderr, "Some of extra bytes in row %d are not equal to zero\n", i);
				exit(-1);
			}
			current_buffer_pos += amount_of_extra_bytes;
		}
	}
}

void read_BMP_pixel_array_24bpp(FILE *file, struct BMP_t *picture) {
	int32_t abs_of_height = picture->height;
	if (abs_of_height < 0) {
		abs_of_height *= -1;
	}

	int32_t width_of_row_in_bytes = picture->width * 3;
	width_of_row_in_bytes += (4 - width_of_row_in_bytes % 4) % 4;
	
	picture->pixel_array_24bpp = (struct color_t **)(malloc(abs_of_height * sizeof(struct color_t *)));
	if (picture->pixel_array_24bpp == NULL) {
		fprintf(stderr, "Could not allocate memory for pixel array\n");
		exit(-2);
	}

	unsigned int buffer_size = abs_of_height * width_of_row_in_bytes;
	char *buffer = (char *)(malloc(buffer_size));
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory for pixel array\n");
		exit(-2);
	}
	int reading_result = fread(buffer, sizeof(char), buffer_size, file);
	if (reading_result != buffer_size) {
		if (feof(file) > 0) {
			fprintf(stderr, "Height and/or width do not match real file size\n");
			exit(-2);
		}
		fprintf(stderr, "Could not read pixel array");
		exit(-2);
	}
	char *current_buffer_pos = buffer;

	for (int32_t i = 0; i < abs_of_height; i++) {
		picture->pixel_array_24bpp[i] = (struct color_t *)(malloc(picture->width * sizeof(struct color_t)));
		if (picture->pixel_array_24bpp[i] == NULL) {
			fprintf(stderr, "Could not allocate memory for row %d of pixel array\n", i);
			exit(-2);
		}
		for (int32_t j = 0; j < picture->width; j++) {
			char color[BYTES_PER_PIXEL_24BPP + 1];
			reading_result = sscanf(current_buffer_pos, FORMAT_STRING[BYTES_PER_PIXEL_24BPP], color);
			if (reading_result != BYTES_PER_PIXEL_24BPP) {
				fprintf(stderr, "Could not read color of pixel with coords (%d, %d)\n", j, i);
				exit(-2);
			}
			picture->pixel_array_24bpp[i][j] = (struct color_t){ .blue = color[0], .green = color[1], .red = color[2] };
			current_buffer_pos += BYTES_PER_PIXEL_24BPP;
		}
		if (picture->width % 4 != 0) {
			uint32_t amount_of_extra_bytes = picture->width % 4;
			char extra_bytes[MAX_EXTRA_BYTES + 1];
			reading_result = sscanf(current_buffer_pos, FORMAT_STRING[amount_of_extra_bytes], extra_bytes);
			if (reading_result != amount_of_extra_bytes) {
				fprintf(stderr, "Could not read extra bytes in row %d\n", i);
				exit(-2);
			}
			if ((extra_bytes[0] != 0) || (amount_of_extra_bytes > 1 && extra_bytes[1] != 0) || (amount_of_extra_bytes == 3 && extra_bytes[2] != 0)) {
				fprintf(stderr, "Some of extra bytes in row %d are not equal to zero\n", i);
				exit(-1);
			}
			current_buffer_pos += amount_of_extra_bytes;
		}
	}
}

void read_BMP(char *filename, struct BMP_t *picture) {

	FILE *input_file = fopen(filename, "rb");
	if (input_file == NULL) {
		fprintf(stderr, "Could not open file %s\n", filename);
		exit(-2);
	}

	read_BMP_header(input_file, picture);
	
	if (picture->bit_count == BITS_PER_PIXEL_8BPP) {
		read_BMP_palette(input_file, picture);
		read_BMP_pixel_array_8bpp(input_file, picture);
	}

	else {
		read_BMP_pixel_array_24bpp(input_file, picture);
	}

	fclose(input_file);
}

void make_BMP_negative(struct BMP_t *picture) {
	if (picture->bit_count == 24) {
		int32_t abs_of_height = picture->height;
		if (abs_of_height < 0) {
			abs_of_height *= -1;
		}
		for (int i = 0; i < abs_of_height; i++) {
			for (int j = 0; j < picture->width; j++) {
				picture->pixel_array_24bpp[i][j] = convert_color_to_negative(picture->pixel_array_24bpp[i][j]);
			}
		}
	}
	else {
		uint32_t clr_used = picture->clr_used;
		if (clr_used == 0)
			clr_used = 256;
		for (uint32_t i = 0; i < clr_used; i++) {
			picture->color_table[i] = convert_color_to_negative(picture->color_table[i]);
		}
	}
}

void save_BMP_header(FILE *file, struct BMP_t *picture) {
	char buffer[BITMAPHEADER_SIZE], *current_buffer_pos = buffer;
	int writing_result;


	if (!picture->uses_little_endian) {
		writing_result = sprintf(current_buffer_pos, "%c%c", 0x4D, 0x42);
	}
	else {
		writing_result = sprintf(current_buffer_pos, "%c%c", 0x42, 0x4D);
	}
	if (writing_result != BFTYPE_SIZE) {
		fprintf(stderr, "Could not write file type to buffer\n");
		exit(-2);
	}
	current_buffer_pos += BFTYPE_SIZE;


	char f_size[BFSIZE_SIZE];
	convert_uint32_to_uchar_array(picture->f_size, f_size, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", f_size);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write file size to buffer\n");
		exit(-2);
	}
	current_buffer_pos += BFSIZE_SIZE;

	char reserved[] = {0, 0, 0, 0};
	writing_result = sprintf(current_buffer_pos, "%s", reserved);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write reserved bytes to buffer\n");
		exit(-2);
	}
	current_buffer_pos += BFRESERVED_SIZE;

	char off_bits[BFOFFBITS_SIZE];
	convert_uint32_to_uchar_array(picture->off_bits, off_bits, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", off_bits);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write offset to buffer\n");
		exit(-2);
	}
	current_buffer_pos += BFOFFBITS_SIZE;

	char size[SIZE_SIZE];
	convert_uint32_to_uchar_array(picture->size, size, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", size);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write infoheader size to buffer\n");
		exit(-2);
	}
	current_buffer_pos += SIZE_SIZE;

	char width[WIDTH_SIZE];
	convert_int32_to_uchar_array(picture->width, width, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", size);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write width to buffer\n");
		exit(-2);
	}
	current_buffer_pos += WIDTH_SIZE;

	char height[HEIGHT_SIZE];
	convert_int32_to_uchar_array(picture->height, height, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", height);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write height to buffer\n");
		exit(-2);
	}
	current_buffer_pos += HEIGHT_SIZE;

	char planes[PLANES_SIZE];
	convert_uint32_to_uchar_array(1, height, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", planes);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write planes to buffer\n");
		exit(-2);
	}
	current_buffer_pos += PLANES_SIZE;

	char bit_count[BITCOUNT_SIZE];
	convert_uint16_to_uchar_array(picture->bit_count, bit_count, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", bit_count);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write bit count to buffer\n");
		exit(-2);
	}
	current_buffer_pos += BITCOUNT_SIZE;

	char compression[COMPRESSION_SIZE];
	convert_uint32_to_uchar_array(picture->compression, compression, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", compression);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write compression to buffer\n");
		exit(-2);
	}
	current_buffer_pos += COMPRESSION_SIZE;

	char size_image[SIZEIMAGE_SIZE];
	convert_uint32_to_uchar_array(picture->size_image, size_image, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", size_image);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write pixel array size to buffer\n");
		exit(-2);
	}
	current_buffer_pos += SIZEIMAGE_SIZE;

	char x_pels_per_meter[XPELSPERMETER_SIZE];
	convert_int32_to_uchar_array(picture->x_pels_per_meter, x_pels_per_meter, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", x_pels_per_meter);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write horizontal resolution to buffer\n");
		exit(-2);
	}
	current_buffer_pos += XPELSPERMETER_SIZE;

	char y_pels_per_meter[YPELSPERMETER_SIZE];
	convert_int32_to_uchar_array(picture->y_pels_per_meter, y_pels_per_meter, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", y_pels_per_meter);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write vertical resolution to buffer\n");
		exit(-2);
	}
	current_buffer_pos += YPELSPERMETER_SIZE;

	char clr_used[CLRUSED_SIZE];
	convert_uint32_to_uchar_array(picture->clr_used, clr_used, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", clr_used);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write palette size to buffer\n");
		exit(-2);
	}
	current_buffer_pos += CLRUSED_SIZE;

	char clr_important[CLRIMPORTANT_SIZE];
	convert_uint32_to_uchar_array(picture->clr_important, clr_important, picture->uses_little_endian);
	writing_result = sprintf(current_buffer_pos, "%s", clr_important);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write important palette size to buffer\n");
		exit(-2);
	}
	current_buffer_pos += CLRIMPORTANT_SIZE;

	writing_result = fprintf(file, "%s", buffer);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write header to file\n");
		exit(-2);
	}
}

void save_BMP_pixel_array_24bpp(FILE *file, struct BMP_t *picture) {
	int32_t abs_of_height = picture->height;
	if (abs_of_height < 0) {
		abs_of_height *= -1;
	}

	int32_t width_of_row_in_bytes = picture->width;
	uint32_t amount_of_extra_bytes = picture->width % 4;
	if (amount_of_extra_bytes != 0) {
		width_of_row_in_bytes += amount_of_extra_bytes;
	}
	char *buffer = (char *)(malloc(abs_of_height * width_of_row_in_bytes * sizeof(char)));
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory for buffer with pixel array\n");
		exit(-2);
	}
	char *current_buffer_pos = buffer;

	int writing_result;
	for (int32_t i = 0; i < abs_of_height; i++) {
		for (int32_t j = 0; j < picture->width; j++) {
			char color[] = { picture->pixel_array_24bpp[i][j].blue, picture->pixel_array_24bpp[i][j].green, picture->pixel_array_24bpp[i][j].red };
			writing_result = sprintf(current_buffer_pos, "%s", color);
			if (writing_result != 1) {
				fprintf(stderr, "Could not write pixel with coords (%d, %d) to buffer\n", j, i);
				exit(-2);
			}
			current_buffer_pos += BYTES_PER_PIXEL_24BPP;
		}
		for (int32_t j = 0; j < amount_of_extra_bytes; j++) {
			writing_result = sprintf(current_buffer_pos, "%c", 0);
			if (writing_result != 1) {
				fprintf(stderr, "Could not write extra byte #%d in row #%d to buffer\n", j + 1, i);
				exit(-2);
			}
			current_buffer_pos += 1;
		}
	}

	writing_result = fprintf(file, "%s", buffer);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write pixel array to file\n");
		exit(-2);
	}

	free(buffer);
}

void save_BMP_palette(FILE *file, struct BMP_t *picture) {
	if (picture->clr_used == 0) {
		picture->clr_used = MAX_COLORS_IN_PALETTE;
	}

	char *buffer = (char*)(malloc(picture->clr_used * sizeof(char)));
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory for buffer with color palette\n");
		exit(-2);
	}
	char *current_buffer_pos = buffer;
	int writing_result;

	for (uint32_t i = 0; i < picture->clr_used; i++) {
		char color[] = { picture->color_table[i].blue, picture->color_table[i].green, picture->color_table[i].red, 0 };
		writing_result = sprintf(current_buffer_pos, "%s", color);
		if (writing_result != 1) {
			fprintf(stderr, "Could not write color #%d to buffer\n", i + 1);
			exit(-2);
		}
		current_buffer_pos += BYTES_PER_COLOR_IN_TABLE;
	}

	writing_result = fprintf(file, "%s", buffer);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write color palette to file\n");
		exit(-2);
	}

	free(buffer);
}

void save_BMP_pixel_array_8bpp(FILE *file, struct BMP_t *picture) {
	int32_t abs_of_height = picture->height;
	if (abs_of_height < 0) {
		abs_of_height *= -1;
	}
	int32_t amount_of_extra_bytes = 4 - picture->width % 4, width_of_row_in_bytes = picture->width;

	char *buffer = (char *)(malloc(abs_of_height * width_of_row_in_bytes *sizeof(char)));
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory for buffer with pixel array\n");
		exit(-2);
	}
	char *current_buffer_pos = buffer;
	int writing_result;

	for (int32_t i = 0; i < abs_of_height; i++) {
		for (int32_t j = 0; j < picture->width; j++) {
			char color_number = picture->pixel_array_8bpp[i][j];
			writing_result = sprintf(current_buffer_pos, "%c", color_number);
			if (writing_result != BYTES_PER_PIXEL_8BPP) {
				fprintf(stderr, "Could not write pixel with coords (%d, %d) to buffer\n", j, i);
				exit(-2);
			}
			current_buffer_pos += BYTES_PER_PIXEL_8BPP;
		}
		for (int32_t j = 0; j < amount_of_extra_bytes; j++) {
			writing_result = sprintf(current_buffer_pos, "%c", 0);
			if (writing_result != 1) {
				fprintf(stderr, "Could not write extra byte #%d in row #%d to buffer\n", j + 1, i);
				exit(-2);
			}
			current_buffer_pos += 1;
		}
	}

	writing_result = fprintf(file, "%s", buffer);
	if (writing_result != 1) {
		fprintf(stderr, "Could not write pixel array to file\n");
		exit(-2);
	}

	free(buffer);
}

void save_BMP(char *filename, struct BMP_t *picture) {
	FILE *output_file = fopen(filename, "wb");
	save_BMP_header(output_file, picture);
	if (picture->bit_count == BITS_PER_PIXEL_24BPP) {
		save_BMP_pixel_array_24bpp(output_file, picture);
	}

	else {
		save_BMP_palette(output_file, picture);
		save_BMP_pixel_array_8bpp(output_file, picture);
	}

	fclose(output_file);
}

void free_BMP(struct BMP_t *picture) {
	if (picture->bit_count == 24) {
		int32_t abs_of_height = picture->height;
		if (abs_of_height < 0) {
			abs_of_height *= -1;
		}
		for (int32_t i = 0; i < abs_of_height; i++) {
			free(picture->pixel_array_24bpp[i]);
		}
		free(picture->pixel_array_24bpp);
	}
	else {
		free(picture->color_table);

		int32_t abs_of_height = picture->height;
		if (abs_of_height < 0) {
			abs_of_height *= -1;
		}
		for (int32_t i = 0; i < abs_of_height; i++) {
			free(picture->pixel_array_8bpp[i]);
		}
		free(picture->pixel_array_8bpp);
	}
}

struct color_t get_pixel_color(struct BMP_t *picture, int32_t coord_by_width, int32_t coord_by_height) {
	if (picture->height < 0) {
		coord_by_height = -picture->height - coord_by_height - 1;
	}
	if (picture->bit_count == 24) {
		return picture->pixel_array_24bpp[coord_by_height][coord_by_width];
	}
	else {
		return picture -> color_table[ picture->pixel_array_8bpp[coord_by_height][coord_by_width] ];
	}
}

int compare_BMP(struct BMP_t *picture1, struct BMP_t *picture2, struct coord_t *coords_of_mismatching_pixels) {
	if (picture1->width != picture2->width) {
		fprintf(stderr, "The width of the first picture is not equal to the width of the second picture\n");
		exit(-1);
	}
	
	if (picture1->height != picture2->height && picture1->height != -picture2->height) {
		fprintf(stderr, "The absolute value of height of the first picture is not equal to the absolute value of height of the second picture\n");
		exit(-1);
	}

	uint8_t number_of_mismatching_pixels = 0;
	int32_t abs_of_height = picture1->height;
	if (abs_of_height < 0) {
		abs_of_height *= -1;
	}

	for (int32_t i = 0; i < abs_of_height; i++) {
		for (int32_t j = 0; j < picture1->width; j++) {
			if (!check_if_colors_are_equal(get_pixel_color(picture1, j, i), get_pixel_color(picture2, j, i))) {
				if (number_of_mismatching_pixels < 100) {
					coords_of_mismatching_pixels[number_of_mismatching_pixels] = (struct coord_t){ .by_height = i, .by_width = j };
					number_of_mismatching_pixels++;
				}
			}
		}
	}

	return number_of_mismatching_pixels;
}