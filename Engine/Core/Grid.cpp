/*
Grid.cpp
Written by Matthew Fisher
*/

#pragma once

#include "Grid.h"

template <class type> void Grid<type>::Allocate(unsigned int Rows, unsigned int Cols)
{
	_Rows = Rows;
	_Cols = Cols;
	if(_Data)
	{
		delete[] _Data;
	}
	_Data = new type[Rows * Cols];
}

template <class type> void Grid<type>::AllocateIfNeeded(unsigned int Rows, unsigned int Cols)
{
	if(_Rows != Rows || _Cols != Cols)
	{
		_Rows = Rows;
		_Cols = Cols;
		if(_Data)
		{
			delete[] _Data;
		}
		_Data = new type[Rows * Cols];
	}
}

template <class type> void Grid<type>::Allocate(unsigned int Rows, unsigned int Cols, const type &clearValue)
{
	_Rows = Rows;
	_Cols = Cols;
	if(_Data)
	{
		delete[] _Data;
	}
	_Data = new type[Rows * Cols];
	Clear(clearValue);
}

template <class type> void Grid<type>::Clear(const type &T)
{
	const unsigned int TotalEntries = _Rows * _Cols;
	for(unsigned int Index = 0; Index < TotalEntries; Index++)
	{
		_Data[Index] = T;
	}
}

template <class type> Vector<type> Grid<type>::MakeRowScanlineVector() const
{
	Vector<type> result(_Rows * _Cols);
	UINT resultIndex = 0;
	for(unsigned int rowIndex = 0; rowIndex < _Rows; rowIndex++)
	{
		for(unsigned int colIndex = 0; colIndex < _Cols; colIndex++)
		{
			result[resultIndex++] = _Data[rowIndex * _Cols + colIndex];
		}
	}
	return result;
}

template <class type> Vec2i Grid<type>::MaxIndex() const
{
	Vec2i maxIndex(0, 0);
	const type *maxValue = &_Data[0];
	for(unsigned int rowIndex = 0; rowIndex < _Rows; rowIndex++)
	{
		for(unsigned int colIndex = 0; colIndex < _Cols; colIndex++)
		{
			const type *curValue = &_Data[rowIndex * _Cols + colIndex];
			if(*curValue > *maxValue)
			{
				maxIndex = Vec2i(rowIndex, colIndex);
				maxValue = curValue;
			}
		}
	}
	return maxIndex;
}

template <class type> const type& Grid<type>::MaxValue() const
{
	Vec2i index = MaxIndex();
	return _Data[index.x * _Cols + index.y];
}

template <class type> Vec2i Grid<type>::MinIndex() const
{
	Vec2i minIndex(0, 0);
	const type *minValue = &_Data[0];
	for(unsigned int rowIndex = 0; rowIndex < _Rows; rowIndex++)
	{
		for(unsigned int colIndex = 0; colIndex < _Cols; colIndex++)
		{
			const type *curValue = &_Data[rowIndex * _Cols + colIndex];
			if(*curValue < *minValue)
			{
				minIndex = Vec2i(rowIndex, colIndex);
				minValue = curValue;
			}
		}
	}
	return minIndex;
}

template <class type> const type& Grid<type>::MinValue() const
{
	Vec2i index = MinIndex();
	return _Data[index.x * _Cols + index.y];
}

template <class type> Grid<type>::Grid()
{
	_Rows = 0;
	_Cols = 0;
	_Data = NULL;
}

template <class type> Grid<type>::Grid(unsigned int Rows, unsigned int Cols)
{
	_Rows = Rows;
	_Cols = Cols;
	_Data = new type[Rows * Cols];
}

template <class type> Grid<type>::Grid(unsigned int Rows, unsigned int Cols, const type &clearValue)
{
	_Rows = Rows;
	_Cols = Cols;
	_Data = new type[Rows * Cols];
	Clear(clearValue);
}

