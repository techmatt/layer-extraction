using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MathNet.Numerics.LinearAlgebra.Double;
using System.Drawing;
using System.Diagnostics;

namespace BaseCodeApp
{
   
    class LayerExtract
    {

        private static DenseVector Project(DenseVector weights, Random random)
        {
            int n = weights.Count;
            DenseVector nw = new DenseVector(n);
            double sum = 0;
            for (int i = 0; i < n; i++)
                sum += Math.Max(weights[i], 0);

            if (sum == 0)
            {
                Console.WriteLine("random projection back");
                double sum2 = 0;
                for (int i = 0; i < n; i++)
                {
                    double val = random.NextDouble();
                    nw[i] = val;
                    sum2 += val;
                }
                for (int i = 0; i < n; i++)
                    nw[i] /= sum2;
            }
            else
            {
                for (int i = 0; i < n; i++)
                    nw[i] = Math.Max(weights[i], 0) / sum;
            }
            return nw;
        }



        public static List<double[,]> SolveLayersGradDescent(DenseVector[,] image, List<DenseVector> palette)
        {
            //store the layers
            int width = image.GetLength(0);
            int height = image.GetLength(1);

            List<double[,]> layers = new List<double[,]>();
            for (int l = 0; l < palette.Count(); l++)
                layers.Add(new double[width, height]);

            double[,] errors = new double[width, height];

            int n = palette.Count();

            DenseMatrix Z = new DenseMatrix(3, palette.Count());
            for (int c = 0; c < palette.Count(); c++)
            {
                
                Z[0, c] = palette[c][0];
                Z[1, c] = palette[c][1];
                Z[2, c] = palette[c][2];
            }
            Console.WriteLine("Z");
            Console.WriteLine(Z);

            int maxIters = 500;
            double convThresh = 0.005;
            double valThresh = 5;
            Random random = new Random();
            double epsilon = 1;

            double sumIters = 0;

            Stopwatch watch = new Stopwatch();
            watch.Start();

            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    DenseVector pixel = image[x,y];
                    DenseMatrix P = new DenseMatrix(3, palette.Count);
                    for (int c = 0; c < palette.Count(); c++)
                    {
                        P[0, c] = pixel[0];
                        P[1, c] = pixel[1];
                        P[2, c] = pixel[2];
                    }
                    DenseMatrix Zp = Z - P;
                    var C = Zp.TransposeThisAndMultiply(Zp);


                    //solve analytically first
                    //Condition if singular
                    DenseVector ones = DenseVector.Create(n, i=> 1);
                    DenseMatrix G = DenseMatrix.OfArray(C.ToArray());
                    if (C.Determinant() == 0)
                        G = DenseMatrix.OfMatrix(C + (DenseMatrix.Identity(C.RowCount).Multiply(epsilon)));

                    DenseVector w = DenseVector.OfVector(G.LU().Solve(ones));
                    double sum = 0;
                    bool valid = true;
                    for (int i = 0; i < n; i++)
                    {
                        sum += Math.Max(w[i], 0);
                        if (w[i] < -0.01)
                            valid = false;
                    }

                    //First see if analytic solution is valid
                    if (valid)
                    {
                        for (int i = 0; i < n; i++)
                            layers[i][x, y] = Math.Max(w[i], 0) / sum;
                        continue;
                    }

                    //randomly initialize weight vector and start gradient descent
                    DenseVector weights = new DenseVector(n);
                    int countValid = 0;
                    if (x > 0 && errors[x - 1, y] < valThresh)
                    {
                        countValid++;
                        for (int i = 0; i < n; i++)
                            weights[i] = layers[i][x - 1, y];
                    }
                    if (y > 0 && errors[x, y - 1] < valThresh)
                    {
                        countValid++;
                        for (int i = 0; i < n; i++)
                            weights[i] += layers[i][x, y - 1];
                    }
                    if (countValid > 0)
                        for (int i = 0; i < n; i++)
                            weights[i] /= countValid;



                    if (countValid == 0)
                    {
                        for (int i = 0; i < n; i++)
                            weights[i] = 1.0 / n;//random.NextDouble());
                        //weights = Project(weights, random);
                    }

                    double value = 0;
                    for (int i = 0; i < maxIters; i++)
                    {
                        //Update and project
                        value = (weights.ToRowMatrix() * C * weights)[0];
                        var gradient = (C * weights).Multiply(2.0);
                        double norm = gradient.Norm(2);
                        if (norm < epsilon || value < valThresh * valThresh)
                        {
                            sumIters += i;
                            break;
                        }
                        double step = value / (norm * norm);
                        var weights_i = Project(DenseVector.OfVector(weights - gradient.Multiply(step)), random);

                        double conv = (weights - weights_i).Norm(2);
                        if (conv == 0)
                        {
                            //try again with better initialization
                            for (int l = 0; l < n; l++)
                                weights_i[l] = random.NextDouble();
                            weights_i = Project(weights_i, random);
                        }
                        else if (conv <= convThresh)
                        {
                            sumIters += i;
                            break;
                        }

                        //if (i == maxIters - 1)
                        //    Console.WriteLine(String.Format("Giving up {0} {1} conv {2} value {3} grad {4}", x, y, conv, Math.Sqrt(value), norm));
                        if (i == maxIters - 1)
                            sumIters += i;

                        weights = weights_i;

                    }
                    weights = Project(weights, random);
                    errors[x, y] = value;

                    //Console.WriteLine("elapsed " + watch.ElapsedMilliseconds / 1000.0);

                    for (int i = 0; i < n; i++)
                        layers[i][x, y] = weights[i];

                }
            }
            double time = watch.ElapsedMilliseconds;
            Console.WriteLine("average iters:" + sumIters / (width * height));
            Console.WriteLine("elapsed(s) " + time / 1000.0);
            Console.WriteLine("time per pixel (ms): " + time / (width * height));


            return layers;
        }
    }
}
