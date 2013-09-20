#pragma once
#include "Main.h"

class NeighborhoodGenerator
{
public:
	NeighborhoodGenerator(UINT neighborhoodSize, UINT numLayers);
	~NeighborhoodGenerator(void);

	void Init(UINT neighborhoodSize);
	bool Generate(const PixelLayerSet &layers, int xCenter, int yCenter, double* result) const;
	
    __forceinline UINT Dimension() const
    {
        return _dimension;
    }

private:
	int _neighborhoodSize;
	int _dimension;


};

