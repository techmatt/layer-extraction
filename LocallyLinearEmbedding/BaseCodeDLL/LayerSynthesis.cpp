#include "Main.h"

/*

S?(5,5)  = RGB(100, 50, 25)
T1(5,5) -= 100 / 255
T2(5,5) -= 50 / 255
T3(5,5) -= 25 / 255

T1 T2
^  ^
|  |
v  v
R1 R2

(0, 1)
(0, 1)
(0.5, 0)
(0, 1)
(0, 1)
(0.5, 0)

Update(T1=0,T2=1) Update(Vector<double>(0,1))
Update(T1=0,T2=1)
Update(T1=0.5,T2=1)
Update(T1=0,T2=1)
Update(T1=0,T2=1)

T1,T2,T3
C1*T1 + C2*T2 + C3*T3 = T
T1 + T2 + T3 = 1

C1*T1 + C2*T2 + C3*T3 + CS*S = T'

solve for: T1',T2',T3'
T1' + T2' + T3' + S = 1
C1*T1' + C2*T2' + C3*T3' + CS*S = T'

T1 T2 T3 T4
^  ^  ^  ^
|  |  |  |
v  v  v  v
R1 R2 R3 R4

0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1

0 0 0 1
0 0 0 1
0.4 0.4 0.4 0
0 0 0 1
0 0 0 1
0.4 0.4 0.4 0

Proposal #1: Update S and T iteratively

UpdateS(1.0)
UpdateS(1.0)
UpdateT(0.5)
UpdateS(1.0)
UpdateS(1.0)
UpdateT(0.5)
...

Proposal #2: Update S and T simultaneously

UpdateST()
UpdateST()
UpdateST()
...

Proposal #3: Update T after the fact
SynthesizeS() -> UpdateS() UpdateS() UpdateS()
FixTheWeights(T1,T2,T3,S?)

All the target layers
face_0.png
face_1.png
face_2.png
face_3.png
face_4_hint.png -> this is the hint for the new layer

The relevant synthesis layers
face_1.png,reference_A.png
face_2.png,reference_B.png

The hint for the newly synthesized layer
synthesis_mask.png

*/

void LayerSynthesis::Init(const GaussianPyramid &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
    InitPCA(layers, generator);
    InitKDTree(layers, generator, reducedDimension);
}

void LayerSynthesis::InitPCA(const GaussianPyramid &layers, const NeighborhoodGenerator &generator)
{
    Console::WriteLine("Initializing PCA...");

    const UINT neighborhoodCount = 32000;
    Vector<const double*> neighborhoods(neighborhoodCount);
    
    const UINT dimension = generator.Dimension();
	const UINT width = layers.Base().First().Width();
	const UINT height = layers.Base().First().Height();
	
	const int nRadius = generator.NeighborhoodRadius()*(int)pow(2,layers.Depth()-1);

    for(UINT neighborhoodIndex = 0; neighborhoodIndex < neighborhoodCount; neighborhoodIndex++)
    {
        double *curNeighborhood = new double[dimension];
        bool success = false;
        while(!success)
        {
			Vec2i centerPt = Vec2i(rand() % (width-nRadius*2)+nRadius, rand() % (height-nRadius*2)+nRadius);
			int xCenter = centerPt.x;
			int yCenter = centerPt.y;

            success = generator.Generate(layers, xCenter, yCenter, curNeighborhood);
        }
        neighborhoods[neighborhoodIndex] = curNeighborhood;
		if (neighborhoodIndex % 1000 == 0)
			Console::WriteLine("Initialized "+String(neighborhoodIndex)+" neighborhoods");
    }
	
    _pca.InitFromDensePoints(neighborhoods, dimension);
    neighborhoods.DeleteMemory();
}

void LayerSynthesis::InitKDTree(const GaussianPyramid &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
	_reducedDimension = reducedDimension;

	const UINT width = layers.Base().First().pixelWeights.Cols();
	const UINT height = layers.Base().First().pixelWeights.Rows();
    const UINT dimension = generator.Dimension();
	const UINT neighbors = 5;

    Vector<const double*> allNeighborhoods;
    
    double *neighborhood = new double[dimension];
    for(UINT y = 0; y < height; y++)
    {
        for(UINT x = 0; x < width; x++)
        {
            if(generator.Generate(layers, x, y, neighborhood))
            {
                double *reducedNeighborhood = new double[reducedDimension];
                _pca.Transform(reducedNeighborhood, neighborhood, reducedDimension);
                allNeighborhoods.PushEnd(reducedNeighborhood);
                _treeCoordinates.PushEnd(Vec2i(x, y));
            }
        }
    }
    delete[] neighborhood;
    Console::WriteLine(String("Building KDTree, ") + String(allNeighborhoods.Length()) + String(" neighborhoods..."));
    _tree.BuildTree(allNeighborhoods, reducedDimension, neighbors);
    allNeighborhoods.DeleteMemory();
}

