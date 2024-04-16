#include "chatmat.h"

int main(void) {
	Matrix mat = {
		.rows = 3,
		.cols = 2,
		.transpose = 0,
		.data = (double *[]){
			(double[]){1,2},
			(double[]){4,5},
			(double[]){7,8}
		}
	};

	matrix_print(&mat);
	mat.transpose = 1;
	matrix_print(&mat);

	return 0;
}
