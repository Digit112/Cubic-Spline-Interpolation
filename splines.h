#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NATURAL 0
#define PERIODIC 1
#define QUADRATIC 2
#define NOTAKNOT 3

double** allocate_matrix(int num_points) {
	int N = num_points * 4 - 4;
	
	// Allocate matrix. Add and extra column for the RHS.
	double** mat = (double**) malloc(sizeof(double*) * N);
	for (int i = 0; i < N; i++) {
		mat[i] = malloc(sizeof(double) * (N+1));
	}
	
	return mat;
}

// Accepts the list of num_points points as an array of 2*num_points doubles.
// points[2*i] = x_i, points[2*i+1] = y_i
void load_matrix(double** mat, int num_points, double* points, int boundary) {
	int N = num_points * 4 - 4;
	
	for (int i = 0; i < N; i++) {
		for (int j = 0; j <= N; j++) {
			mat[i][j] = 0;
		}
	}
	
	// Split the x and y values into separate arrays, which makes the array accesses a bit simpler down the line.
	double X[num_points];
	double Y[num_points];
	for (int i = 0; i < num_points; i++) {
		X[i] = points[i*2];
		Y[i] = points[i*2+1];
	}
	
	// Initialize matrix.
	// Calculate and cache the squares and cubes of each of the x values.
	double squares[num_points];
	double cubes[num_points];
	for (int i = 0; i < num_points; i++) {
		squares[i] = X[i] * X[i];
		cubes[i] = X[i] * squares[i];
	}
	
	// The row we're currently packing an equation into.
	// This will be incremented every time we move to another row.
	int r = 0;
	
	// Restrict functions by a pair of points that they must pass through.
	for (int i = 0; i < num_points-1; i++) {
		// a_i * x_i^3 + b_i * x_i^2 + c_i * x_i + d_i = y_i
		mat[r][i*4  ] = cubes[i];
		mat[r][i*4+1] = squares[i];
		mat[r][i*4+2] = X[i];
		mat[r][i*4+3] = 1;
		mat[r][N] = Y[i];
		
		r++;
		
		// a_i * x_(i+1)^3 + b_i * x_(i+1)^2 + c_i * x_(i+1) + d_i = y_(i+1)
		mat[r][i*4  ] = cubes[i+1];
		mat[r][i*4+1] = squares[i+1];
		mat[r][i*4+2] = X[i+1];
		mat[r][i*4+3] = 1;
		mat[r][N] = Y[i+1];
		
		r++;
	}
	
	// Restrict adjacent segments to have the same first derivative where they meet.
	for (int i = 0; i < num_points-2; i++) {
		// 3 * a_i * x_(i+_1)^2 + 2 * b_i * x_(i+1) + c_i  -  3 * a_(i+1) * x_(i+1)^2 - 2 * b_(i+1) * x_(i+1) - c_(i+1) = 0
		mat[r][i*4  ] = 3 * squares[i+1];
		mat[r][i*4+1] = 2 * X[i+1];
		mat[r][i*4+2] = 1;
		
		mat[r][(i+1)*4  ] = -3 * squares[i+1];
		mat[r][(i+1)*4+1] = -2 * X[i+1];
		mat[r][(i+1)*4+2] = -1;
		
		r++;
	}
	
	// Restrict adjacent segments to have the same second derivative where they meet.
	for (int i = 0; i < num_points-2; i++) {
		// 6 * a_i * x_(i+1) + 2 * b_i  -  6 * a_(i+1) * x_(i+1) - 2 * b_(i+1) = 0;
		mat[r][i*4  ] = 6 * X[i+1];
		mat[r][i*4+1] = 2;
		
		mat[r][(i+1)*4  ] = -6 * X[i+1];
		mat[r][(i+1)*4+1] = -2;
		
		r++;
	}
	
	// Restrict the second derivative at the first and last points.
	if (boundary == NATURAL) {
		// 6 * a_1 * x_1 + 2 * b_1 = 0
		mat[r][0] = 6 * X[0];
		mat[r][1] = 2;
		
		r++;
		
		// 6 * a_(n-1) * x_n + 2 * b_(n-1) = 0
		mat[r][(num_points-2)*4  ] = 6 * X[num_points-1];
		mat[r][(num_points-2)*4+1] = 2;
	}
	else if (boundary == PERIODIC) {
		// 3 * a_1 * x_1^2 + 2 * b_1 * x_1 + c_1  -  3 * a_(n-1) * x_n^2 - 2 * b_(n-1) * x_n - c_(n-1) = 0
		mat[r][0] = 3 * squares[0];
		mat[r][1] = 2 * X[0];
		mat[r][2] = 1;
		
		mat[r][(num_points-2)*4  ] = -3 * squares[num_points-1];
		mat[r][(num_points-2)*4+1] = -2 * X[num_points-1];
		mat[r][(num_points-2)*4+2] = -1;
		
		r++;
		
		// 6 * a_1 * x_1 + 2 * b_1  -  6 * a_(n-1) * x_n - 2 * b_(n-1) = 0
		mat[r][0] = 6 * X[1];
		mat[r][1] = 2;
		
		mat[r][(num_points-2)*4  ] = -6 * X[num_points-1];
		mat[r][(num_points-2)*4+1] = -2;
	}
	else if (boundary == QUADRATIC) {
		// a_1 = 0
		mat[r][0] = 1;
		
		r++;
		
		// a_(n-1) = 0;
		mat[r][(num_points-2)*4] = 1;
	}
	else if (boundary == NOTAKNOT) {
		// a_1 = a_2
		mat[r][0] = 1;
		mat[r][4] = -1;
		
		r++;
		
		// a_(n-2) = a_(n-1)
		mat[r][(num_points-3)*4] = 1;
		mat[r][(num_points-2)*4] = -1;
	}
}

