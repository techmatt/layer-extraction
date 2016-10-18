
// interface

#include "CMAES/src/cmaes_interface.h"

typedef function<double(const vector<float> &x)> FitnessFunction;
typedef function<Bitmap(const vector<float> &x)> RenderFunction;
class GradientFreeProblem
{
public:
	FitnessFunction fitness;
	RenderFunction render;
	//vector< vector<float> > startingPoints;
};

class GradientFreeOpt
{
public:
	virtual vector<float> optimize(const GradientFreeProblem &problem, const vector<float> &startX, vector< vector<float> > &candidates) = 0;
};

class GradientFreeOptCMAES
{
public:
	vector<float> optimize(const GradientFreeProblem &problem, const vector<float> &startX, vector< vector<float> > &candidates)
	{
		vector<float> x = optimizeInternal(problem, startX, 50, 20, nullptr);
		x = optimizeInternal(problem, x, 50, 20, nullptr);
		x = optimizeInternal(problem, x, 50, 20, nullptr);
		x = optimizeInternal(problem, x, 50, 20, nullptr);
		x = optimizeInternal(problem, x, 100, 50, nullptr);
		x = optimizeInternal(problem, x, 100, 50, nullptr);
		x = optimizeInternal(problem, x, 100, 50, nullptr);
		x = optimizeInternal(problem, x, 100, 200, &candidates);
		return x;
	}

	vector<float> optimizeInternal(const GradientFreeProblem &problem, const vector<float> &startingPoint, int lambda, int maxIters, vector< vector<float> > *candidates)
	{
		cmaes_t opt;

		const int dimension = startingPoint.size();
		vector<double> xStart, stdDev;
		for (float f : startingPoint)
		{
			stdDev.push_back(0.01f);
			xStart.push_back(f);
		}
		
		/* Initialize everything into the struct evo, 0 means default */
		double *arFunVals = cmaes_init(&opt, dimension, xStart.data(), stdDev.data(), 0, lambda, nullptr);
		//cout << cmaes_SayHello(&opt) << endl;
		int iter = 0;
		const bool verbose = false;
		while (!cmaes_TestForTermination(&opt) && iter <= maxIters)
		{
			double *const* pop = cmaes_SamplePopulation(&opt);

			int bestPopIndex = 0;
			double bestPopValue = std::numeric_limits<double>::max();

#pragma omp parallel for schedule(dynamic, 1)
			for (int i = 0; i < lambda; i++)
			{
				vector<float> x(dimension);
				for (int j = 0; j < dimension; j++)
					x[j] = (float)pop[i][j];

				

				arFunVals[i] = -problem.fitness(x);

				if (arFunVals[i] < bestPopValue)
				{
					bestPopIndex = i;
					bestPopValue = arFunVals[i];
				}

				if (verbose && i == 0)
				{
					cout << "iter " << iter << ": " << arFunVals[0] << ", " << arFunVals[1] << endl;
					const Bitmap bmp = problem.render(x);
					LodePNG::save(bmp, R"(C:\Code\layer-extraction\Images\debug\)" + to_string(iter) + ".png");
				}
			}

			if (candidates != nullptr)
			{
				vector<float> x(dimension);
				for (int j = 0; j < dimension; j++)
					x[j] = (float)pop[bestPopIndex][j];
				candidates->push_back(x);
				cout << "iter " << iter << ": " << bestPopValue << endl;
			}

			iter++;
			cmaes_UpdateDistribution(&opt, arFunVals);
		}
		string terminationReason = "maxIter reached";
		if (cmaes_TestForTermination(&opt) != nullptr)
			terminationReason = cmaes_TestForTermination(&opt);
		//cout << "done: " << terminationReason << endl;
		cout << "optInternal done, " << terminationReason << endl;
		
		//xfinal = cmaes_GetNew(&opt, "xmean"); /* "xbestever" might be used as well */
		double *xFinal = cmaes_GetNew(&opt, "xbestever"); /* "xbestever" might be used as well */
		cmaes_exit(&opt); /* release memory */

		vector<float> result(dimension);
		for (int j = 0; j < dimension; j++)
			result[j] = (float)xFinal[j];
		if (candidates != nullptr) candidates->push_back(result);
		free(xFinal);

		return result;
	}
};

class GradientFreeOptRandomWalk
{
public:
	vector<float> optimize(const GradientFreeProblem &problem, const vector<float> &startX)
	{
		const int totalMutations = 1000;
		const int wavefrontSize = 50;
		const int wavefrontCullRate = 50;
		const bool verbose = true;

		addWavefrontEntry(problem, startX);
		
		int mutationCount = 0;
		while (mutationCount < totalMutations)
		{
			const vector<float> mutatedX = mutate(util::randomElement(wavefront).values);
			addWavefrontEntry(problem, mutatedX);

			mutationCount++;
			if (mutationCount % wavefrontCullRate == 0)
			{
				sort(wavefront.begin(), wavefront.end(), [](const WavefrontEntry &a, const WavefrontEntry &b) { return a.fitness > b.fitness; });
				if (wavefront.size() > wavefrontSize) wavefront.resize(wavefrontSize);
				cout << "mutation " << mutationCount << ": " << wavefront.front().fitness << " -> " << wavefront.back().fitness << endl;
				if (verbose)
				{
					Bitmap bmp = problem.render(wavefront.front().values);
					LodePNG::save(bmp, R"(C:\Code\layer-extraction\Images\debug\)" + to_string(mutationCount) + ".png");
				}
			}
		}

		sort(wavefront.begin(), wavefront.end(), [](const WavefrontEntry &a, const WavefrontEntry &b) { return a.fitness > b.fitness; });
		return wavefront[0].values;
	}

private:
	struct WavefrontEntry
	{
		vector<float> values;
		double fitness;
	};

	void addWavefrontEntry(const GradientFreeProblem &problem, const vector<float> &x)
	{
		WavefrontEntry entry;
		entry.values = x;
		entry.fitness = problem.fitness(x);
		wavefront.push_back(entry);
	}

	vector<float> mutate(const vector<float> &x)
	{
		const float expectedMutations = 3.0f;
		const float mutationChance = expectedMutations / (float)x.size();

		vector<float> result = x;
		for (int i = 0; i < x.size(); i++)
		{
			if (util::randomUniformf() < mutationChance)
				result[i] += (float)RNG::global.normal() * 0.1f;
		}
		return result;
	}

	vector< WavefrontEntry > wavefront;
};
