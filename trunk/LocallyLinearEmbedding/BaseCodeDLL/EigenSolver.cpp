#include "Main.h"

Vector<double> EigenSolver::Solve(const SparseMatrix<double> &M, const Vector<double> &b, Method method)
{
    Console::WriteLine("Loading Eigen matrix...");
    Eigen::SparseMatrix<double> eigenMatrix(M.RowCount(), M.ColCount());
    auto triplets = Utility::MakeEigenTriplets(M);
    eigenMatrix.setFromTriplets(triplets.begin(), triplets.end());
    return Solve(eigenMatrix, b, method);
}

Vector<double> EigenSolver::Solve(const Eigen::SparseMatrix<double> &M, const Vector<double> &b, Method method)
{
    Console::WriteLine("Solving using method: " + GetMethodName(method));
    Clock timer;

    const Eigen::VectorXd bEigen = Utility::MakeEigenVector(b);
    Eigen::VectorXd x;
    
    if(method == LLT)
    {
        Eigen::SimplicialLLT< Eigen::SparseMatrix<double> > factorization(M);
        x = factorization.solve(bEigen);
    }
    else if(method == LDLT)
    {
        Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > factorization(M);
        x = factorization.solve(bEigen);
    }
    //else if(method == LU)
    //{
    //    Eigen::SparseLU< Eigen::SparseMatrix<double> > factorization(M);
    //    x = factorization.solve(bEigen);
    //}
    //else if(method == QR)
    //{
    //    Eigen::SparseQR< Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int> > factorization(M);
    //    x = factorization.solve(bEigen);
    //}
    else if(method == ConjugateGradient_Diag)
    {
        Eigen::ConjugateGradient< Eigen::SparseMatrix<double>, Eigen::Lower, Eigen::DiagonalPreconditioner<double > > solver;
        solver.setTolerance(1e-7);
        solver.compute(M);
        x = solver.solve(bEigen);
        Console::WriteLine("Iterations: " + String(solver.iterations()));
        Console::WriteLine("Error: " + String(solver.error()));
    }
    else if(method == BiCGSTAB_Diag)
    {
        Eigen::BiCGSTAB< Eigen::SparseMatrix<double>, Eigen::DiagonalPreconditioner<double > > solver;
        solver.setTolerance(1e-7);
        solver.compute(M);
        x = solver.solve(bEigen);
        Console::WriteLine("Iterations: " + String(solver.iterations()));
        Console::WriteLine("Error: " + String(solver.error()));
    }
    else if(method == BiCGSTAB_LUT)
    {
        Eigen::BiCGSTAB< Eigen::SparseMatrix<double>, Eigen::IncompleteLUT<double > > solver;
        solver.setTolerance(1e-7);
        solver.compute(M);
        x = solver.solve(bEigen);
        Console::WriteLine("Iterations: " + String(solver.iterations()));
        Console::WriteLine("Error: " + String(solver.error()));
    }
    else if(method == Profile)
    {
        Console::WriteLine("Profiling");
        const int methodCount = (int)Profile;
        Vector< Vector<double> > results(methodCount);
        for(int methodIndex = 0; methodIndex < methodCount; methodIndex++)
        {
            results[methodIndex] = Solve(M, b, (Method)methodIndex);
            if(methodIndex != 0)
            {
                double maxDeviation = 0.0;
                for(UINT variableIndex = 0; variableIndex < b.Length(); variableIndex++) maxDeviation = Math::Max(maxDeviation, fabs(results[methodIndex][variableIndex] - results[0][variableIndex]));
                Console::WriteLine("Max deviation from LLT: " + String(maxDeviation));
            }
        }
    }
    else
    {
        SignalError("Unknown method");
    }

    Console::WriteLine("Solve time: " + String(timer.Elapsed()));

    return Utility::DumpEigenVector(x);
}
