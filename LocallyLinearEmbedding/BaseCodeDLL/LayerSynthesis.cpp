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

void LayerSynthesis::Init(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
    InitPCA(layers, generator);
    InitKDTree(layers, generator, reducedDimension);
}

void LayerSynthesis::InitPCA(const PixelLayerSet &layers, const NeighborhoodGenerator &generator)
{
    Console::WriteLine("Initializing PCA...");

    const UINT neighborhoodCount = 32000;
    Vector<const double*> neighborhoods(neighborhoodCount);
    
    const UINT dimension = generator.Dimension();
	const UINT width = layers.First().pixelWeights.Cols();
	const UINT height = layers.First().pixelWeights.Rows();

    for(UINT neighborhoodIndex = 0; neighborhoodIndex < neighborhoodCount; neighborhoodIndex++)
    {
        double *curNeighborhood = new double[dimension];
        bool success = false;
        while(!success)
        {
			int xCenter = rand() % width;
			int yCenter = rand() % height;
            success = generator.Generate(layers, xCenter, yCenter, curNeighborhood);
			for(UINT i = 0; i < dimension; i++)
			{
				if(curNeighborhood[i] < -10000.0 || curNeighborhood[i] > 10000.0)
				{
					success = generator.Generate(layers, xCenter, yCenter, curNeighborhood);
				}
			}
        }
        neighborhoods[neighborhoodIndex] = curNeighborhood;
    }

    _pca.InitFromDensePoints(neighborhoods, dimension);
    neighborhoods.DeleteMemory();
}

void LayerSynthesis::InitKDTree(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
	_reducedDimension = reducedDimension;

	const UINT width = layers.First().pixelWeights.Cols();
	const UINT height = layers.First().pixelWeights.Rows();
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

PixelLayerSet LayerSynthesis::Synthesize(const AppParameters &parameters, const PixelLayerSet &reference, const PixelLayerSet &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule, NeighborhoodGenerator &generator)
{
	//original - the original (incomplete) set of layers
	//pixels - the region drawn by the user, where a new layer(s) should be synthesized (including color information).
	//returns a new set of layers (the original layers might be modified a bit to take into account the new layer(s))

    //...
	Console::WriteLine("Synthesizing layers...");
	Grid<Vec2i> sourceCoordinates(original.First().pixelWeights.Rows(), original.First().pixelWeights.Cols(), Vec2i(-1,-1));

    PixelLayerSet target = original;
	for (UINT i=0; i<target.Length(); i++)
	{
		target[i].SavePNG("layer_"+String(i)+"_iter0.png");
	}
    for(UINT row = 0; row < updateSchedule.Rows(); row++)
    {
		int step = pixels.Length()/5;
		int stepIdx=0;
		for (UINT pixelIndex=0; pixelIndex<pixels.Length(); pixelIndex+=step)
		{
			//VisualizeNeighbors(reference, target, pixels[pixelIndex], generator, "Neighbors_"+String(stepIdx)+"_iter"+String(row)+".png");
			VisualizeMatches(parameters, reference, target, pixels[pixelIndex], generator, "NMatch_"+String(stepIdx++)+"_iter"+String(row)+".png", sourceCoordinates);			
		}

        SynthesizeStepInPlace(parameters, reference, target, pixels, updateSchedule.ExtractRow(row), generator, sourceCoordinates);
		for (UINT i=0; i<target.Length(); i++)
		{
			target[i].SavePNG("layer_"+String(i)+"_iter"+String(row+1)+".png");
		}
    }
	Console::WriteLine("Done!");
    return target;
}

void LayerSynthesis::SynthesizeStepInPlace(const AppParameters &parameters, const PixelLayerSet &reference, PixelLayerSet &target, const Vector<Vec2i> &pixels, const Vector<double> &updateSchedule, NeighborhoodGenerator &generator, Grid<Vec2i> &sourceCoordinates)
{
	//find the pixel with the nearest transformed neighborhood, and replace with that
	UINT layerCount = target.Length();
	UINT width = target.First().pixelWeights.Cols();
	UINT height = target.First().pixelWeights.Rows();
	UINT dimension = generator.Dimension();

	double coherenceParam = parameters.coherenceParameter;
	double coherentUsed = 0;
	for (UINT pixelIndex=0; pixelIndex < pixels.Length(); pixelIndex++)
	{
		int x = pixels[pixelIndex].x;
		int y = pixels[pixelIndex].y;

		Vec2i approximateMatchPt, coherentMatchPt;
		double nearestDist = BestApproximateMatch(pixels[pixelIndex], reference, target, generator, approximateMatchPt);
		double nearestCoherentDist = BestCoherentMatch(pixels[pixelIndex], reference, target, generator, sourceCoordinates, coherentMatchPt);

		Vec2i bestPt;
		if (nearestCoherentDist < (nearestDist + coherenceParam))
		{
			bestPt = coherentMatchPt;
			coherentUsed++;
		} else
			bestPt = approximateMatchPt;

		sourceCoordinates(y,x) = bestPt;

		for (UINT layerIndex=0; layerIndex < layerCount; layerIndex++)
		{
			double weight = reference[layerIndex].pixelWeights(bestPt.y, bestPt.x);
			double start = target[layerIndex].pixelWeights(y,x);

			target[layerIndex].pixelWeights(y,x) = Math::Lerp(start, weight, updateSchedule[layerIndex]);
		}
	}
	Console::WriteLine("Frac Coherent pixels used " + String(coherentUsed/pixels.Length()));

}


Vector<Vec2i> LayerSynthesis::GetCandidateSourceNeighbors(Vec2i targetPt, const Grid<Vec2i> &sourceCoordinates, const PixelLayerSet &reference)
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
			if (reference.First().pixelWeights.ValidCoordinates(candidate.y, candidate.x))
				candidatePts.PushEnd(candidate);
		}
	}
	return candidatePts;
}

