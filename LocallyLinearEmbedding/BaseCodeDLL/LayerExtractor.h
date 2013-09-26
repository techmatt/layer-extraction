
struct Layer
{
    Vec3f color;
    Vector<double> superpixelWeights;
};

struct PixelLayer
{
	Vec3f color;
	Grid<double> pixelWeights;

	double WeightMean()
	{
		double mean = 0;
		for (UINT r=0; r<pixelWeights.Rows(); r++)
			for (UINT c=0; c<pixelWeights.Cols(); c++)
				mean += pixelWeights(r,c);
		mean /= (pixelWeights.Rows()*pixelWeights.Cols());
		return mean;
	}

	double WeightVariance()
	{
		double variance = 0;
		double mean = WeightMean();
		for (UINT r=0; r<pixelWeights.Rows(); r++)
				for (UINT c=0; c<pixelWeights.Cols(); c++)
					variance += Math::Square(pixelWeights(r,c)-mean);

		variance/= (pixelWeights.Rows()*pixelWeights.Cols());
		return variance;
	}

	void WriteToFile(const String &filename)
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

	void SavePNG(const String &filename, bool renderColor=true)
	{
		Bitmap image(pixelWeights.Cols(), pixelWeights.Rows());
		for (UINT x=0; x<image.Width(); x++)
			for (UINT y=0; y<image.Height(); y++)
				image[y][x] = RGBColor(Utility::BoundToByte(pixelWeights(y,x)*255),Utility::BoundToByte(pixelWeights(y,x)*255), Utility::BoundToByte(pixelWeights(y,x)*255));

		//draw a color strip
		if (renderColor)
		{
			int strip = 0.10*image.Width();
			for (UINT x=0; x<strip; x++)
				for (UINT y=0; y<image.Height(); y++)
					image[y][x] = RGBColor(color);
		}

		image.SavePNG(filename);
	}

};

typedef Vector<PixelLayer> PixelLayerSet;

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
    Vector<SuperpixelLayerConstraint> constraints;
};

class LayerExtractor
{
public:
    void Init(const AppParameters &parameters, const Bitmap &bmp);
    void InitLayersFromPixelConstraints(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, LayerSet &result);
    void InitLayersFromPalette(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> &palette, LayerSet &result);
	void AddLayerPreferenceConstraints(const AppParameters &parameters, const Bitmap &bmp,LayerSet &result);
    void AddNegativeConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result);
    void ExtractLayers(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers);

    void TestLayerRecoloring(const Bitmap &bmp, const LayerSet &layers) const;
    Bitmap RecolorSuperpixels(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const;
    Bitmap RecolorLayers(const Bitmap &bmp, const LayerSet &layers, const Vector<RGBColor> &newLayerColors) const;

	Vector<PixelLayer> GetPixelLayers(const Bitmap &bmp, const LayerSet &layers) const;
    
    const Vector<ColorCoordinate>& SuperpixelColors() const
    {
        return superpixelColors;
    }
private:
    void ComputeSuperpixels(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeSourceDistance(const AppParameters &parameters, const Bitmap &bmp, UINT sourceSuperpixelIndex);
    void ComputeWeightMatrix(const AppParameters &parameters);
    
    Vector<double> ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures);

    void VisualizeReconstruction(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeLayers(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeLayerConstraints(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const;
    void VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const;
    void TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const;

    Vector<ColorCoordinate> superpixelColors;
    Vector<SuperpixelNeighborhood> superpixelNeighbors;
    
    Grid<PixelNeighborhood> pixelNeighborhoods;

    SparseMatrix<double> WBase;
    int pass;
};
