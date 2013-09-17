#pragma once
#include "Main.h"

class NeighborhoodGenerator
{
public:
	NeighborhoodGenerator(UINT neighborhoodSize);
	~NeighborhoodGenerator(void);

	void Init(UINT neighborhoodSize);
	bool Generate(const PixelLayerSet &layers, int xCenter, int yCenter, float* result) const;

private:
	UINT _neighborhoodSize;
	UINT _dimension;


};

