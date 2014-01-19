class EigenSolver
{
public:
    enum Method
    {
        LLT,
        LDLT,
        //LU, //Inferior to LLT for symmetric problems
        QR, //Extremely slow
        ConjugateGradient_Diag,
        //ConjugateGradient_LUT, //Not a valid solver
        BiCGSTAB_Diag,
        BiCGSTAB_LUT,
        Profile,
    };

    Vector<double> Solve(const SparseMatrix<double> &M, const Vector<double> &b, Method method);
    Vector<double> Solve(const Eigen::SparseMatrix<double> &M, const Vector<double> &b, Method method);

private:
    static String GetMethodName(Method m)
    {
        switch(m)
        {
            case LLT: return "LLT";
            case LDLT: return "LDLT";
            //case LU: return "LU";
            case QR: return "QR";
            case ConjugateGradient_Diag: return "ConjugateGradient_Diag";
            //case ConjugateGradient_LUT: return "ConjugateGradient_LUT";
            case BiCGSTAB_Diag: return "BiCGSTAB_Diag";
            case BiCGSTAB_LUT: return "BiCGSTAB_LUT";
            case Profile: return "Profile";
            default: return "Unknown";
        }
    }
};
