
struct PixelNeighborhood
{
    Vector<UINT> indices;
    Vector<double> weights;
};

struct SuperpixelNeighborhood
{
    double shortestDist;
    bool visited;
    Vector<UINT> indices;
    Vector<double> embeddingWeights;
    Vector<double> similarityWeights;
};

struct SuperpixelQueueEntry
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
}


struct PixelConstraint
{
    PixelConstraint()
    {

    }

	PixelConstraint(const Vec2i &_coord, const Vec3f &_targetColor, bool _change=true)
    {
        coord = _coord;
        targetColor = _targetColor;
		frame = 0;
		change = _change;
    }

    Vec2i coord;
    Vec3f targetColor;
	int frame;
	bool change;
};

struct SuperpixelConstraint
{
    SuperpixelConstraint()
    {

    }
    SuperpixelConstraint(UINT _index)
    {
        index = _index;
        targetColor = Vec3f::Origin;
        count = 0;
        weight = 1.0;
    }
    SuperpixelConstraint(UINT _index, const Vec3f &_targetColor, double _weight)
    {
        index = _index;
        targetColor = _targetColor;
        weight = _weight;
        count = 1;
    }
    UINT index;
    Vec3f targetColor;
    double weight;
    UINT count;
};

class Recolorizer
{
public:
    void Init(const AppParameters &parameters, const Bitmap &bmp);
    Bitmap Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, double lowQuartile, double highQuartile);
    Bitmap Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors);
    Bitmap Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<SuperpixelConstraint> &targetPixelColors);

private:
    void ComputeSuperpixel(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeSourceDistance(const AppParameters &parameters, const Bitmap &bmp, UINT sourceSuperpixelIndex);
    void ComputeWeightMatrix(const AppParameters &parameters);
    
    Vector<double> ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures);

    void VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const;
    void VisualizeSourceDistance(const AppParameters &parameters, const Bitmap &bmp) const;
    void VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const;
    void TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const;

    Bitmap Recolor(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const;

    Vector<SuperpixelConstraint> MapPixelConstraintsToSuperpixelConstraints(const AppParameters &parameters, const Vector<PixelConstraint> &targetPixelColors) const;

    Vector<ColorCoordinate> superpixelColors;
    Vector<SuperpixelNeighborhood> superpixelNeighbors;
    
    Grid<PixelNeighborhood> pixelNeighborhoods;

    Vector<double> weightMatrixDiagonal;
    Vector<Eigen::Triplet<double> > weightMatrixTriplets;
};
