#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "qdbmp_negative.h"
#include "custom_bmp.h"

bool parse_arguments(int argc, char **argv[], bool *use_their_realization, char **input_file_name, char **output_file_name) {
	if (argc < 4) {
		fprintf(stderr, "Not enough command prompt arguments\n");
		return 0;
	}
	if (argc > 4) {
		fprintf(stderr, "Too many command prompt arguments\n");
		return 0;
	}
	bool theirs_or_mine_is_given = 0, input_file_name_is_given = 0, output_file_name_is_given = 0;
	char *mine_argument = "--mine", *theirs_argument = "--theirs";
	for (int i = 1; i <= 3; i++) {
		if (strcmp((*argv)[i], mine_argument) == 0) {
			if (!theirs_or_mine_is_given) {
				theirs_or_mine_is_given = 1;
				*use_their_realization = 0;
			}
			else if (!(*use_their_realization)) {
				fprintf(stderr, "--mine argument is given too many times\n");
				return 0;
			}
			else {
				fprintf(stderr, "--theirs and --mine arguments are both given\n");
				return 0;
			}
		}
		else if (strcmp((*argv)[i], theirs_argument) == 0) {
			if (!theirs_or_mine_is_given) {
				theirs_or_mine_is_given = 1;
				*use_their_realization = 1;
			}
			else if (!(*use_their_realization)) {
				fprintf(stderr, "--theirs and --mine arguments are both given\n");
				return 0;
			}
			else {
				fprintf(stderr, "--theirs argument is given too many times\n");
				return 0;
			}
		}
		else if (check_if_string_ends_with((*argv)[i], ".bmp") || check_if_string_ends_with((*argv)[i], ".BMP")) {
			if (!input_file_name_is_given) {
				*input_file_name = (*argv)[i];
				input_file_name_is_given = 1;
			}
			else if (!output_file_name_is_given) {
				*output_file_name = (*argv)[i];
				output_file_name_is_given = 1;
			}
			else {
				fprintf(stderr, "Too many file names in command prompt arguments\n");
				return 0;
			}
		}
		else {
			fprintf(stderr, "Incorrect command prompt argument: %s\n", (*argv)[i]);
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[]) {
	char *input_file_name = "", *output_file_name = "";
	bool use_their_realization = 0, arguments_are_correct = parse_arguments(argc, &argv, &use_their_realization, &input_file_name, &output_file_name);
	if (!arguments_are_correct) {
		return -4;
	}
	if (use_their_realization) {
		char *args[] = {"qdbmp_negative", input_file_name, output_file_name};
		if (negative_by_qdbmp(3, args) != 0) {
			fprintf(stderr, "Something went wrong while converting to negative with qdbmp library\n");
			return -3;
		}
		return 0;
	}
	struct BMP_t picture;
	read_BMP(input_file_name, &picture);
	make_BMP_negative(&picture);
	save_BMP(output_file_name, &picture);
	free_BMP(&picture);
    return 0;
}