#include "Main.h"
#include "LayerMesh.h"


LayerMesh::LayerMesh(const PixelLayerSet &layers)
{
	PixelLayerSet blurredLayers(layers);
	Filter filter;
	int radius = filter.Radius();
	for (UINT layerIndex=0; layerIndex < layers.Length(); layerIndex++)
	{
		blurredLayers[layerIndex].pixelWeights.Clear(0);

		for (int x=0; x<layers[layerIndex].Width(); x++)
			for (int y=0; y<layers[layerIndex].Height(); y++)
				for (int dx = -radius; dx <= radius; dx++)
					for (int dy = -radius; dy<=radius; dy++)
						if (layers[layerIndex].pixelWeights.ValidCoordinates(y+dy, x+dx))
							blurredLayers[layerIndex].pixelWeights(y, x) += filter(dx, dy) * layers[layerIndex].pixelWeights(y+dy, x+dx);
	}

	_blurredLayers = blurredLayers;
	_layers = layers;

	// compute features for each layer
	for (UINT layerIndex=0; layerIndex < layers.Length(); layerIndex++)
	{		
		const PixelLayer &currLayer = layers[layerIndex];
		int layerWidth = currLayer.Width();
		int layerHeight = currLayer.Height();
		Console::WriteLine("Width "+String(layerWidth)+" Height "+String(layerHeight));

		Segment segment;

		Console::WriteLine("Centroid");
		// normalized centroid (image center is (0,0))
		Vec2f centroid(0,0);

		for (int r=0; r<layerHeight; r++)
			for (int c=0; c<layerWidth; c++)
				centroid += currLayer.pixelWeights(r,c)*Vec2f(c,r);
		centroid.x /= (layerWidth*layerHeight)*layerWidth;
		centroid.y /= (layerWidth*layerHeight)*layerHeight;
		
		Vector<double> centroidVec; centroidVec.PushEnd(centroid.x); centroidVec.PushEnd(centroid.y);
		segment.features.Add("RelativeCentroid", centroidVec);

		Console::WriteLine("Distance");
		// radial distance to center
		double centerDist = Vec2f::Dist(centroid, Vec2f(0,0));
		Vector<double> centerDistVec; centerDistVec.PushEnd(centerDist);
		segment.features.Add("RadialDistance", centerDistVec);

		Console::WriteLine("Weight");
		// average weight
		double avgWeight = currLayer.WeightMean();
		segment.features.Add("RelativeSize", Vector<double>(1,avgWeight));

		Console::WriteLine("Overlap");
		// overlap with other layers (after blurring)
		double totalOverlap = 0;
		for (UINT otherLayerIndex=0; otherLayerIndex < layers.Length(); otherLayerIndex++)
		{
			if (otherLayerIndex == layerIndex)
				continue;
			int numPixels = layerWidth*layerHeight;
			double dotprod = blurredLayers[layerIndex].DotProduct(blurredLayers[otherLayerIndex]);
			double thisWeight = blurredLayers[layerIndex].WeightMean()*numPixels;
			double otherWeight = blurredLayers[otherLayerIndex].WeightMean()*numPixels;
			double overlap = dotprod/(thisWeight*otherWeight);
			segment.adjacencies.Add(otherLayerIndex, pair<double,double>(overlap, overlap));
			totalOverlap += overlap;
		}
		Console::WriteLine("Add adjacency");
		Vector<int> &neighbors = segment.adjacencies.Keys();
		for (int neighbor:neighbors)
			segment.adjacencies[neighbor].first /= totalOverlap;

		// Compute PCA 
		PCA<double> pca;
		int numIntervals = 10000;

		// for every pixel, assign a range
		pair<double, double>* ranges = new pair<double,double>[layerWidth*layerHeight];

		double currentStart = 0;
		// populate ranges
		for (int y=0; y<layerHeight; y++)
		{
			for (int x=0; x<layerWidth; x++)
			{
				int idx = y*layerWidth + x;
				ranges[idx].first = currentStart;
				ranges[idx].second = currentStart + Math::Max(currLayer.pixelWeights(y,x),0.0);

				currentStart = ranges[idx].second;
				//Console::WriteLine(String(ranges[idx].first)+"-"+String(ranges[idx].second));
			}
		}

		double totalWeight = currLayer.TotalNonNegativeWeight()+0.001;
		double start = 0;
		double end = totalWeight;
		double step = totalWeight/numIntervals;
		int pixelIndex = 0;

		Vector<const double*> samples;
		PixelLayer sampleViz(Vec3f(0,0,0), layerWidth, layerHeight);

		int samplesCount = 0;
		while (start < end)
		{
			double sampleWeight = ((double)rand()/(double)RAND_MAX)*step+start;
			
			bool inrange = false;
			while (!inrange)
			{
				if (pixelIndex >= (layerWidth*layerHeight))
					break;
				if (ranges[pixelIndex].first <= sampleWeight && sampleWeight < ranges[pixelIndex].second)
				{
					inrange = true;
					break;
				}
				pixelIndex++;
			}
			int x = pixelIndex%layerWidth;
			int y = pixelIndex/layerWidth;
			
			
			double* sample = new double[2];
			sample[0] = (double)x;
			sample[1] = (double)y;

			samples.PushEnd(sample);

			sampleViz.pixelWeights(y,x) += 1;
			samplesCount++;
			start += step;
		}
		sampleViz.SavePNG("../ColorSuggestions/Samples_"+String(layerIndex)+".png");

		pca.InitFromDensePoints(samples, 2);

		// PCA eccentricity (eigenvector division)
		double firstAxis = pca.GetEigenvalue(0);
		double secondAxis = pca.GetEigenvalue(1);
		double eccentricity = min(firstAxis,secondAxis)/max(firstAxis, secondAxis);
		segment.features.Add("Eccentricity", Vector<double>(1,eccentricity));


		//cleanup
		delete[] ranges;


		Console::WriteLine("Sizes");
		// histogram of sizes at different weight values (divided by total image size)
		Vector<double> sizes(4, 0);
		for (int r=0; r<layerHeight; r++)
		{
			for (int c=0; c<layerWidth; c++)
			{
				double value = currLayer.pixelWeights(r,c);
				int bin = Math::Min(Math::Max(0.0, value*4-1), 3.0);
				sizes[bin]++;
			}
		}
		for (UINT i=0; i<sizes.Length(); i++)
			sizes[i] /= (layerWidth*layerHeight);

		segment.features.Add("RelativeSizes", sizes);

		segment.features.Add("Label", Vector<double>(3,0));


		// Future: bag of words features?
		segments.PushEnd(segment);
	}

	//initialize the groups, one segment per group
	for (UINT groupIndex=0; groupIndex < layers.Length(); groupIndex++)
	{
		SegmentGroup group;
		group.members.PushEnd(groupIndex);
		group.observedColor = layers[groupIndex].color;

		//dummy size feature
		group.features.Add("RelativeSize", segments[groupIndex].features["RelativeSize"]);
		groups.PushEnd(group);
	}
	VisualizeContrast();

}

