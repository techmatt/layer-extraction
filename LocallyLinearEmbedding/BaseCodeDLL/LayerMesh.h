#pragma once
#include "Main.h"

class LayerMesh
{
public:
	LayerMesh(const PixelLayerSet &layers);
	~LayerMesh(void);

	struct Segment
	{
		Dictionary<String, Vector<double>> features;
		Dictionary<int, pair<double, double>> adjacencies;

	};

	struct SegmentGroup
	{
		Dictionary<String, Vector<double>> features;
		Vector<int> members;
		Vec3f observedColor;
	};

	void SaveToFile(String filename);
	Vector<String> StringRepresentation();

	template<class T>
	String StringJoin(Vector<T> vector, String separator)
	{
		String result = "";
		result += vector.First();
		for (int i=1; i<vector.Length(); i++)
			result += (separator + String(vector[i]));
		return result;
	}

private:
	Vector<Segment> segments;
	Vector<SegmentGroup> groups;


};