PixelLayerSet LayerSynthesis::Synthesize(const AppParameters &parameters, const GaussianPyramid &reference, const GaussianPyramid &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule, NeighborhoodGenerator &generator)
{
	//original - the original (incomplete) set of layers
	//pixels - the region drawn by the user, where a new layer(s) should be synthesized (including color information).
	//returns a new set of layers (the original layers might be modified a bit to take into account the new layer(s))

    //...
	Console::WriteLine("Synthesizing layers...");
	Grid<Vec2i> sourceCoordinates(original.Base().First().Height(), original.Base().First().Width(), Vec2i(-1,-1));

    GaussianPyramid target = original;
	for (UINT i=0; i<target.NumLayers(); i++)
	{
		target[0][i].SavePNG("layer_"+String(i)+"_iter0.png");
	}
    for(UINT row = 0; row < updateSchedule.Rows(); row++)
    {
		Console::WriteLine("Synthesizing iter "+ String(row));
		int step = pixels.Length()/5;
		int stepIdx=0;
		for (UINT pixelIndex=0; pixelIndex<pixels.Length(); pixelIndex+=step)
		{
			//VisualizeNeighbors(reference, target, pixels[pixelIndex], generator, "Neighbors_"+String(stepIdx)+"_iter"+String(row)+".png");
			VisualizeMatches(parameters, reference, target, pixels[pixelIndex], generator, "NMatch_"+String(stepIdx++)+"_iter"+String(row)+".png", sourceCoordinates);			
		}
		
        SynthesizeStepInPlace(parameters, reference, target, pixels, updateSchedule.ExtractRow(row), generator, sourceCoordinates);
		for (UINT i=0; i<target.NumLayers(); i++)
		{
			target.Base()[i].SavePNG("layer_"+String(i)+"_iter"+String(row+1)+".png");
		}
		
		//update the target pyramid
		target.Init(PixelLayerSet(target.Base()), target.Depth());
    }
	Console::WriteLine("Done!");
    return target.Base();
}

void LayerSynthesis::SynthesizeStepInPlace(const AppParameters &parameters, const GaussianPyramid &reference, GaussianPyramid &target, const Vector<Vec2i> &pixels, const Vector<double> &updateSchedule, NeighborhoodGenerator &generator, Grid<Vec2i> &sourceCoordinates)
{
	//find the pixel with the nearest transformed neighborhood, and replace with that
	UINT layerCount = target.NumLayers();
	UINT width = target.Base().First().Width();
	UINT height = target.Base().First().Height();
	UINT dimension = generator.Dimension();

	double coherenceParam = parameters.coherenceParameter;
	int progressStep = pixels.Length()/10;
	for (UINT pixelIndex=0; pixelIndex < pixels.Length(); pixelIndex++)
	{
		if (pixelIndex % progressStep == 0)
			Console::WriteLine("Done with "+String(pixelIndex) + " out of " + String(pixels.Length()));

		int x = pixels[pixelIndex].x;
		int y = pixels[pixelIndex].y;

		Vec2i bestPt = BestMatch(parameters, pixels[pixelIndex], reference, target, generator, sourceCoordinates);
		sourceCoordinates(y,x) = bestPt;

		for (UINT layerIndex=0; layerIndex < layerCount; layerIndex++)
		{
			double weight = reference[0][layerIndex].pixelWeights(bestPt.y, bestPt.x);
			double start = target[0][layerIndex].pixelWeights(y,x);

			target[0][layerIndex].pixelWeights(y,x) = Math::Lerp(start, weight, updateSchedule[layerIndex]);
		}
	}

}


