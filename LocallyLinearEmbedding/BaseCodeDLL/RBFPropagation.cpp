#include "Main.h"


RBFPropagation::RBFPropagation(void)
{
}


RBFPropagation::~RBFPropagation(void)
{
}

VecNf RBFPropagation::MakeFeatureVector(const AppParameters &parameters, const Video &video, Vec2i coord, int frame)
{
	RGBColor color = video.frames[frame][coord.y][coord.x];
	VecNf features(6);
    features[0] = (color.r / 255.0f) / parameters.rbf_colorScale;
    features[1] = (color.g / 255.0f) / parameters.rbf_colorScale;
    features[2] = (color.b / 255.0f) / parameters.rbf_colorScale;
    features[3] = (coord.x/(float)video.Width()) / parameters.rbf_spatialScale;
    features[4] = (coord.y/(float)video.Height()) / parameters.rbf_spatialScale; //though, in the code they normalize this by the width and not the height
	features[5] = (frame/(float)video.frames.Length()) / parameters.rbf_timeScale;
	return features;
}


Video RBFPropagation::Recolor(const AppParameters &parameters, const Video &video, const Vector<PixelConstraint> &targetPixelColors)
{
	//parameters
	double constraintRatio = parameters.rbf_basisRatio;
	int numBasis = targetPixelColors.Length()*constraintRatio;

	Console::WriteLine("Using "+String(numBasis)+ " basis functions");

	int numConstraints = targetPixelColors.Length();

	//Select a random subset of the edits
	Vector<PixelConstraint> randomConstraints(targetPixelColors);
	randomConstraints.Randomize();

	//Construct the interpolation function
	DenseMatrix<double> A(numConstraints, numBasis);
	Vector<double> b(numConstraints);
	Vector<VecNf> centers;

	//Assume there is only one color change, find the average shift.
	Vec3f fromColor(0,0,0);
	Vec3f targetColor(0,0,0);
	double numChanges = 0;

	Bitmap debug(video.Width(), video.Height(), RGBColor::Black);

	for (UINT constraintIndex=0; constraintIndex < targetPixelColors.Length(); constraintIndex++)
	{
		const PixelConstraint& constraint = targetPixelColors[constraintIndex];
		if (constraint.change)
		{		
			targetColor += targetPixelColors[constraintIndex].targetColor;
			fromColor += Vec3f(video.frames[constraint.frame][constraint.coord.y][constraint.coord.x]);
			numChanges++;
			debug[constraint.coord.y][constraint.coord.x] = RGBColor(targetPixelColors[constraintIndex].targetColor);
		} else
			debug[constraint.coord.y][constraint.coord.x] = RGBColor::White;
	}
	debug.SavePNG("scribbles.png");
	fromColor = fromColor/numChanges;
	targetColor = targetColor/numChanges;
	Vec3f edit = targetColor - fromColor;
	Console::WriteLine("Changing from/to " + RGBColor(fromColor).CommaSeparatedString() + " " + RGBColor(targetColor).CommaSeparatedString());
	Console::WriteLine("Negative strokes " + String(targetPixelColors.Length()-numChanges));


	for (int r=0; r < numConstraints; r++)
	{	
		const PixelConstraint& trainConstraint = targetPixelColors[r];
		VecNf trainFeature = MakeFeatureVector(parameters, video, trainConstraint.coord, trainConstraint.frame);
		b[r] = (trainConstraint.change)? 1 : 0;

		for (int c = 0; c < numBasis; c++)
		{
			const PixelConstraint& basisConstraint = randomConstraints[c];
			VecNf basisFeature = MakeFeatureVector(parameters, video, basisConstraint.coord, basisConstraint.frame);

			A(r,c) = exp(-1*VecNf::DistSq(basisFeature, trainFeature));	
		
		}
	}

	for (int c = 0; c < numBasis; c++)
	{	
		const PixelConstraint& basisConstraint = randomConstraints[c];
		VecNf basisFeature = MakeFeatureVector(parameters, video, basisConstraint.coord, basisConstraint.frame);
		centers.PushEnd(basisFeature);
	}
		
	NNLeastSquaresSolver solver;
	Vector<double> solution = solver.Solve(A, b);

	GaussianInterpolationFunction predictor;
	predictor.alphas = solution;
	predictor.centers = centers;

	Console::WriteLine("Recoloring Video");

	//Recolor the edit according to the interpolation function
	int frameWidth = video.Width();
	int frameHeight = video.Height();
	int numFrames = video.frames.Length();
	Video result;
	
	for (int f=0; f<numFrames; f++)
	{
		Bitmap frame(video.frames[f]);
		for (int x=0; x<frameWidth; x++)
		{
			for (int y=0; y<frameHeight; y++)
			{
				VecNf feature = MakeFeatureVector(parameters, video, Vec2i(x,y), f);
				double editWeight = predictor.Evaluate(feature);
				//Console::WriteLine("Weight "+String(editWeight));
				frame[y][x] = RGBColor(Vec3f(frame[y][x]) + editWeight*edit);
			}
		}
		frame.SavePNG("frame_"+String(f)+".png");
		result.frames.PushEnd(frame);
	}
	Console::WriteLine("Done recoloring");
	return result;
}