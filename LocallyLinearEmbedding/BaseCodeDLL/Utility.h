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
    Eigen::VectorXd MakeEigenVector(const Vector<T> &v)
    {
        const UINT n = v.Length();
        Eigen::VectorXd result(n);
        for(UINT i = 0; i < n; i++) result[i] = v[i];
        return result;
    }

    __forceinline Vector<double> DumpEigenVector(const Eigen::VectorXd &v)
    {
        const UINT n = (UINT)v.size();
        Vector<double> result(n);
        for(UINT i = 0; i < n; i++) result[i] = v[i];
        return result;
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

    __forceinline Bitmap MakePaletteOverlay(const Bitmap &bmp)
    {
        const UINT buffer = 3;
        const UINT size = 50 * 1280 / 960;
        Vector<RGBColor> colors;
        for(UINT y = 0; y < bmp.Height(); y++)
            for(UINT x = 0; x < bmp.Width(); x++)
            {
                RGBColor c = bmp[y][x];
                if(c != RGBColor::Black && !colors.Contains(c)) colors.PushEnd(c);
            }
        Bitmap result(size, size * colors.Length(), RGBColor::Black);
        for(UINT colorIndex = 0; colorIndex < colors.Length(); colorIndex++)
        {
            for(UINT y = buffer; y < size - buffer; y++)
            {
                for(UINT x = buffer; x < size - buffer; x++)
                {
                    result[y + colorIndex * size][x] = colors[colorIndex];
                }
            }
        }
        return result;
    }
};