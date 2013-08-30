namespace Utility
{
    __forceinline void AddMaskConstraints(const Bitmap &bmp, const Bitmap &mask, Vector<PixelConstraint> &constraints)
    {
        for(UINT y = 0; y < bmp.Height(); y++)
        {
            for(UINT x = 0; x < bmp.Width(); x++)
            {
                if(bmp[y][x] != mask[y][x])
                {
                    if(mask[y][x] == RGBColor::Magenta)
                    {
                        constraints.PushEnd(PixelConstraint(Vec2i(x, y), Vec3f(bmp[y][x])));
                    }
                    else
                    {
                        constraints.PushEnd(PixelConstraint(Vec2i(x, y), Vec3f(mask[y][x])));
                    }
                }
            }
        }
    }

    template<class T>
    Vector<Eigen::Triplet<T> > MakeEigenTriplets(const SparseMatrix<T> &M)
    {
        Vector<Eigen::Triplet<T> > triplets;
        for(UINT rowIndex = 0; rowIndex < M.RowCount(); rowIndex++)
        {
            const SparseRow<double> &row = M.Rows()[rowIndex];
            for(UINT colIndex = 0; colIndex < row.Data.Length(); colIndex++)
            {
                triplets.PushEnd(Eigen::Triplet<double>(rowIndex, row.Data[colIndex].Col, row.Data[colIndex].Entry));
            }
        }
        return triplets;
    }

    template<class T>
    Vector<double> EigenSolve(const Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > &choleskyFactorization, const Vector<T> &b)
    {
        const UINT n = b.Length();
        Eigen::VectorXd bEigen(n);

        for(UINT i = 0; i < n; i++) bEigen[i] = b[i];
        
        Eigen::VectorXd x = choleskyFactorization.solve(bEigen);
        
        Vector<double> result(n);
        for(UINT i = 0; i < n; i++) result[i] = x[i];
        return result;
    }

    template<class T>
    Vector<double> EigenSolve(const Eigen::SimplicialCholesky< Eigen::SparseMatrix<double> > &choleskyFactorization, const Vector<T> &b)
    {
        const UINT n = b.Length();
        Eigen::VectorXd bEigen(n);

        for(UINT i = 0; i < n; i++) bEigen[i] = b[i];
        
        Eigen::VectorXd x = choleskyFactorization.solve(bEigen);
        
        Vector<double> result(n);
        for(UINT i = 0; i < n; i++) result[i] = x[i];
        return result;
    }

    template<class T>
    void DumpSparsityPattern(const SparseMatrix<T> &M)
    {
        for(UINT rowIndex = 0; rowIndex < M.RowCount(); rowIndex++)
        {
            const SparseRow<double> &row = M.Rows()[rowIndex];
            for(UINT colIndex = 0; colIndex < row.Data.Length(); colIndex++)
            {
                //
            }
        }
    }
};