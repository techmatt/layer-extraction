#pragma once
#include "Main.h"

class LayerModel
{
public:
	LayerModel(void);
	~LayerModel(void);

	struct Segment
	{
		Dictionary<String, Vector<double>> features;
	


		
	};

	struct SegmentGroup
	{
		Dictionary<String, Vector<double>> features;
		set<int> members;


	};

};


