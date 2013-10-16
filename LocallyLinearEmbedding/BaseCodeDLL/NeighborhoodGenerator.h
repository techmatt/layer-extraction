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
	bool Generate(const GaussianPyramid &pyramid, int level, int xCenter, int yCenter, double* result) const; // only single level of pyramid, for texture synthesis
	bool Generate(const GaussianPyramid &pyramid, const Grid<Vec2i> &coordinates, int level, int width, int height, int xCenter, int yCenter, double* result) const; // generate from coordinates, for texture synthesis
	bool Generate(const PixelLayerSet &layers, const Vector<int> &order, int iteration, int xCenter, int yCenter, double* result) const; // synthesis by layer

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