template <class type> Grid<type>::Grid(const Grid<type> &G)
{
	_Rows = G._Rows;
	_Cols = G._Cols;

	const unsigned int TotalEntries = _Rows * _Cols;
	_Data = new type[TotalEntries];
	for(unsigned int Index = 0; Index < TotalEntries; Index++)
	{
		_Data[Index] = G._Data[Index];
	}
}

template <class type> Grid<type>::Grid(Grid<type> &&G)
{
	_Rows = G._Rows;
	_Cols = G._Cols;
	_Data = G._Data;

	G._Rows = 0;
	G._Cols = 0;
	G._Data = NULL;
}

template <class type> Grid<type>::~Grid()
{
	FreeMemory();
}

template <class type> void Grid<type>::FreeMemory()
{
	_Rows = 0;
	_Cols = 0;
	if(_Data != NULL)
	{
		delete[] _Data;
		_Data = NULL;
	}
}

template <class type> Grid<type>& Grid<type>::operator = (const Grid<type> &G)
{
	if(_Data)
	{
		delete[] _Data;
	}
	_Rows = G._Rows;
	_Cols = G._Cols;

	const unsigned int TotalEntries = _Rows * _Cols;
	_Data = new type[TotalEntries];
	for(unsigned int Index = 0; Index < TotalEntries; Index++)
	{
		_Data[Index] = G._Data[Index];
	}

	return *this;
}

// assumes small filter, boundaries...
template <class type> void Grid<type>::Convolve(const Grid<double>& filter)
{
	Assert(filter.Cols() % 2 != 0 && filter.Rows() % 2 != 0, "filter dimensions must be odd");
	Grid<type> copy(*this);

	int xradius = (int) (filter.Cols() / 2);
	int yradius = (int) (filter.Rows() / 2);

	Grid<type> debug(_Rows, _Cols, 0);

	double sum;
	//int nx, ny;
	for (int y = 0; y < (int)_Rows; y++) {
		for (int x = 0; x < (int)_Cols; x++) {

			sum = 0;
			for (int j = -yradius; j <= yradius; j++) {
				for (int i = -xradius; i <= xradius; i++) {

					//ny = (y + j + _Rows) % _Rows; 
					//nx = (x + i + _Cols) % _Cols;
					//Assert(ValidCoordinates(ny, nx), "coordinates out of bounds");
					if (ValidCoordinates(y+j, x+i))
						sum += filter(j+yradius, i+xradius) * copy(y+j, x+i); //copy(ny, nx);

				}
			}
			_Data[y * _Cols + x] = (type) sum;
		}
	}
}

template <class type> void Grid<type>::PercentileFilter(int grid_radius, float percentile)
{
	Assert(grid_radius >= 0, "filter radius must be non-negative");
	Grid<type> copy(*this);

	int grid_radius_squared = grid_radius * grid_radius;
	int max_samples = (2*grid_radius+1) * (2*grid_radius+1);
	type *samples = new type [max_samples];
	int r = grid_radius;

	// set every sample to be kth percentile of surrounding region in input grid
	for (int cy = 0; cy < (int)_Rows; cy++) {
		for (int cx = 0; cx < (int)_Cols; cx++) {
			// build list of grid values in neighborhood
			int nsamples = 0;
			int ymin = max(0, cy - r);
			int ymax = min((int)_Rows-1, cy + r);
			for (int y = ymin; y <= ymax; y++) {
				int xmin = max(0, cx - r);
				int xmax = min((int)_Cols-1, cx + r);
				int dy = y - cy;
				for (int x = xmin; x <= xmax; x++) {
					int dx = x - cx;
					int d_squared = dx*dx + dy*dy;
					if (d_squared > grid_radius_squared) continue;
					samples[nsamples++] = copy(y,x);
				}
			}

			if (nsamples > 0) {
				// sort samples found in neighborhood
				std::sort(samples, samples + nsamples);

				// set grid value to percentile of neighborhood
				int index = (int) (percentile * nsamples);
				if (index < 0) index = 0;
				else if (index >= nsamples) index = nsamples-1;
				_Data[cy * _Cols + cx] = samples[index];
			}
		}
	}

	// clean up
	delete [] samples;
}

