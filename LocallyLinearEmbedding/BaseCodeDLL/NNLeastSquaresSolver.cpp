#include "Main.h"
#include "NNLeastSquaresSolver.h"


NNLeastSquaresSolver::NNLeastSquaresSolver(void)
{
}


NNLeastSquaresSolver::~NNLeastSquaresSolver(void)
{
}

Vector<double> NNLeastSquaresSolver::Solve(const SparseMatrix<double> &A, const Vector<double> &b)
{
	PersistentAssert(A.RowCount() == b.Length(), "NNLeastSquaresSolver: Mismatched dimensions!");

	int mda = A.RowCount();
	int m = A.RowCount();
	int n = A.ColCount();

	double* nnls_a = new double[m*n];
	double* nnls_b = new double[m];

	double* x = new double[n];

	//populate
	for (int r=0; r<m; r++)
		for (int c=0; c<n; c++)
			nnls_a[c*m+r] = A.GetElement(r,c);

	for (int r=0; r<m; r++)
		nnls_b[r] = b[r];
	
	//temporary vars for nnls
	double* w = new double[n];
	double* zz = new double[m];
	int* indx = new int[n];
	double rnorm;
	int mode;

	nnls(nnls_a, mda, m, n, nnls_b, x, &rnorm, w, zz, indx, &mode);

	if (mode == 1)
		Console::WriteLine("Solve succeeded! Error: " + String(rnorm));
	else if (mode == 2)
		Console::WriteLine("Bad dimensions");
	else if (mode == 3)
		Console::WriteLine("Iteration count exceeded");

	//cleanup
	delete[] nnls_a;
	delete[] nnls_b;
	delete[] x;
	delete[] w;
	delete[] zz;
	delete[] indx;

	Vector<double> result(n);
	for (int i=0; i<n; i++)
		result[i] = x[i];
	return result;

}

Vector<double> NNLeastSquaresSolver::Solve(const DenseMatrix<double> &A, const Vector<double> &b)
{
	PersistentAssert(A.RowCount() == b.Length(), "NNLeastSquaresSolver: Mismatched dimensions!");

	int mda = A.RowCount();
	int m = A.RowCount();
	int n = A.ColCount();

	double* nnls_a = new double[m*n];
	double* nnls_b = new double[m];

	double* x = new double[n];

	//populate
	for (int r=0; r<m; r++)
		for (int c=0; c<n; c++)
			nnls_a[c*m+r] = A(r,c);

	for (int r=0; r<m; r++)
		nnls_b[r] = b[r];
	
	//temporary vars for nnls
	double* w = new double[n];
	double* zz = new double[m];
	int* indx = new int[n];
	double rnorm;
	int mode;

	nnls(nnls_a, mda, m, n, nnls_b, x, &rnorm, w, zz, indx, &mode);

	if (mode == 1)
		Console::WriteLine("Solve succeeded! Error: " + String(rnorm));
	else if (mode == 2)
		Console::WriteLine("Bad dimensions");
	else if (mode == 3)
		Console::WriteLine("Iteration count exceeded");

	//cleanup
	delete[] nnls_a;
	delete[] nnls_b;
	delete[] x;
	delete[] w;
	delete[] zz;
	delete[] indx;

	Vector<double> result(n);
	for (int i=0; i<n; i++)
		result[i] = x[i];
	return result;

}
