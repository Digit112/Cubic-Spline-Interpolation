#include <stdio.h>

#include "splines.h"

int main() {
	double points[] = {0, 2, 1, 8, 4, 4, 5, 4, 7, 6};
	int num_points = 5;
	
	double** mat = allocate_matrix(num_points);
	
	load_matrix(mat, num_points, points, NATURAL);
	
	gaussian_elimination(mat, num_points);
	int err = back_substitution(mat, num_points);
	
	if (err == 1) {
		printf("Could not solve.\n");
		print_matrix(mat, num_points);
	}
	else {
		print_matrix(mat, num_points);
		printf("\n");
		print_RHS(mat, num_points);
	}
	
	free_matrix(mat, num_points);
	
	return 0;
}