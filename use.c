#include "chatmat.h"

int main(void) {
	Matrix mat = {
		.rows = 3,
		.cols = 3,
		.data = (double *[]){
			(double[]){1,0,0},
			(double[]){0,1,0},
			(double[]){0,0,1}
		}
	};

	matrix_print(&mat);

	return 0;
}
