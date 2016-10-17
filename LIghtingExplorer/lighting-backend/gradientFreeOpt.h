
// interface

typedef function<float(const vector<float> &x)> FitnessFunction;
class GradientFreeProblem
{
public:
	FitnessFunction fitness;
	vector< vector<float> > startingPoints;
};

class GradientFreeOpt
{
public:
	virtual vector<float> optimize(const GradientFreeProblem &problem) = 0;
};

class GradientFreeOptRandomWalk
{
public:
	vector<float> optimize(const GradientFreeProblem &problem)
	{
		const int totalMutations = 10000;
		const int wavefrontSize = 100;
		const int wavefrontCullRate = 50;

		for (auto &i : problem.startingPoints)
			addWavefrontEntry(problem, i);

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
			}
		}

		sort(wavefront.begin(), wavefront.end(), [](const WavefrontEntry &a, const WavefrontEntry &b) { return a.fitness > b.fitness; });
		return wavefront[0].values;
	}

private:
	struct WavefrontEntry
	{
		vector<float> values;
		float fitness;
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
		const float expectedMutations = 5.0f;
		const float mutationChance = expectedMutations / (float)x.size();

		vector<float> result = x;
		for (int i = 0; i < x.size(); i++)
		{
			if (util::randomUniformf() < mutationChance)
				result[i] += (float)RNG::global.normal();
		}
		return result;
	}

	vector< WavefrontEntry > wavefront;
};
