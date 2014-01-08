#pragma once

struct GaussianInterpolationFunction
{
	Vector<double> alphas;
	Vector<VecNf> centers;

	double Evaluate(VecNf value)
	{
		double result = 0;
		for (UINT i=0; i<alphas.Length(); i++)
		{
			double sqdist = VecNf::DistSq(centers[i],value);
			result += alphas[i]*exp(-1*sqdist);
		}
		return result;
	}

};

class RBFPropagation
{
public:
	RBFPropagation(void);
	~RBFPropagation(void);

	Video Recolor(const AppParameters &parameters, const Video &video, const Vector<PixelConstraint> &targetPixelColors);

private:
	VecNf MakeFeatureVector(const AppParameters &parameters, const Video &video, Vec2i coord, int frame);

};