Vec2i LayerSynthesis::BestMatch(const AppParameters &parameters, Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, const Grid<Vec2i> &sourceCoordinates)
{
	double coherenceParam = parameters.coherenceParameter;
	
	Vec2i bestPt;
	if (!parameters.useMajorityVote)
	{
		Vec2i approximateMatchPt, coherentMatchPt;
		double nearestDist = BestApproximateMatch(targetPt, reference, target, generator, approximateMatchPt);
		double nearestCoherentDist = BestCoherentMatch(targetPt, reference, target, generator, sourceCoordinates, coherentMatchPt);
		
		if (nearestCoherentDist < (nearestDist + coherenceParam))
			bestPt = coherentMatchPt;
		else
			bestPt = approximateMatchPt;

	} else
		BestNeighborVotedMatch(targetPt, reference, target, generator, bestPt, parameters.neighborRange);

	return bestPt;
}



//See where the neighbors would match to, and pick the match that is spatially close to the majority neighbor matches and is feature-wise close to the target neighborhood
double LayerSynthesis::BestNeighborVotedMatch(Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, Vec2i &outPt, int range)
{
	Vector<Vec2i> candidatePts;
	UINT dimension = generator.Dimension();
	
	for (int dx=-range; dx<=range; dx+=range)
	{
		for (int dy=-range; dy<=range; dy+=range)
		{
			//if neighbor is out of bounds, ignore it
			if (!target.Base().First().pixelWeights.ValidCoordinates(targetPt.y+dy, targetPt.x+dx))
				continue;

			//find the best match for the neighbor
			Vec2i neighbor = targetPt + Vec2i(dx,dy);
			Vec2i neighborMatch;
			BestApproximateMatch(neighbor, reference, target, generator, neighborMatch);

			Vec2i candidate = neighborMatch - Vec2i(dx,dy);

			if (reference.Base().First().pixelWeights.ValidCoordinates(candidate.y, candidate.x))
				candidatePts.PushEnd(candidate);
		}
	}

	double* targetNeighborhood = new double[dimension]; 
	double* targetTransformedNeighborhood = new double[_reducedDimension];
	generator.Generate(target, targetPt.x, targetPt.y, targetNeighborhood);
	_pca.Transform(targetTransformedNeighborhood, targetNeighborhood, _reducedDimension);

	double* neighborhood = new double[dimension]; 
	double* transformedNeighborhood = new double[_reducedDimension];
	double bestScore = numeric_limits<double>::max();

	for (UINT candidateIndex = 0; candidateIndex < candidatePts.Length(); candidateIndex++)
	{
		Vec2i candidate = candidatePts[candidateIndex];

		//get the neighborhood distance
		generator.Generate(reference, candidate.x, candidate.y, neighborhood);
		_pca.Transform(transformedNeighborhood, neighborhood, _reducedDimension);
		double neighborhoodDistance = NeighborhoodDistance(targetTransformedNeighborhood, transformedNeighborhood, _reducedDimension);

		//get the distance to the other candidates
		Vector<Vec2i> sortedCandidates(candidatePts);
		sortedCandidates.Sort([&candidate](const Vec2i &a, const Vec2i &b){ return Vec2i::Dist(a, candidate) < Vec2i::Dist(b, candidate); });

		//only consider the top 60% of candidates
		UINT topNum = (int)Math::Round(0.6*sortedCandidates.Length());
		double neighborMatchDistance = 0;
		
		if (topNum > 0)
		{
			for (int neighborIndex=0; neighborIndex < (int)topNum; neighborIndex++)
				neighborMatchDistance += Vec2i::Dist(candidate, sortedCandidates[neighborIndex]);
			neighborMatchDistance /= topNum;
		}

		double score = neighborhoodDistance + neighborMatchDistance;
		if (score < bestScore)
		{
			bestScore = score;
			outPt.x = candidate.x;
			outPt.y = candidate.y;
		}

	}

	//outPt.x = 0;
	//outPt.y = 0;

	delete[] targetNeighborhood;
	delete[] targetTransformedNeighborhood;

	delete[] neighborhood;
	delete[] transformedNeighborhood;

	return bestScore;
}


Vector<Vec2i> LayerSynthesis::GetCandidateSourceNeighbors(Vec2i targetPt, const Grid<Vec2i> &sourceCoordinates, const GaussianPyramid &reference)
{
	//search for candidate coherent neighbors to inspect
	Vector<Vec2i> candidatePts;

	for (int dx=-1; dx<=1; dx++)
	{
		for (int dy=-1; dy<=1; dy++)
		{
			if (dx == 0 && dy == 0)
				continue;

			//hasn't been assigned yet, or out of bounds
			if (sourceCoordinates(targetPt.y, targetPt.x).x < 0 || sourceCoordinates(targetPt.y, targetPt.x).y < 0 || !sourceCoordinates.ValidCoordinates(targetPt.y+dy, targetPt.x+dx) || sourceCoordinates(targetPt.y+dy, targetPt.x+dx).x < 0 || sourceCoordinates(targetPt.y+dy, targetPt.x+dx).y < 0 )
				continue;

			Vec2i candidate = sourceCoordinates(targetPt.y+dy, targetPt.x+dx) - Vec2i(dx,dy);
			if (reference.Base().First().pixelWeights.ValidCoordinates(candidate.y, candidate.x))
				candidatePts.PushEnd(candidate);
		}
	}
	return candidatePts;
}


