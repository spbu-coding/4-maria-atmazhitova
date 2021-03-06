#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "custom_bmp.h"

void parse_arguments(int argc, char *argv[], char **files_names) {
	if (argc < 3) {
		fprintf(stderr, "Not enough command prompt arguments\n");
		exit(-2);
	}
	if (argc > 3) {
		fprintf(stderr, "Too many command prompt arguments\n");
		exit(-2);
	}
	for (int i = 0; i < 2; i++) {
		if (!check_if_string_ends_with(argv[i + 1], ".bmp") && !check_if_string_ends_with(argv[i + 1], ".BMP")) {
			fprintf(stderr, "Wrong name of file #%d, expected a bmp file\n", i + 1);
			exit(-2);
		}
		files_names[i] = argv[i + 1];
	}
}

int main(int argc, char *argv[]) {
	char *files_names[2];
	parse_arguments(argc, argv, files_names);
	struct BMP_t pictures[2];
	for (int i = 0; i < 2; i++) {
		read_BMP(files_names[i], &pictures[i]);
	}

	struct coord_t coords_of_mismatching_pixels[100];
	int number_of_mismatching_pixels = compare_BMP(&pictures[0], &pictures[1], coords_of_mismatching_pixels);
	for (int i = 0; i < 2; i++) {
		free_BMP(&pictures[i]);
	}

	if (number_of_mismatching_pixels == 0) {
		return 0;
	}

	char *buffer = malloc(1024 * sizeof(char)), *pointer_to_buffer_pos = buffer;
	for (int i = 0; i < number_of_mismatching_pixels; i++) {
		pointer_to_buffer_pos += sprintf(pointer_to_buffer_pos, "%d %d\n", coords_of_mismatching_pixels[i].by_width, coords_of_mismatching_pixels[i].by_height);
	}
	fprintf(stderr, "%s", buffer);
	free(buffer);
	return number_of_mismatching_pixels;
}