void print_matrix(double** mat, int num_points) {
	int N = num_points * 4 - 4;
	
	for (int j = 0; j < N; j++) {
		printf("%7d ", j);
	}
	printf("\n");
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%7.3lf ", mat[i][j]);
		}
		printf("| %7.3lf\n", mat[i][N]);
	}
}

// Print the right-hand side of the matrix.
// These values can be copy-pasted into Desmos.
void print_RHS(double** mat, int num_points) {
	int N = num_points * 4 - 4;
	
	for (int i = 0; i < N; i++) {
		printf("%10.4lf\n", mat[i][N]);
	}
}

// Swap two rows in the matrix
void swap_rows(double** mat, int i, int j) {
	double* temp = mat[i];
	mat[i] = mat[j];
	mat[j] = temp;
}

// Perform Gaussian elimination to bring the matrix into row echelon form.
// The matrix is not put into reduced row echelon form.
void gaussian_elimination(double** mat, int num_points) {
	int N = num_points * 4 - 4;
	
	// Iterate over the diagonal.
	for (int k = 0; k < N; k++) {
		// Find the largest entry among the entries beneath k.
		double max_i = fabs(mat[k][k]);
		int max_i_row = k;
		
		for (int i = k+1; i < N; i++) {
			if (fabs(mat[i][k]) > max_i) {
				max_i = fabs(mat[i][k]);
				max_i_row = i;
			}
		}
		
		// If the largest entry is 0, the system of equations either has no solutions or infinitely many solutions.
		if (max_i == 0) {
			continue;
		}
		
		// If a larger potential k value exists, swap the two rows.
		if (max_i_row != k) {
			swap_rows(mat, k, max_i_row);
		}
		
		// Add multiples of the kth row to each of the below columns in order to make all of the entries beneath k 0.
		for (int i = k+1; i < N; i++) {
			double m = -mat[i][k] / mat[k][k];
			
			for (int j = 0; j <= N; j++) {
				mat[i][j] += m * mat[k][j];
			}
		}
	}
}

// Solve for the unknowns in reverse order. The results are placed in column N in the RHS.
int back_substitution(double** mat, int num_points) {
	int N = num_points * 4 - 4;
	
	// Solve from bottom to top
	for (int i = N-1; i >= 0; i--) {
		// Subtract all previous terms.
		for (int j = N-1; j > i; j--) {
			mat[i][N] -= mat[j][N] * mat[i][j];
			mat[i][j] = 0; // unnecessary
		}
		
		// If the leading entry and right hand side are both zero, then the equation is
		// something to the effect of 0ax = 0 which of course means that a can be anything.
		// Therefore, there are infinite solutions. In this case, we assume the this unknown is equal to 0.
		if (mat[i][i] == 0) {
			if (mat[i][N] == 0) {
				mat[i][N] = 0; // Assume value of unknown that can be anything.
			}
			else {
				// equation is like 0ax = 1 (no solutions).
				printf("%lf * a = %lf\n", mat[i][i], mat[i][N]);
				return 1;
			}
		}
		else {
			mat[i][N] /= mat[i][i];
			mat[i][i] = 0; // unnecessary
		}
	}
	
	return 0;
}

void free_matrix(double** mat, int num_points) {
	int N = num_points * 4 - 4;
	
	for (int i = 0; i < N; i++) {
		free(mat[i]);
	}
	free(mat);
}