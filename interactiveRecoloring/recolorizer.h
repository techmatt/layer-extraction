
/*struct SuperpixelQueueEntry
{
    SuperpixelQueueEntry(SuperpixelNeighborhood &_n, double _shortestDist)
    {
        n = &_n;
        shortestDist = _shortestDist;
    }
    SuperpixelNeighborhood *n;
    double shortestDist;
};

__forceinline bool operator < (const SuperpixelQueueEntry &a, const SuperpixelQueueEntry &b)
{
    return (a.shortestDist > b.shortestDist);
}*/

class Recolorizer
{
public:
    void Init(const Bitmap &bmp);
    //Bitmap Recolor(const Bitmap &bmp, const vector<PixelConstraint> &targetPixelColors, double lowQuartile, double highQuartile);
    //Bitmap Recolor(const Bitmap &bmp, const vector<PixelConstraint> &targetPixelColors);
    //Bitmap Recolor(const Bitmap &bmp, const vector<SuperpixelConstraint> &targetPixelColors);

private:
    void ComputeNearestNeighbors(const Bitmap &bmp);
    void ComputeNeighborWeights(const Bitmap &bmp);
    void ComputeSourceDistance(const Bitmap &bmp, size_t sourceSuperpixelIndex);
    void ComputeWeightMatrix(const AppParameters &parameters);
    
    vector<double> ComputeWeights(const vector<size_t> &indices, const float *pixelFeatures);

    void VisualizeSuperpixels(const Bitmap &bmp, const vector<vec3f> *newSuperpixelColors, const string &filename) const;
    void VisualizeSourceDistance(const Bitmap &bmp) const;
    void VisualizeNearestNeighbors(const Bitmap &bmp) const;
    void TestNeighborWeights(const Bitmap &bmp) const;

    Bitmap Recolor(const Bitmap &bmp, const vector<vec3f> &newSuperpixelColors) const;

    vector<SuperpixelConstraint> MapPixelConstraintsToSuperpixelConstraints(const vector<PixelConstraint> &targetPixelColors) const;

    vector<double> weightMatrixDiagonal;
    vector<Eigen::Triplet<double> > weightMatrixTriplets;
};