double LayerSynthesis::BestCoherentMatch(Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, const Grid<Vec2i> &sourceCoordinates, Vec2i &outPt)
{
	Vector<Vec2i> candidates = GetCandidateSourceNeighbors(targetPt, sourceCoordinates, reference);

	UINT dimension = generator.Dimension();

	double* targetNeighborhood = new double[dimension]; 
	double* targetTransformedNeighborhood = new double[_reducedDimension];

	generator.Generate(target, targetPt.x, targetPt.y, targetNeighborhood);
	_pca.Transform(targetTransformedNeighborhood, targetNeighborhood, _reducedDimension);

	double bestDist = numeric_limits<double>::max();
	double* neighborhood = new double[dimension]; 
	double* transformedNeighborhood = new double[_reducedDimension];

	for (UINT candidateIndex=0; candidateIndex < candidates.Length(); candidateIndex++)
	{
		Vec2i candidate = candidates[candidateIndex];

		generator.Generate(target, candidate.x, candidate.y, neighborhood);
		_pca.Transform(transformedNeighborhood, neighborhood, _reducedDimension);

		double dist = NeighborhoodDistance(targetTransformedNeighborhood, transformedNeighborhood, _reducedDimension);
		if (dist < bestDist)
		{
			bestDist = dist;
			outPt.x = candidate.x;
			outPt.y = candidate.y;
		}
	}

	delete[] targetNeighborhood;
    delete[] targetTransformedNeighborhood;

	delete[] neighborhood;
    delete[] transformedNeighborhood;

	return bestDist;
}

double LayerSynthesis::BestApproximateMatch(Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, Vec2i &outPt)
{
	UINT dimension = generator.Dimension();
	double* neighborhood = new double[dimension]; 
	double* transformedNeighborhood = new double[_reducedDimension];
	Vector<UINT> indices;

	generator.Generate(target, targetPt.x, targetPt.y, neighborhood);
	_pca.Transform(transformedNeighborhood, neighborhood, _reducedDimension);

	//find nearest pixel neighborhood			
	_tree.KNearest(transformedNeighborhood, 1, indices, 0.0f);
	Vec2i sourceCoordinate = _treeCoordinates[indices[0]];

	outPt.x = sourceCoordinate.x;
	outPt.y = sourceCoordinate.y;

	double* matchedNeighborhood = _tree.GetDataPoint(indices[0]);

	double distance = NeighborhoodDistance(transformedNeighborhood, matchedNeighborhood, _reducedDimension);
	delete[] neighborhood;
    delete[] transformedNeighborhood;

	return distance;
}

double LayerSynthesis::NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension)
{
	double result = 0;
	for (UINT i=0; i<dimension; i++)
		result += Math::Square(neighborhoodA[i]-neighborhoodB[i]);
	return result;
}

