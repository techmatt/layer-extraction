#pragma once
class RecolorizerPixel
{
public:
	RecolorizerPixel(void);
	~RecolorizerPixel(void);

    void Init(const AppParameters &parameters, const Bitmap &bmp);
    Bitmap Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors);

private:
    void ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeWeightMatrix(const AppParameters &parameters, const Bitmap &bmp);
    
    Vector<double> ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures);

    void VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const;
    void VisualizeSourceDistance(const AppParameters &parameters, const Bitmap &bmp) const;
    void VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const;
    void TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const;

    
    Grid<PixelNeighborhood> pixelNeighborhoods;
	Vector<ColorCoordinate> pixelColors;

	SparseMatrix<double> WBase;

};

