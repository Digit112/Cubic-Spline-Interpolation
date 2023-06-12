#include <stdlib.h>
#include <stdio.h>

#include "splines.h"

/*
	Generates random, arbitrarily long, approximated splines.
	
	Generates total_num_points random points and calculates the spline passing through all of them.
	To facilitate extremely long splines with memory restrictions in mind, this program calculates many small, overlapping splines, and uses only the middle segment of each.
*/

int main() {
	int total_num_points = 32;
	
	// When generating one segment, also generate and discard this many segments on either side.
	// Higher values rapidly increase the accuracy of the approximation.
	int buffer_splines = 4;
	
	// Position of the first point.
	double init_x = 0;
	double init_y = 0.5;
	
	// Min and max y values for randomly generated points.
	double min_y = -100;
	double max_y = 100;
	
	// Min and max difference in the y value of two adjacent points.
	double min_y_sep = -0.4;
	double max_y_sep = 0.4;
	
	// Min and max difference in the x value of two adjacent points.
	double min_x_sep = 0.2;
	double max_x_sep = 0.4;
	
	srand(3);
	
	/* ---- End Parameters ---- */
	
	// Error checking.
	if (min_y > max_y)
		printf("min_y must be less than or equal to max_y.\n");
	
	if (min_y_sep > max_y_sep)
		printf("min_y_sep must be less than or equal to max_y_sep.\n");
	
	if (min_x_sep > max_x_sep)
		printf("min_x_sep must be less than or equal to max_x_sep.\n");
	
	if (min_x_sep <= 0)
		printf("Because min_x_sep is not positive, the generated spline may not be a function.\n");
	
	// List of all points to generate splines passing through them.
	double all_points[total_num_points * 2];
	
	// Generate all points.
	all_points[0] = init_x;
	all_points[1] = init_y;
	
	for (int i = 1; i < total_num_points; i++) {
		double tmp_min_y = min_y;
		double tmp_max_y = max_y;
		
		if (all_points[i*2 - 1] + min_y_sep > tmp_min_y) {
			tmp_min_y = all_points[i*2 - 1] + min_y_sep;
		}
		if (all_points[i*2 - 1] + max_y_sep < tmp_max_y) {
			tmp_max_y = all_points[i*2 - 1] + max_y_sep;
		}
		
		double tmp_min_x = all_points[i*2 - 2] + min_x_sep;
		double tmp_max_x = all_points[i*2 - 2] + max_x_sep;
		
		if (tmp_max_y < tmp_min_y) {
			// To my knowledge, this can only happen if the min/max y parameters are reversed
			// Or if the initial y-point is too far outside the range of valid y values.
			printf("Cannot generate points. No value of y would satisfy the requirements.\n");
			return 1;
		}
		
		all_points[i*2  ] = (double) rand() / RAND_MAX * (tmp_max_x - tmp_min_x) + tmp_min_x;
		all_points[i*2+1] = (double) rand() / RAND_MAX * (tmp_max_y - tmp_min_y) + tmp_min_y;
	}
	
	// for (int i = 0; i < total_num_points; i++) {
		// printf("%5.2lf, %5.2lf\n", all_points[i*2], all_points[i*2+1]);
	// }
	
	int num_points = buffer_splines * 2 + 2;
	int N = num_points * 4 - 4;
	
	FILE* fout = fopen("spline.txt", "w");
	if (fout == NULL) {
		printf("Could not open file for writing spline.\n");
		return 1;
	}
	
	char buf[64];
	int buf_n;
	
	buf_n = sprintf(buf, "%lf %lf\n", all_points[0], all_points[1]);
	fwrite(buf, 1, buf_n, fout);
	
	double** mat = allocate_matrix(num_points);
	
	for (int i = 1; i < total_num_points; i++) {
		// Get the index of the first point to send to get the spline.
		int start_pnt_i = i - buffer_splines - 1;
		if (start_pnt_i < 0) {
			start_pnt_i = 0;
		}
		
		if (start_pnt_i + num_points - 1 >= total_num_points) {
			start_pnt_i = total_num_points - num_points;
		}
		
		load_matrix(mat, num_points, all_points + start_pnt_i * 2, NOTAKNOT);
		
		int err = gaussian_elimination(mat, num_points);
		
		// This should never happen because ANY sequence of points should always have one valid solution.
		if (err == 1) {
			printf("Could not find a solution.\n");
			free_matrix(mat, num_points);
			fclose(fout);
			return 1;
		}
		
		back_substitution(mat, num_points);
		
		int coef_i = (i - start_pnt_i - 1) * 4;
		
		buf_n = sprintf(buf, "%lf %lf %lf %lf\n", mat[coef_i][N], mat[coef_i+1][N], mat[coef_i+2][N], mat[coef_i+3][N]);
		fwrite(buf, 1, buf_n, fout);
		
		buf_n = sprintf(buf, "%lf %lf\n", all_points[i*2], all_points[i*2+1]);
		fwrite(buf, 1, buf_n, fout);
	}
	
	fclose(fout);
	
	free_matrix(mat, num_points);
	
	return 0;
}