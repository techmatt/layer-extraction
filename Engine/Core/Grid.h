/*
Grid.h
Written by Matthew Fisher

Arbitrary-type grid structure.
*/

#pragma once

template <class type> class Grid
{
public:
	Grid();
	Grid(unsigned int Rows, unsigned int Cols);
	Grid(unsigned int Rows, unsigned int Cols, const type &clearValue);
	Grid(const Grid<type> &G);
	Grid(Grid<type> &&G);

	~Grid();

	// filtering operations
	void GaussianBlur(double sigma, double scale_factor);
	void BilateralFilter(double grid_sigma = 2, double value_sigma = -1);
	void PercentileFilter(int grid_radius, float percentile);
	void MedianFilter(int grid_radius) { PercentileFilter(grid_radius, 0.5f); }
	void MotionBlur(const Vec2f& direction, int size, double scale_factor);
	void Laplacian(void);
	void Emboss(void);
	void Sharpen(void);
	void Convolve(const Grid<double>& filter);
	void Erode(double sigma);
	void Threshold(double threshold, double low, double high);
	void SquaredDistanceTransform(void);

	//
	// Memory
	//
	void FreeMemory();
	Grid<type>& operator = (const Grid<type> &G);
	void Allocate(unsigned int Rows, unsigned int Cols);
	void AllocateIfNeeded(unsigned int Rows, unsigned int Cols);
	void Allocate(unsigned int Rows, unsigned int Cols, const type &clearValue);

	__forceinline Grid<type>& Grid<type>::operator += (const Grid<type> &Right)
	{
		Assert(_Rows == Right.Rows() && _Cols == Right.Cols(), "grid dimensions must be equal");
		for (unsigned int r = 0; r < _Rows; r++) {
			for (unsigned int c = 0; c < _Cols; c++)
				_Data[r * _Cols + c] += Right(r,c);
		}
		return *this;
	}
	__forceinline Grid<type>& Grid<type>::operator *= (type Right)
	{
		for (unsigned int r = 0; r < _Rows; r++) {
			for (unsigned int c = 0; c < _Cols; c++)
				_Data[r * _Cols + c] *= Right;
		}
		return *this;
	}

	__forceinline Grid<type> operator * (type x)
	{
		Grid<type> result(_Rows, _Cols);
		for (unsigned int r = 0; r < _Rows; r++) {
			for (unsigned int c = 0; c < _Cols; c++)
				result(r,c) = _Data[r * _Cols + c] * x;
		}
		return result;
	}

	//
	// Accessors
	//
	__forceinline type& operator() (unsigned int Row, unsigned int Col)
	{
#ifdef VECTOR_DEBUG
		Assert( (Row < _Rows) && (Col < _Cols), "Out-of-bounds grid access");
#endif
		return _Data[Row * _Cols + Col];
	}
	__forceinline const type& operator() (unsigned int Row, unsigned int Col) const
	{
#ifdef VECTOR_DEBUG
		Assert( (Row < _Rows) && (Col < _Cols), "Out-of-bounds grid access");
#endif
		return _Data[Row * _Cols + Col];
	}
	__forceinline unsigned int Rows() const
	{
		return _Rows;
	}
	__forceinline unsigned int Cols() const
	{
		return _Cols;
	}
	__forceinline Vec2i Dimensions() const
	{
		return Vec2i(_Rows, _Cols);
	}
	__forceinline bool Square() const
	{
		return (_Rows == _Cols);
	}
	__forceinline type* CArray()
	{
		return _Data;
	}
	__forceinline const type* CArray() const
	{
		return _Data;
	}
	Vector<type> MakeRowScanlineVector() const;

	//
	// Query
	//
	__forceinline bool ValidCoordinates(int Row, int Col) const
	{
		return (Row >= 0 && Row < int(_Rows) && Col >= 0 && Col < int(_Cols));
	}

	void ExtractRow(unsigned int Row, Vector<type> &Result) const
	{
		const type *CPtr = _Data;
		Result.Allocate(_Cols);
		for(unsigned int Col = 0; Col < _Cols; Col++)
		{
			Result[Col] = CPtr[Row * _Cols + Col];
		}
	}

	void ExtractCol(unsigned int Col, Vector<type> &Result) const
	{
		const type *CPtr = _Data;
		Result.Allocate(_Rows);
		for(unsigned int Row = 0; Row < _Rows; Row++)
		{
			Result[Row] = CPtr[Row * _Cols + Col];
		}
	}

	Vector<type> ExtractRow(unsigned int Row) const
	{
		Vector<type> Result;
		const type *CPtr = _Data;
		Result.Allocate(_Cols);
		for(unsigned int Col = 0; Col < _Cols; Col++)
		{
			Result[Col] = CPtr[Row * _Cols + Col];
		}
		return Result;
	}

	void SetRow(unsigned int Row, const Vector<type> &Values)
	{
		type *CPtr = _Data;
		for(unsigned int Col = 0; Col < _Cols; Col++)
		{
			CPtr[Row * _Cols + Col] = Values[Col];
		}
	}

	void SetCol(unsigned int Col, const Vector<type> &Values)
	{
		type *CPtr = _Data;
		for(unsigned int Row = 0; Row < _Rows; Row++)
		{
			CPtr[Row * _Cols + Col] = Values[Row];
		}
	}

	Vector<type> ExtractCol(unsigned int Col) const
	{
		Vector<type> Result;
		const type *CPtr = _Data;
		Result.Allocate(_Rows);
		for(unsigned int Row = 0; Row < _Rows; Row++)
		{
			Result[Col] = CPtr[Row * _Cols + Col];
		}
		return Result;
	}

	Vec2i MaxIndex() const;
	const type& MaxValue() const;
	Vec2i MinIndex() const;
	const type& MinValue() const;

	//
	// Modifiers
	//
	void Clear(const type &T);

	//
	// File
	//
	void SaveToASCIIFile(const String &Filename)
	{
		ofstream File(Filename.CString());
		PersistentAssert(!File.fail(), "Failed to open file");
		File << _Rows << '\t' << _Cols << endl;
		for(unsigned int Row = 0; Row < _Rows; Row++)
		{
			for(unsigned int Col = 0; Col < _Cols; Col++)
			{
				File << _Data[Row * _Cols + Col] << '\t';
			}
			File << endl;
		}
	}

protected:
	type *_Data;
	unsigned int _Rows, _Cols;
};

#include "Grid.cpp"