template <class type> void Grid<type>::BilateralFilter(double grid_sigma, double value_sigma)
{
	Grid<type> copy(*this);

	// determine reasonable value sigma
	if (value_sigma == -1)
		value_sigma = 0.01 * (MaxValue() - MinValue());

	Console::WriteLine("bilateral filter with grid_sigma = " + String(grid_sigma) + " and value_sigma = " + String(value_sigma));

	double grid_denom = 2.0 * grid_sigma * grid_sigma;
	double value_denom = 2.0 * value_sigma * value_sigma;
	double grid_radius = 3 * grid_sigma;
	int r = (int) (grid_radius + 1);
	int r_squared = r * r;

	// set every sample to be filter of surrounding region in input grid
	double sum, weight;
	for (int cy = 0; cy < (int)_Rows; cy++) {
		for (int cx = 0; cx < (int)_Cols; cx++) {
			sum = 0;
			weight = 0;
			int ymin = max(0, cy - r);
			int ymax = min((int)_Rows-1, cy + r);
			for (int y = ymin; y <= ymax; y++) {
				int xmin = max(0, cx - r);
				int xmax = min((int)_Cols-1, cx + r);
				int dy = y - cy;
				for (int x = xmin; x <= xmax; x++) {
					int dx = x - cx;
					int grid_distance_squared = dx*dx + dy*dy;
					if (grid_distance_squared > r_squared) continue;
					double value_distance_squared = (double) (copy(cy, cx) - copy(y, x));
					value_distance_squared *= value_distance_squared;
					double w = exp(-grid_distance_squared/grid_denom) * exp(-value_distance_squared/value_denom);
					sum += w * copy(y, x);
					weight += w;
				}
			}

			if (weight != 0) _Data[cy * _Cols + cx] = (type) (sum / weight);
		}
	}
}

template <class type> void Grid<type>::GaussianBlur(double sigma, double scale_factor)
{
	// build filter
	int filter_radius = (int) (3 * sigma + 0.5);
	double *filter = new double[filter_radius + 1];

	// buffer for temporary copy of row/col
	unsigned int size = max(_Rows, _Cols);
	type *buffer = new type[size];

	// gaussian 
	double a = sqrt(Math::PI * 2) * sigma;
	double fac = 1.0 / (a * a * a);
	double denom = 2.0 * sigma * sigma;
	double normalization = 0;
	for (int i = 0; i <= filter_radius; i++) {
		filter[i] = fac * exp(-i * i / denom);
		normalization += filter[i];
	}
	for (int i = 0; i <= filter_radius; i++)
		filter[i] *= scale_factor / normalization;

	// convolve in x direction
	double sum, weight;
	for (int j = 0; j < (int)_Rows; j++) { 
		for (int i = 0; i < (int)_Cols; i++) 
			buffer[i] = _Data[j * (int)_Cols + i]; 
		for (int i = 0; i < (int)_Cols; i++) {
			sum = filter[0] * buffer[i];
			weight = filter[0];
			int nsamples = i;
			if (nsamples > filter_radius) nsamples = filter_radius;
			for (int m = 1; m <= nsamples; m++) {
				sum += filter[m] * buffer[i - m];
				weight += filter[m];
			}
			nsamples = (int)_Cols - 1 - i;
			if (nsamples > filter_radius) nsamples = filter_radius;
			for (int m = 1; m <= nsamples; m++) {
				sum += filter[m] * buffer[i + m];
				weight += filter[m];
			}
			if (weight > 0) _Data[j * (int)_Cols + i] = (type) sum;//(sum / weight);
		}
	}

	// convolve in y direction
	for (int j = 0; j < (int)_Cols; j++) { 
		for (int i = 0; i < (int)_Rows; i++) 
			buffer[i] = _Data[i * (int)_Cols + j]; 
		for (int i = 0; i < (int)_Rows; i++) {
			sum = filter[0] * buffer[i];
			weight = filter[0];
			int nsamples = i;
			if (nsamples > filter_radius) nsamples = filter_radius;
			for (int m = 1; m <= nsamples; m++) {
				sum += filter[m] * buffer[i - m];
				weight += filter[m];
			}
			nsamples = (int)_Rows - 1 - i;
			if (nsamples > filter_radius) nsamples = filter_radius;
			for (int m = 1; m <= nsamples; m++) {
				sum += filter[m] * buffer[i + m];
				weight += filter[m];
			}
			if (weight > 0) _Data[i * (int)_Cols + j] = (type) sum; //(sum / weight);
		}
	}

	// clean up
	delete [] filter;
	delete [] buffer;
}

