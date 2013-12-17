
struct Video
{
    UINT Width() const  { return frames[0].Width();  }
    UINT Height() const { return frames[0].Height(); }

    Vector<Vec3f> ComputePaletteKMeans(UINT paletteSize) const;
    Vector<Vec3f> ComputeFrame0Palette( const Vector<UINT>& frameids, const Vector<String> &filename ) const;

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

struct Superpixel3D
{
	void Reset(const Video &vid, const Vec3i &seed);

	Vec3i MassCentroid() const;
	double AssignmentError(const Video &vid, const Vec3i &coord) const;

	__forceinline void AddCoord(const Vec3i &coord)
	{
		pixels.PushEnd(coord);
	}
	void ResetColor( const RGBColor &_color );
	void ComputeColor( const Video &vid );

	Vec3f color;
	Vector<Vec3i> pixels;
	Vec3i seed;
};

class SuperpixelExtractorVideoSuperpixel
{
public:
    struct QueueEntry
    {
        double priority;
        Vec3i coord;
        UINT superpixelIndex;
    };

    Vector<ColorCoordinateVideo> Extract(const AppParameters &parameters, const Video &vid);
    void Extract(const AppParameters &parameters, const Video &vid, Vector<Superpixel3D> &superpixelsOut, Vector< Grid<UINT> > &assignmentsOut);

private:
    void InitializeSuperpixels(const AppParameters &parameters, const Video &vid);
    void AssignPixel(const AppParameters &parameters, const Video &vid, const Vec3i &coord, UINT clusterIndex);
    void GrowSuperpixels(const AppParameters &parameters, const Video &vid);
    void RecenterSuperpixels(const AppParameters &parameters, const Video &vid);
    
    static void DrawSuperpixelIDs(const Vector< Grid<UINT> > &superpixelIDs, Video &vid, int startframeid, int nframes);
    void DrawSuperpixelColors(const Video &inputVid, Video &outputVid, int startframeid, int nframes);

    Vector<Superpixel3D> _superpixels;
    priority_queue<QueueEntry> _queue;
    Vector< Grid<UINT> > _assignments;
    Vec3i _dimensions;
};

__forceinline bool operator < (const SuperpixelExtractorVideoSuperpixel::QueueEntry &a, const SuperpixelExtractorVideoSuperpixel::QueueEntry &b)
{
    return (a.priority < b.priority);
}