double LayerSynthesis::BestCoherentMatch(Vec2i targetPt, const PixelLayerSet &reference, const PixelLayerSet &target, NeighborhoodGenerator &generator, const Grid<Vec2i> &sourceCoordinates, Vec2i &outPt)
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

double LayerSynthesis::BestApproximateMatch(Vec2i targetPt, const PixelLayerSet &reference, const PixelLayerSet &target, NeighborhoodGenerator &generator, Vec2i &outPt)
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


	double distance = NeighborhoodDistance(transformedNeighborhood, _tree.GetDataPoint(indices[0]), _reducedDimension);
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

void LayerSynthesis::VisualizeNeighbors(const PixelLayerSet &reference, const PixelLayerSet &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename)
{
	//visualize the match for the given target Pt, and the matches for the neighbors
	UINT width = target.First().pixelWeights.Cols();
	UINT height = target.First().pixelWeights.Rows();

	//initialize the base image
	Bitmap result(width + reference.First().pixelWeights.Cols(), Math::Max(height, reference.First().pixelWeights.Rows()), RGBColor(255,255,255));
	Bitmap targetImage, refImage;
	VisualizeLayers(target, targetImage);
	VisualizeLayers(reference, refImage);
	for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
			if (targetImage.ValidCoordinates(x,y))
				result[y][x] = targetImage[y][x];
	for (int x=width; x<(int)result.Width(); x++)
		for(int y=0; y<(int)result.Height(); y++)
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
	
	for (int i=0; i<indices.Length(); i++)
	{
		Vec2i sourceCoordinate = _treeCoordinates[indices[i]];		
		render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(sourceCoordinate+Vec2i(width,0), Vec2i(2, 2)), colors[i], colors[i]);
	}

	result.SavePNG(filename);
}


void LayerSynthesis::VisualizeMatches(const AppParameters &parameters, const PixelLayerSet &reference, const PixelLayerSet &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename, const Grid<Vec2i> &sourceCoordinates)
{
	//visualize the match for the given target Pt, and the matches for the neighbors
	UINT width = target.First().pixelWeights.Cols();
	UINT height = target.First().pixelWeights.Rows();

	//initialize the base image
	Bitmap result(width + reference.First().pixelWeights.Cols(), Math::Max(height, reference.First().pixelWeights.Rows()), RGBColor(255,255,255));
	Bitmap targetImage, refImage;
	VisualizeLayers(target, targetImage);
	VisualizeLayers(reference, refImage);
	for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
			if (targetImage.ValidCoordinates(x,y))
				result[y][x] = targetImage[y][x];
	for (int x=width; x<result.Width(); x++)
		for(int y=0; y<result.Height(); y++)
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

	for (int i=0; i<colors.Length(); i++)
	{
		Vec2i neighbor = targetPt + 5*deltas[i];
		
		if (neighbor.x >= 0 && neighbor.x < width && neighbor.y >=0 && neighbor.y < height)
		{
			Vec2i approximateMatchPt, coherentMatchPt;
			double nearestDist = BestApproximateMatch(neighbor, reference, target, generator, approximateMatchPt);
			double nearestCoherentDist = BestCoherentMatch(neighbor, reference, target, generator, sourceCoordinates, coherentMatchPt);

			Vec2i bestPt;
			if (nearestCoherentDist < (nearestDist + coherenceParam))
				bestPt = coherentMatchPt;
			else
				bestPt = approximateMatchPt;
			Vec2i sourceCoordinate = bestPt;

			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(neighbor, Vec2i(2, 2)), colors[i], colors[i]);
			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(sourceCoordinate+Vec2i(width,0), Vec2i(2, 2)), colors[i], colors[i]);
		}
	}
	result.SavePNG(filename);
}

void LayerSynthesis::VisualizeLayers(const PixelLayerSet &layers, Bitmap &result)
{
	int width = layers.First().pixelWeights.Cols();
	int height = layers.First().pixelWeights.Rows();
	result.Allocate(width, height);
	
	for (int x=0; x<width; x++)
	{
		for (int y=0; y<height; y++)
		{
			Vec3f value(0,0,0);
			for (UINT layerIndex=0; layerIndex<layers.Length(); layerIndex++)
				value += (float)layers[layerIndex].pixelWeights(y,x)*layers[layerIndex].color;
			result[y][x] = RGBColor(value);
		}
	}
}