void LayerSynthesis::VisualizeNeighbors(const GaussianPyramid &reference, const GaussianPyramid &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename)
{
	//visualize the match for the given target Pt, and the matches for the neighbors
	UINT width = target.Base().First().pixelWeights.Cols();
	UINT height = target.Base().First().pixelWeights.Rows();

	//initialize the base image
	Bitmap result(width + reference.Base().First().pixelWeights.Cols(), Math::Max(height, reference.Base().First().pixelWeights.Rows()), RGBColor(255,255,255));
	Bitmap targetImage, refImage;
	VisualizeLayers(target, targetImage);
	VisualizeLayers(reference, refImage);
	for (int x=0; x<(int)width; x++)
		for (int y=0; y<(int)height; y++)
			if (targetImage.ValidCoordinates(x,y))
				result[y][x] = targetImage[y][x];
	for (UINT x=width; x<(int)result.Width(); x++)
		for(UINT y=0; y<(int)result.Height(); y++)
			if (refImage.ValidCoordinates(x-width,y))
				result[y][x] = refImage[y][x-width];
	

	AliasRender render;
	
	Vector<RGBColor> colors;
	colors.PushEnd(RGBColor::Red);
	colors.PushEnd(RGBColor::Yellow);
	colors.PushEnd(RGBColor::Green);
	colors.PushEnd(RGBColor::Blue);
	colors.PushEnd(RGBColor::Purple);

	UINT dimension = generator.Dimension();
	double* neighborhood = new double[dimension]; 
	double* transformedNeighborhood = new double[_reducedDimension];
	Vector<UINT> indices;

	generator.Generate(target, targetPt.x, targetPt.y, neighborhood);
	_pca.Transform(transformedNeighborhood, neighborhood, _reducedDimension);

	//find nearest pixel neighborhood			
	_tree.KNearest(transformedNeighborhood, 5, indices, 0.0f);
	Vec2i sourceCoordinate = _treeCoordinates[indices[0]];		
	render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(sourceCoordinate, Vec2i(2, 2)), colors[0], colors[0]);
	
	for (UINT i=0; i<indices.Length(); i++)
	{
		Vec2i sourceCoordinate = _treeCoordinates[indices[i]];		
		render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(sourceCoordinate+Vec2i(width,0), Vec2i(2, 2)), colors[i], colors[i]);
	}

	result.SavePNG(filename);
}


void LayerSynthesis::VisualizeMatches(const AppParameters &parameters, const GaussianPyramid &reference, const GaussianPyramid &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename, const Grid<Vec2i> &sourceCoordinates)
{
	//visualize the match for the given target Pt, and the matches for the neighbors
	UINT width = target.Base().First().pixelWeights.Cols();
	UINT height = target.Base().First().pixelWeights.Rows();

	//initialize the base image
	Bitmap result(width + reference.Base().First().pixelWeights.Cols(), Math::Max(height, reference.Base().First().pixelWeights.Rows()), RGBColor(255,255,255));
	Bitmap targetImage, refImage;
	VisualizeLayers(target, targetImage);
	VisualizeLayers(reference, refImage);
	for (int x=0; x<(int)width; x++)
		for (int y=0; y<(int)height; y++)
			if (targetImage.ValidCoordinates(x,y))
				result[y][x] = targetImage[y][x];
	for (int x=width; x<(int)result.Width(); x++)
		for(int y=0; y<(int)result.Height(); y++)
			if (refImage.ValidCoordinates(x-width,y))
				result[y][x] = refImage[y][x-width];
	

	AliasRender render;
	
	Vector<RGBColor> colors;
	colors.PushEnd(RGBColor::Cyan);
	colors.PushEnd(RGBColor::Magenta);
	colors.PushEnd(RGBColor::Yellow);
	colors.PushEnd(RGBColor::Green);
	colors.PushEnd(RGBColor::Purple);

	Vec2i deltas[] = {Vec2i(0,0), Vec2i(-1,0), Vec2i(1,0), Vec2i(0,1), Vec2i(0,-1)};

	UINT dimension = generator.Dimension();
	double* neighborhood = new double[dimension]; 
	double* transformedNeighborhood = new double[_reducedDimension];
	Vector<UINT> indices;

	double coherenceParam = parameters.coherenceParameter;

	for (UINT i=0; i<colors.Length(); i++)
	{
		Vec2i neighbor = targetPt + 5*deltas[i];
		
		if (neighbor.x >= 0 && neighbor.x < (int)width && neighbor.y >=0 && neighbor.y < (int)height)
		{
			Vec2i bestPt = BestMatch(parameters, neighbor, reference, target, generator, sourceCoordinates);

			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(neighbor, Vec2i(2, 2)), colors[i], colors[i]);
			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(bestPt+Vec2i(width,0), Vec2i(2, 2)), colors[i], colors[i]);
		}
	}
	result.SavePNG(filename);
}

void LayerSynthesis::VisualizeLayers(const GaussianPyramid &layers, Bitmap &result)
{
	int width = layers.Base().First().pixelWeights.Cols();
	int height = layers.Base().First().pixelWeights.Rows();
	result.Allocate(width, height);
	
	for (int x=0; x<width; x++)
	{
		for (int y=0; y<height; y++)
		{
			Vec3f value(0,0,0);
			for (UINT layerIndex=0; layerIndex<layers.NumLayers(); layerIndex++)
				value += (float)layers[0][layerIndex].pixelWeights(y,x)*layers[0][layerIndex].color;
			result[y][x] = RGBColor(value);
		}
	}
}
