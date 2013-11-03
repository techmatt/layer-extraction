
struct Video
{
    UINT Width() const  { return frames[0].Width();  }
    UINT Height() const { return frames[0].Height(); }

    Vector<Vec3f> ComputePaletteKMeans(UINT paletteSize) const;

    Vector<Bitmap> frames;
};

struct ColorCoordinateVideo
{
    ColorCoordinateVideo()
    {

    }
    ColorCoordinateVideo(const AppParameters &parameters, RGBColor _color, Vec2i _coord, UINT _frame, int width, int height)
    {
        color = _color;
        coord = _coord;
        frame = _frame;
        MakeFeatureVector(parameters, width, height);
    }
    RGBColor color;
    Vec2i coord;
    UINT frame;
    float features[6];

private:
    void MakeFeatureVector(const AppParameters &parameters, int width, int height)
    {
        features[0] = color.r / 255.0f;
        features[1] = color.g / 255.0f;
        features[2] = color.b / 255.0f;
        features[3] = coord.x/(float)width * parameters.spatialToColorScale;
        features[4] = coord.y/(float)height * parameters.spatialToColorScale;
        features[5] = frame * parameters.temporalToColorScale;
    }
};

class SuperpixelExtractorVideoPeriodic
{
public:
    Vector<ColorCoordinateVideo> Extract(const AppParameters &parameters, const Video &video);
};

/*class SuperpixelExtractorVideoSuperpixel
{
public:
    struct QueueEntry
    {
        double priority;
        Vec2i coord;
        UINT superpixelIndex;
    };

    Vector<ColorCoordinate> Extract(const AppParameters &parameters, const Bitmap &bmp);
    void Extract(const AppParameters &parameters, const Bitmap &bmp, UINT paletteCount, Vector<Superpixel> &superpixelsOut, Grid<UINT> &assignmentsOut);

private:
    void InitializeSuperpixels(const AppParameters &parameters, const Bitmap &bmp);
    void AssignPixel(const AppParameters &parameters, const Bitmap &bmp, const Vec2i &coord, UINT clusterIndex);
    void GrowSuperpixels(const AppParameters &parameters, const Bitmap &bmp);
    void RecenterSuperpixels(const AppParameters &parameters, const Bitmap &bmp);
    
    static void DrawSuperpixelIDs(const Grid<UINT> &superpixelIDs, Bitmap &bmp);
    void DrawSuperpixelColors(const Bitmap &inputBmp, Bitmap &outputBmp);

    Vector<Superpixel> _superpixels;
    priority_queue<QueueEntry> _queue;
    Grid<UINT> _assignments;
    Vec2i _dimensions;
    UINT _paletteCount;
};

__forceinline bool operator < (const SuperpixelExtractorSuperpixel::QueueEntry &a, const SuperpixelExtractorSuperpixel::QueueEntry &b)
{
    return (a.priority < b.priority);
}
*/