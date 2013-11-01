#pragma once

class Filter
{
	public:

		Filter(double alpha=0.4)
		{
			_data.Allocate(5, 5, 0);

			double w[] = {1, 0.25, 0.25-0.5*alpha};

			for (int i=-2; i<=2; i++)
				for (int j=-2; j<=2; j++)
					_data(i+2, j+2) = w[Math::AbsInt(i)]*w[Math::AbsInt(j)];
		};

		__forceinline const double& operator() (unsigned int Col, unsigned int Row) const
		{
			return _data(Row+2, Col+2);
		};

		int Radius()
		{
			return 2;
		}
			

	private:
		Grid<double> _data;

};

class GaussianPyramid
{
public:
	GaussianPyramid();
	GaussianPyramid(const PixelLayerSet &original, int depth, double alpha=0.4);

	void Init(const PixelLayerSet& original, int depth, double alpha=0.4);
	Vec2i TransformCoordinates(Vec2i point, int fromDepth, int toDepth) const;

	UINT NumLayers() const
	{
		if (_data.Length() > 0)
			return _data.First().Length(); 
		return 0;
	}

	__forceinline UINT Depth() const
	{
		return _data.Length();
	}

	__forceinline PixelLayerSet& operator [] (unsigned int d)
    {
		return _data[d];
	}

	__forceinline const PixelLayerSet& operator [] (unsigned int d) const
    {
		return _data[d];
	}

	__forceinline PixelLayerSet& Base()
	{
		return _data.First();
	}

	__forceinline const PixelLayerSet& Base() const
	{
		return _data.First();
	}


private:
	static double ApplyFilter(Vec2i point, Filter filter, const PixelLayer &layer);
	Vector<PixelLayerSet> _data;

};