template <class type> void Grid<type>::Laplacian(void)
{
	// build filter
	Grid<double> filter(3, 3);
	filter(0,0) = -0.125;    filter(0,1) = -0.125;    filter(0,2) = -0.125;
	filter(1,0) = -0.125;    filter(1,1) = 1;         filter(1,2) = -0.125;
	filter(2,0) = -0.125;    filter(2,1) = -0.125;    filter(2,2) = -0.125;

	Convolve(filter);
}

template <class type> void Grid<type>::Emboss(void)
{
	// build filter
	Grid<double> filter(3, 3);
	filter(0,0) = -1;    filter(0,1) = -1;    filter(0,2) =  0;
	filter(1,0) = -1;    filter(1,1) =  0;    filter(1,2) =  1;
	filter(2,0) =  0;    filter(2,1) =  1;    filter(2,2) =  1;

	Convolve(filter);
}

template <class type> void Grid<type>::Sharpen(void)
{
	// build filter
	Grid<double> filter(3, 3);
	filter(0,0) = -1;    filter(0,1) = -1;    filter(0,2) = -1;
	filter(1,0) = -1;    filter(1,1) =  9;    filter(1,2) = -1;
	filter(2,0) = -1;    filter(2,1) = -1;    filter(2,2) = -1;

	Convolve(filter);
}

// super lazy motion blur
template <class type> void Grid<type>::MotionBlur(const Vec2f& direction, int size, double scale_factor)
{
	Assert(size > 0 && size % 2 != 0, "filter dimensions must be positive & odd"); // because lazy...

	Grid<type> copy(*this);

	// linear ramp in direction of length size
	int radius = size / 2;
	double *filter = new double[size];
	//double normalization = size * (size + 1) / 2.0;
	for (int i = 0; i < size; i++)
		filter[i] = (i + 1); // / normalization;

	Vec2f dir = Vec2f::Normalize(direction);

	double sum, weight;
	for (int cy = 0; cy < (int)_Rows; cy++) {
		for (int cx = 0; cx < (int)_Cols; cx++) {

			sum = 0;
			weight = 0;
			for (int i = -radius; i <= radius; i++) {
				int nx = (int) (cx + direction.x * i);
				int ny = (int) (cy + direction.y * i);
				if (ValidCoordinates(ny, nx)) {
					sum += copy(ny, nx) * filter[i+radius];
					weight += filter[i+radius];
				}
			}
			if (weight > 0) _Data[cy * (int)_Cols + cx] = (type) (sum * scale_factor / weight);

		}
	}

	delete [] filter;
}

