struct Layer
{
    Vec3f color;
    Vector<double> superpixelWeights;

	double Overlap(Layer other) const
	{
		//compute dot product
		PersistentAssert(superpixelWeights.Length() == other.superpixelWeights.Length(), "Superpixel count is not the same");
		
		double overlap = 0;

		for (UINT i=0; i<superpixelWeights.Length(); i++)
			overlap += superpixelWeights[i] * other.superpixelWeights[i];

		return overlap;
	};

	double AveragePositive() const
	{
		double pos = 0;
		for (UINT i=0; i<superpixelWeights.Length(); i++)
		{
			if (superpixelWeights[i] > 0)
				pos += superpixelWeights[i];
		}
		if (superpixelWeights.Length() > 0)
			return pos/superpixelWeights.Length();
		return 0;
	}

	double MaxWeight() const
	{
		return superpixelWeights.MaxValue();
	}

	double MinWeight() const
	{
		return superpixelWeights.MinValue();
	}

	double WeightMean() const
	{
		if (superpixelWeights.Length() == 0)
			return 0;
		return superpixelWeights.Sum()/superpixelWeights.Length();
	};

	double PercentNegative() const
	{
		if (superpixelWeights.Length() == 0)
			return 0;

		double negCount = 0;
		for (UINT i=0; i<superpixelWeights.Length(); i++)
			if (superpixelWeights[i] < 0)
				negCount++;
		return negCount/superpixelWeights.Length();
	};

	double AverageNegative() const
	{
		double neg = 0;
		for (UINT i=0; i<superpixelWeights.Length(); i++)
		{
			if (superpixelWeights[i] < 0)
				neg += superpixelWeights[i];
		}
		if (superpixelWeights.Length() > 0)
			return neg/superpixelWeights.Length();
		return 0;
	}

	
};

struct SuperpixelLayerConstraint
{
    SuperpixelLayerConstraint()
    {

    }
    SuperpixelLayerConstraint(UINT _index, UINT _layerIndex, double _target, double _weight)
    {
        index = _index;
        layerIndex = _layerIndex;
        target = _target;
        weight = _weight;
    }

    UINT index;
    UINT layerIndex;
    double target;
    double weight;
};


struct LayerSet
{
    void Dump(const String &filename, const Vector<ColorCoordinate> &superpixelColors) const;
    Vector<Layer> layers;
    Dictionary<String, Vector<SuperpixelLayerConstraint>> constraints;
};

struct PixelLayer
{
	Vec3f color;
	Grid<double> pixelWeights;

	int Width() const
	{
		return pixelWeights.Cols();
	}

	int Height() const
	{
		return pixelWeights.Rows();
	}

	double WeightMean() const
	{
		double mean = 0;
		for (UINT r=0; r<pixelWeights.Rows(); r++)
			for (UINT c=0; c<pixelWeights.Cols(); c++)
				mean += pixelWeights(r,c);
		mean /= (pixelWeights.Rows()*pixelWeights.Cols());
		return mean;
	}

	double WeightVariance() const
	{
		double variance = 0;
		double mean = WeightMean();
		for (UINT r=0; r<pixelWeights.Rows(); r++)
				for (UINT c=0; c<pixelWeights.Cols(); c++)
					variance += Math::Square(pixelWeights(r,c)-mean);

		variance/= (pixelWeights.Rows()*pixelWeights.Cols());
		return variance;
	}

	double DotProduct(const PixelLayer& other)
	{
		PersistentAssert(pixelWeights.Rows() == other.pixelWeights.Rows() && pixelWeights.Cols() == other.pixelWeights.Cols(), "PixelLayer::DotProduct: Dimensions don't match!");
		double result = 0;
		for (int r=0; r<pixelWeights.Rows(); r++)
			for (int c=0; c<pixelWeights.Cols(); c++)
				result += pixelWeights(r,c)*other.pixelWeights(r,c);
		return result;
	}

	void WriteToFile(const String &filename) const
	{
		ofstream File(filename.CString());
        PersistentAssert(!File.fail(), "Failed to open file");
        File << pixelWeights.Rows() << '\t' << pixelWeights.Cols() << endl;
		File << color.TabSeparatedString() << endl;
        for(unsigned int Row = 0; Row < pixelWeights.Rows(); Row++)
        {
            for(unsigned int Col = 0; Col < pixelWeights.Cols(); Col++)
            {
                File << pixelWeights(Row, Col) << '\t';
            }
            File << endl;
        }		
	}

	PixelLayer(){}

	PixelLayer(const String &filename)
	{
		Vector<String> lines = Utility::GetFileLines(filename);
		//get the row and cols
		Vector<String> fields = lines.First().Partition('\t');
		pixelWeights.Allocate(fields[0].ConvertToUnsignedInteger(), fields[1].ConvertToUnsignedInteger()); 

		Vector<String> colorFields = lines[1].Partition('\t');
		color = Vec3f(colorFields[0].ConvertToFloat(), colorFields[1].ConvertToFloat(), colorFields[2].ConvertToFloat());

		for (unsigned int Row = 0; Row < pixelWeights.Rows(); Row++)
		{
			Vector<String> curRow = lines[Row+2].Partition('\t');

			for (unsigned int Col = 0; Col < pixelWeights.Cols(); Col++)
			{
				pixelWeights(Row, Col) = curRow[Col].ConvertToDouble();
			}
		}
	}

	PixelLayer(const PixelLayer& p)
	{
		color = Vec3f(p.color);
		pixelWeights.Allocate(p.pixelWeights.Rows(), p.pixelWeights.Cols());
		for (unsigned int Row = 0; Row < pixelWeights.Rows(); Row++) {
			for (unsigned int Col = 0; Col < pixelWeights.Cols(); Col++) {
				pixelWeights(Row, Col) = p.pixelWeights(Row, Col);
			}
		}
	}

	PixelLayer(Vec3f col, int width, int height)
	{
		color = col;
		pixelWeights.Allocate(height, width, 0);
	}

	void SavePNG(const String &filename, bool renderColor=true) const
	{
		Bitmap image(pixelWeights.Cols(), pixelWeights.Rows());
		for (UINT x=0; x<image.Width(); x++)
			for (UINT y=0; y<image.Height(); y++)
				image[y][x] = RGBColor(Utility::BoundToByte(pixelWeights(y,x)*255),Utility::BoundToByte(pixelWeights(y,x)*255), Utility::BoundToByte(pixelWeights(y,x)*255));

		//draw a color strip
		if (renderColor)
		{
			UINT strip = (int)(0.10*image.Width());
			for (UINT x=0; x<strip; x++)
				for (UINT y=0; y<image.Height(); y++)
					image[y][x] = RGBColor(color);
		}

		image.SavePNG(filename);
	}


};

typedef Vector<PixelLayer> PixelLayerSet;

