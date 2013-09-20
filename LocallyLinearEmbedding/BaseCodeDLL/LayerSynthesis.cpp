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
			for(int i = 0; i < dimension; i++)
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
	//TODO: select mask/window for the reference layers?

	_reducedDimension = reducedDimension;

	const UINT width = layers.First().pixelWeights.Cols();
	const UINT height = layers.First().pixelWeights.Rows();
    const UINT dimension = generator.Dimension();

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
    _tree.BuildTree(allNeighborhoods, reducedDimension, 1);
    allNeighborhoods.DeleteMemory();
}

PixelLayerSet LayerSynthesis::Synthesize(const AppParameters &parameters, const PixelLayerSet &reference, const PixelLayerSet &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule, NeighborhoodGenerator &generator)
{
	//original - the original (incomplete) set of layers
	//pixels - the region drawn by the user, where a new layer(s) should be synthesized (including color information).
	//returns a new set of layers (the original layers might be modified a bit to take into account the new layer(s))

    //...
	Console::WriteLine("Synthesizing layers...");
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
			VisualizeMatches(reference, target, pixels[pixelIndex], generator, "NMatch_"+String(stepIdx++)+"_iter"+String(row)+".png");

        SynthesizeStepInPlace(parameters, reference, target, pixels, updateSchedule.ExtractRow(row), generator);
		for (UINT i=0; i<target.Length(); i++)
		{
			target[i].SavePNG("layer_"+String(i)+"_iter"+String(row+1)+".png");
		}
    }
	Console::WriteLine("Done!");
    return target;
}

void LayerSynthesis::SynthesizeStepInPlace(const AppParameters &parameters, const PixelLayerSet &reference, PixelLayerSet &target, const Vector<Vec2i> &pixels, const Vector<double> &updateSchedule, NeighborhoodGenerator &generator)
{
    //Math::Lerp(start, end, 1.0);
	//find the pixel with the nearest transformed neighborhood, and replace with that
	UINT layerCount = target.Length();
	UINT width = target.First().pixelWeights.Cols();
	UINT height = target.First().pixelWeights.Rows();
	UINT dimension = generator.Dimension();

	double* neighborhood = new double[dimension]; 
	double* transformedNeighborhood = new double[_reducedDimension];
	Vector<UINT> indices;

	for (UINT pixelIndex=0; pixelIndex < pixels.Length(); pixelIndex++)
	{
		int x = pixels[pixelIndex].x;
		int y = pixels[pixelIndex].y;

		generator.Generate(target, x, y, neighborhood);
		_pca.Transform(transformedNeighborhood, neighborhood, _reducedDimension);

		//find nearest pixel neighborhood			
		_tree.KNearest(transformedNeighborhood, 1, indices, 0.0f);
		Vec2i sourceCoordinate = _treeCoordinates[indices[0]];

		for (UINT layerIndex=0; layerIndex < layerCount; layerIndex++)
		{
			double weight = reference[layerIndex].pixelWeights(sourceCoordinate.y, sourceCoordinate.x);
			double start = target[layerIndex].pixelWeights(y,x);

			target[layerIndex].pixelWeights(y,x) = Math::Lerp(start, weight, updateSchedule[layerIndex]);
		}
	}



	delete[] neighborhood;
    delete[] transformedNeighborhood;

}


void LayerSynthesis::VisualizeMatches(const PixelLayerSet &reference, const PixelLayerSet &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename)
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

	for (int i=0; i<colors.Length(); i++)
	{
		Vec2i neighbor = targetPt + 5*deltas[i];
		if (neighbor.x >= 0 && neighbor.x < width && neighbor.y >=0 && neighbor.x < height)
		{
			generator.Generate(target, neighbor.x, neighbor.y, neighborhood);
			_pca.Transform(transformedNeighborhood, neighborhood, _reducedDimension);

			//find nearest pixel neighborhood			
			_tree.KNearest(transformedNeighborhood, 1, indices, 0.0f);
			Vec2i sourceCoordinate = _treeCoordinates[indices[0]];

			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(neighbor, Vec2i(2, 2)), colors[i], colors[i]);
			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(sourceCoordinate+Vec2i(width,0), Vec2i(2, 2)), colors[i], colors[i]);
		}
	}

	result.SavePNG(filename);
}

void LayerSynthesis::VisualizeLayers(const PixelLayerSet &layers, Bitmap &result)
{
	UINT width = layers.First().pixelWeights.Cols();
	UINT height = layers.First().pixelWeights.Rows();
	result.Allocate(width, height);
	
	for (int x=0; x<width; x++)
	{
		for (int y=0; y<height; y++)
		{
			Vec3f value(0,0,0);
			for (int layerIndex=0; layerIndex<layers.Length(); layerIndex++)
				value += layers[layerIndex].pixelWeights(y,x)*layers[layerIndex].color;
			result[y][x] = RGBColor(value);
		}
	}
}