template <class type> void Grid<type>::Erode(double sigma)
{
	/*Grid<type> invert(_Rows, _Cols);
	for (unsigned int r = 0; r < _Rows; r++) {
	for (unsigned int c = 0; c < _Cols; c++) {
	invert(r,c) = 1 - _Data[r * _Cols + c];
	}
	}
	invert.GaussianBlur(sigma, 1.0);
	for (unsigned int r = 0; r < _Rows; r++) {
	for (unsigned int c = 0; c < _Cols; c++) {
	if (_Data[r * _Cols + c] >= 0) {
	if (_Data[r * _Cols + c] > invert(r,c))
	_Data[r * _Cols + c] -= invert(r,c);
	else
	_Data[r * _Cols + c] = 0;
	}
	}
	}*/
	Grid<type> copy(*this);

	// keep only pixels at least distance from some zero pixel
	Threshold(1.0E-20, 1, 0);
	SquaredDistanceTransform();
	Threshold(sigma * sigma, 0, 1);
}

template <class type> void Grid<type>::Threshold(double threshold, double low, double high)
{
	// Set grid value to low (high) if less/equal (greater) than threshold
	unsigned int index;
	for (unsigned int r = 0; r < _Rows; r++) {
		for (unsigned int c = 0; c < _Cols; c++) {
			index = r * _Cols + c;
			if (_Data[index] <= threshold)
				_Data[index] = low;
			else
				_Data[index] = high;
		}
	}
}

template <class type> void Grid<type>::SquaredDistanceTransform(void)
{
	int x, y, s, t;
	int dist, square, new_dist;
	int first;
	int i;

	// Allocate temporary buffers
	unsigned int res = max(_Rows, _Cols);
	Vector<int> oldBuffer(res);
	Vector<int> newBuffer(res);

	// Initalize values (0 if was set, max_value if not)
	type max_value = (type) 2 * (res+1) * (res+1);
	unsigned int index;
	for (unsigned int r = 0; r < _Rows; r++) {
		for (unsigned int c = 0; c < _Cols; c++) {
			index = r * _Cols + c;
			if (_Data[index] == 0.0) _Data[index] = max_value;
			else _Data[index] = 0.0;
		}
	}

	// Scan along x axis
	for (y = 0; y < (int)_Rows; y++) {
		first = 1;
		dist = 0;
		for (x = 0; x < (int)_Cols; x++) {
			if (_Data[y * _Cols + x] == 0.0) {
				dist=0;
				first=0;
			}
			else if (first == 0) {
				dist++;
				square = dist*dist;
				_Data[y * _Cols + x] =  square;
			}
		}

		// backward scan
		dist = 0;
		first = 1;
		for (x = (int)_Cols-1; x >= 0; x--) {
			if (_Data[y * _Cols + x] == 0.0) {
				dist = 0;
				first = 0;
			}
			else if (first == 0) {
				dist++;
				square = dist*dist;
				if (square < _Data[y * _Cols + x]) {
					_Data[y * _Cols + x] = square;
				}
			}
		}
	}

	// Scan along y axis
	for (x = 0; x < (int)_Cols; x++) {
		// Copy grid values
		for (y = 0; y < (int)_Rows; y++) 
			oldBuffer[y] = (int) (_Data[y * _Cols + x] + 0.5);

		// forward scan
		s = 0;
		for (y = 0; y < (int)_Rows; y++) {
			dist = oldBuffer[y];
			if (dist) {
				for (t = s; t <= y ; t++) {
					new_dist = oldBuffer[t] + (y - t) * (y - t);
					if (new_dist <= dist){
						dist = new_dist;
						s = t;
					}
				}
			}
			else { 
				s = y;
			}
			newBuffer[y] = dist;
		}

		// backward scan
		s = (int)_Rows - 1;
		for(y = (int)_Rows-1; y >=0 ; y--) {
			dist = newBuffer[y];
			if (dist) {
				for (t = s; t > y ; t--) {
					new_dist = oldBuffer[t] + (y - t) * (y - t);
					if (new_dist <= dist){
						dist = new_dist;
						s = t;
					}
				}
				_Data[y * _Cols + x] = dist;
			}
			else { 
				s = y; 
			}
		}
	}
}