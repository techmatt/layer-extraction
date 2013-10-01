#pragma once
#include "Main.h"

class NeighborhoodGenerator
{
public:
	NeighborhoodGenerator(UINT neighborhoodSize, UINT numLayers, UINT depth);
	~NeighborhoodGenerator(void);

	void Init(UINT neighborhoodSize);
	//bool Generate(const PixelLayerSet &layers, int xCenter, int yCenter, double* result) const;

	bool Generate(const GaussianPyramid &layers, int xCenter, int yCenter, double* result) const;
	
    __forceinline UINT Dimension() const
    {
        return _dimension;
    };

	__forceinline UINT NeighborhoodRadius() const
	{
		return _neighborhoodSize;
	}

private:
	int _neighborhoodSize;
	int _dimension;


};