void LayerMesh::VisualizeContrast()
{
	//blur each layer, then take the diff between each pair of layers
	int width = _layers.First().Width();
	int height = _layers.First().Height();

	for (int layerIndex = 0; layerIndex < _layers.Length(); layerIndex++)
	{
		for (int otherLayerIndex = layerIndex+1; otherLayerIndex < _layers.Length(); otherLayerIndex++)
		{
			PixelLayer diff(Vec3f(0,0,0), width, height);
			PixelLayer overlap(Vec3f(0,0,0), width, height);
			for (int x=0; x<width; x++)
				for (int y=0; y<height; y++)
				{
					diff.pixelWeights(y,x) = fabs(_blurredLayers[layerIndex].pixelWeights(y,x)-_blurredLayers[otherLayerIndex].pixelWeights(y,x));
					overlap.pixelWeights(y,x) = _blurredLayers[layerIndex].pixelWeights(y,x)*_blurredLayers[otherLayerIndex].pixelWeights(y,x);
				}
			diff.SavePNG("../ColorSuggestions/Contrast_"+String(layerIndex)+"_"+String(otherLayerIndex)+".png");
			overlap.SavePNG("../ColorSuggestions/Overlap_"+String(layerIndex)+"_"+String(otherLayerIndex)+".png");
		}
		//save the blurred layers
		_blurredLayers[layerIndex].SavePNG("../ColorSuggestions/Blurred_"+String(layerIndex)+".png");
	}
}


Vector<String> LayerMesh::StringRepresentation()
{
	Vector<String> lines;
	//write all segments
	for (UINT segmentIndex=0; segmentIndex < segments.Length(); segmentIndex++)
	{
		lines.PushEnd("SegmentBegin");
		Segment &segment = segments[segmentIndex];
		Vector<String> &featureNames = segment.features.Keys();
		for (String f : featureNames)
			lines.PushEnd(f+" "+ StringJoin(segment.features[f], " "));
		
		Vector<int> adjIds = segment.adjacencies.Keys();
		Vector<String> adjacencyStrings;
		for (int nid:adjIds)
			adjacencyStrings.PushEnd(String(nid)+ "^" + String(segment.adjacencies[nid].first) + "^" + String(segment.adjacencies[nid].second) + "^" + String(segment.adjacencies[nid].second));

		lines.PushEnd("AdjacentTo "+ StringJoin(adjacencyStrings, " "));
		lines.PushEnd("SegmentEnd");
	}
	lines.PushEnd("");

	//write all groups
	for (SegmentGroup &group: groups)
	{
		lines.PushEnd("GroupBegin");
		Vector<String> &featureNames = group.features.Keys();
		for (String f : featureNames)
			lines.PushEnd(f+" "+ StringJoin(group.features[f], " "));
		lines.PushEnd("ObservedColor " + String(group.observedColor.x) +" " + String(group.observedColor.y) + " " + String(group.observedColor.z));
		lines.PushEnd("Members " + StringJoin(group.members, " "));
		lines.PushEnd("GroupEnd");

	}

	return lines;

}


void LayerMesh::SaveToFile(String filename)
{
	Console::WriteLine("Saving mesh to " + filename);
	Vector<String> lines = StringRepresentation();
	ofstream File(filename.CString());
    PersistentAssert(!File.fail(), "Failed to open file");
    for(const String &line:lines)
        File << line << endl;
	File.close();
}



LayerMesh::~LayerMesh(void)
{
}



