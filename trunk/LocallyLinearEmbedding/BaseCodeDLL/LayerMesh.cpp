#include "Main.h"
#include "LayerMesh.h"


LayerMesh::LayerMesh(const PixelLayerSet &layers)
{
	PixelLayerSet blurredLayers(layers);
	Filter filter;
	int radius = filter.Radius();
	for (int layerIndex=0; layerIndex < layers.Count; layerIndex++)
	{
		blurredLayers[layerIndex].pixelWeights.Clear(0);

		for (int x=0; x<layers[layerIndex].Width; x++)
			for (int y=0; y<layers[layerIndex].Height; y++)
				for (int dx = -radius; dx <= radius; dx++)
					for (int dy = -radius; dy<=radius; dy++)
						if (layers[layerIndex].pixelWeights.ValidCoordinates(y+dy, x+dx))
							blurredLayers[layerIndex].pixelWeights(y, x) += filter(dx, dy) * layers[layerIndex].pixelWeights(y+dy, x+dx);
	}


	// compute features for each layer
	for (int layerIndex=0; layerIndex < layers.Count; layerIndex++)
	{		
		Segment segment;

		// normalized centroid (image center is (0,0))
		Vec2f centroid(0,0);
		int layerWidth = layers[layerIndex].Width();
		int layerHeight = layers[layerIndex].Height();
		for (int r=0; r<layerHeight; r++)
			for (int c=0; c<layerWidth; c++)
				centroid += layers[layerIndex].pixelWeights(r,c)*Vec2f(c,r);
		centroid.x /= layerWidth;
		centroid.y /= layerHeight;
		
		Vector<double> centroidVec; centroidVec.PushEnd(centroid.x); centroidVec.PushEnd(centroid.y);
		segment.features.Add("RelativeCentroid", centroidVec);

		// radial distance to center
		double centerDist = Vec2f::Dist(centroid, Vec2f(0,0));
		Vector<double> centerDistVec; centerDistVec.PushEnd(centerDist);
		segment.features.Add("RadialDistance", centerDistVec);

		// average weight
		double avgWeight = layers[layerIndex].WeightMean();
		segment.features.Add("AvgWeight", Vector<double>(1,avgWeight));

		// overlap with other layers (after blurring)
		double totalOverlap = 0;
		for (int otherLayerIndex=0; otherLayerIndex < layers.Count; otherLayerIndex++)
		{
			if (otherLayerIndex == layerIndex)
				continue;
			double dotprod = blurredLayers[layerIndex].DotProduct(blurredLayers[otherLayerIndex]);
			double thisWeight = blurredLayers[layerIndex].WeightMean()*(blurredLayers[layerIndex].Width()*blurredLayers[layerIndex].Height());
			double otherWeight = blurredLayers[otherLayerIndex].WeightMean();
			double overlap = dotprod/(thisWeight*otherWeight);

			segment.adjacencies.Add(otherLayerIndex, pair<double,double>(overlap, overlap));
			totalOverlap += overlap;
		}
		Vector<int> &neighbors = segment.adjacencies.Keys();
		for (int neighborIndex=0; neighborIndex<neighbors.Count; neighborIndex++)
		{
			segment.adjacencies[neighborIndex].first /= totalOverlap;
		}

		// TODO: Implement weighted PCA
		// Compute PCA eigenvectors
		// PCA<double> pca;

		// PCA eigenvector product

		// PCA eccentricity (eigenvector division)

		// histogram of sizes at different weight values (divided by total image size)
		Vector<double> sizes(4, 0);
		double binStep = 0.25;
		for (int r=0; r<layers[layerIndex].Height(); r++)
		{
			for (int c=0; c<layers[layerIndex].Width(); c++)
			{
				double value = layers[layerIndex].pixelWeights(r,c);
				int bin = Math::Max(Math::Min(1.0, value*4), 0.0);
				sizes[bin]++;
			}
		}
		for (int i=0; i<sizes.Length; i++)
			sizes[i] /= (layers.First().Width()*layers.First().Height());

		segment.features.Add("RelativeSizes", sizes);


		// Future: bag of words features?
		segments.PushEnd(segment);
	}

	//initialize the groups, one segment per group
	for (int groupIndex=0; groupIndex < layers.Length; groupIndex++)
	{
		SegmentGroup group;
		group.members.PushEnd(groupIndex);
		group.observedColor = layers[groupIndex].color;
		groups.PushEnd(group);
	}
}


Vector<String> LayerMesh::StringRepresentation()
{
	Vector<String> lines;
	//write all segments
	for (int segmentIndex=0; segmentIndex < segments.Length; segmentIndex++)
	{
		lines.PushEnd("SegmentBegin");
		Segment &segment = segments[segmentIndex];
		Vector<String> &featureNames = segment.features.Keys();
		for (String f : featureNames)
			lines.PushEnd(f+" "+ StringJoin(segment.features[f], " "));
		
		Vector<int> adjIds = segment.adjacencies.Keys();
		Vector<String> adjacencyStrings;
		for (int nid:adjIds)
			adjacencyStrings.PushEnd(String(segment.adjacencies[nid].first) + "^" + String(segment.adjacencies[nid].second) + "^" + String(segment.adjacencies[nid].second));

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



