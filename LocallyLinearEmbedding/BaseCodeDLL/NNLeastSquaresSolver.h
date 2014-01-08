#pragma once
class NNLeastSquaresSolver
{
public:
	NNLeastSquaresSolver(void);
	~NNLeastSquaresSolver(void);

	Vector<double> Solve(const SparseMatrix<double> &M, const Vector<double> &b);
	Vector<double> Solve(const DenseMatrix<double> &M, const Vector<double> &b);
};

