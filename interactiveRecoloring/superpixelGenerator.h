
struct SuperpixelCoord
{
    SuperpixelCoord()
    {

    }
    SuperpixelCoord(const vec4uc &_color, const vec2i &_coord, int width, int height)
    {
        color = colorUtil::toVec3f(_color);
        coord = _coord;
        makeFeatureVector(width, height);
    }
	SuperpixelCoord(const Bitmap &bmp, size_t x, size_t y)
	{
		color = colorUtil::toVec3f(bmp(x, y));
		coord = vec2i((int)x, (int)y);
		makeFeatureVector(bmp.getDimX(), bmp.getDimY());
	}
    vec3f color;
    vec2i coord;
    float features[5];

private:
    void makeFeatureVector(int width, int height)
    {
        features[0] = color.r;
        features[1] = color.g;
        features[2] = color.b;
        features[3] = coord.x / (float)width * appParams().spatialToColorScale;
        features[4] = coord.y / (float)height * appParams().spatialToColorScale;
    }
};

class SuperpixelGeneratorPeriodic
{
public:
	vector<SuperpixelCoord> extract(const Bitmap &bmp);
};

struct SuperpixelCluster
{
    void reset(const Bitmap &bmp, const vec2i &seed);

    vec2i massCentroid() const;
    double assignmentError(const Bitmap &bmp, const vec2i &coord) const;
    
    void addCoord(const vec2i &coord)
    {
        pixels.push_back(coord);
    }
    void resetColor( const RGBColor &_color );
    void computeColor( const Bitmap &bmp );

    vec3f color;
    vector<vec2i> pixels;
    vec2i seed;
};

class SuperpixelGeneratorSuperpixel
{
public:
    struct QueueEntry
    {
        double priority;
        vec2i coord;
		int superpixelIndex;
    };

    vector<SuperpixelCoord> extract(const Bitmap &bmp);
	vector<SuperpixelCoord> extract(const Bitmap &bmp, Grid2<int> &assignmentsOut);
    void extract(const Bitmap &bmp, vector<SuperpixelCluster> &superpixelsOut, Grid2<int> &assignmentsOut);

private:
    void initializeSuperpixels(const Bitmap &bmp);
    void assignPixel(const Bitmap &bmp, const vec2i &coord, int clusterIndex);
    void growSuperpixels(const Bitmap &bmp);
    void recenterSuperpixels(const Bitmap &bmp);
    
    Bitmap drawSuperpixelIDs();
    Bitmap drawSuperpixelColors();

    vector<SuperpixelCluster> _superpixels;
    priority_queue<QueueEntry> _queue;
    Grid2<int> _assignments;
    vec2i _dimensions;
};

__forceinline bool operator < (const SuperpixelGeneratorSuperpixel::QueueEntry &a, const SuperpixelGeneratorSuperpixel::QueueEntry &b)
{
    return (a.priority < b.priority);
}
