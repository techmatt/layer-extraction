#include "Main.h"

vector<SuperpixelCoord> SuperpixelGeneratorPeriodic::extract(const Bitmap &bmp)
{
    vector<SuperpixelCoord> result;
    for(int y = 0; y < (int)bmp.getDimY(); y += appParams().periodicBasisCount)
    {
        for(int x = 0; x < (int)bmp.getDimX(); x += appParams().periodicBasisCount)
        {
            SuperpixelCoord coord(bmp(x, y), vec2i(x, y), bmp.getDimX(), bmp.getDimY());
            result.push_back(coord);
        }
    }
    return result;
}

void SuperpixelCluster::reset(const Bitmap &bmp, const vec2i &_seed)
{
    pixels.clear();
    seed = _seed;
    addCoord(seed);
}

vec2i SuperpixelCluster::massCentroid() const
{
    vec2f center = vec2f::origin;
    for(const vec2i &p : pixels) center += vec2f(p);
    center /= float(pixels.size());
    
    vec2i bestCoord = pixels[0];
    float bestDistSq = 100000000.0f;
    for(const vec2i &p : pixels)
    {
        float curDistSq = vec2f::distSq(vec2f(p), center);
        if(curDistSq < bestDistSq)
        {
            bestDistSq = curDistSq;
            bestCoord = p;
        }
    }
    return bestCoord;
}

double SuperpixelCluster::assignmentError(const Bitmap &bmp, const vec2i &coord) const
{
    return vec3f::distSq(color, colorUtil::toVec3f(bmp(coord.x, coord.y)));
}

void SuperpixelCluster::resetColor( const RGBColor &_color )
{
    color = vec3f(_color);
}

void SuperpixelCluster::computeColor( const Bitmap &bmp )
{
    color = vec3f::origin;
    for(vec2i p : pixels) color += colorUtil::toVec3f(bmp(p.x, p.y));
    color /= (float)pixels.size();
}

vector<SuperpixelCoord> SuperpixelGeneratorSuperpixel::extract(const Bitmap &bmp)
{
	Grid2<int> assignmentsOut;
	return extract(bmp, assignmentsOut);
}

vector<SuperpixelCoord> SuperpixelGeneratorSuperpixel::extract(const Bitmap &bmp, Grid2<int> &assignmentsOut)
{
    vector<SuperpixelCluster> superpixelsOut;
    extract(bmp, superpixelsOut, assignmentsOut);

    vector<SuperpixelCoord> result;
    for(const SuperpixelCluster &p : superpixelsOut)
    {
        result.push_back(SuperpixelCoord(RGBColor(p.color), p.seed, bmp.getDimX(), bmp.getDimY()));
    }
    return result;
}

void SuperpixelGeneratorSuperpixel::extract(const Bitmap &bmp, vector<SuperpixelCluster> &superpixelsOut, Grid2<int> &assignmentsOut)
{
    ComponentTimer timer( "Creating superpixels, " + bmp.getDimensions().toString(','));
    
    _dimensions = vec2i(bmp.getDimX(), bmp.getDimY());
    _assignments.allocate(_dimensions, -1);
    _superpixels.resize(appParams().superpixelCount);

    initializeSuperpixels(bmp);

    const size_t iterationCount = appParams().superpixelIterations;
    for(size_t iterationIndex = 0; iterationIndex < iterationCount; iterationIndex++)
    {
        cout << "Starting superpixel iteration " << iterationIndex << endl;
        //ComponentTimer timer( "Iteration " + string(iterationIndex) );
        growSuperpixels(bmp);

        const bool dumpIntermediateResults = true;
        if(dumpIntermediateResults)
        {
			LodePNG::save(drawSuperpixelIDs(), appParams().debugDir + "i" + to_string(iterationIndex) + "-IDs.png");
			LodePNG::save(drawSuperpixelColors(), appParams().debugDir + "i" + to_string(iterationIndex) + "-colors.png");
        }

        recenterSuperpixels(bmp);
    }

    growSuperpixels(bmp);
    for(SuperpixelCluster &p : _superpixels)
    {
        p.computeColor(bmp);
    }

    superpixelsOut = _superpixels;
    assignmentsOut = _assignments;
}

void SuperpixelGeneratorSuperpixel::initializeSuperpixels(const Bitmap &bmp)
{
    vector<vec2i> seeds;
    for(SuperpixelCluster &p : _superpixels)
    {
        vec2i randomSeed(rand() % _dimensions.x, rand() % _dimensions.y);
        while(util::contains(seeds, randomSeed))
        {
            randomSeed = vec2i(rand() % _dimensions.x, rand() % _dimensions.y);
        }
        
        p.resetColor(bmp(randomSeed.x, randomSeed.y));
        p.reset(bmp, randomSeed);
        seeds.push_back(randomSeed);
    }
}

void SuperpixelGeneratorSuperpixel::assignPixel(const Bitmap &bmp, const vec2i &coord, int superpixelIndex)
{
    if(_assignments(coord.x, coord.y) != -1)
    {
        return;
    }
    _assignments(coord.x, coord.y) = superpixelIndex;
    _superpixels[superpixelIndex].addCoord(coord);

    const size_t neighborCount = 4;
    const int XOffsets[neighborCount] = {-1, 1, 0, 0};
    const int YOffsets[neighborCount] = {0, 0, -1, 1};
    for(size_t neighborIndex = 0; neighborIndex < neighborCount; neighborIndex++)
    {
        vec2i finalCoord(coord.x + XOffsets[neighborIndex], coord.y + YOffsets[neighborIndex]);
        if(_assignments.isValidCoordinate(finalCoord.x, finalCoord.y) && _assignments(finalCoord) == -1)
        {
            QueueEntry newEntry;
            newEntry.superpixelIndex = superpixelIndex;
            newEntry.coord = finalCoord;
            newEntry.priority = 1.0 - _superpixels[superpixelIndex].assignmentError(bmp, finalCoord);
            _queue.push(newEntry);
        }
    }
}

void SuperpixelGeneratorSuperpixel::growSuperpixels(const Bitmap &bmp)
{
    _assignments.setValues(-1);

    //
    // Insert all seeds
    //
    for(size_t superpixelIndex = 0; superpixelIndex < _superpixels.size(); superpixelIndex++)
    {
        assignPixel(bmp, _superpixels[superpixelIndex].seed, (int)superpixelIndex);
    }

    while(!_queue.empty())
    {
        QueueEntry curEntry = _queue.top();
        _queue.pop();
        assignPixel(bmp, curEntry.coord, curEntry.superpixelIndex);
    }
}

void SuperpixelGeneratorSuperpixel::recenterSuperpixels(const Bitmap &bmp)
{
    const size_t clusterSizeCutoff = 10;

    size_t teleportCount = 0;
    vector<vec2i> seeds;
    for(SuperpixelCluster &p : _superpixels)
    {
        vec2i newSeed;
        if(p.pixels.size() < clusterSizeCutoff)
        {
            newSeed = vec2i(rand() % _dimensions.x, rand() % _dimensions.y);
            while(util::contains(seeds, newSeed))
            {
                newSeed = vec2i(rand() % _dimensions.x, rand() % _dimensions.y);
            }
            teleportCount++;
            p.resetColor(bmp(newSeed.x, newSeed.y));
        }
        else
        {
            newSeed = p.massCentroid();
            p.computeColor(bmp);
        }
        p.reset(bmp, newSeed);
        seeds.push_back(newSeed);
    }
}

Bitmap SuperpixelGeneratorSuperpixel::drawSuperpixelIDs()
{
    const size_t superpixelCount = _assignments.getMaxValue() + 1;
    vector<vec4uc> colors(superpixelCount);
    for(vec4uc &c : colors) c = colorUtil::randomColorVec4uc();

	Bitmap result(_assignments.getDimensions());
	for (auto &p : result)
	{
		p.value = colors[_assignments(p.x, p.y)];
	}
	return result;
}

Bitmap SuperpixelGeneratorSuperpixel::drawSuperpixelColors()
{
	Bitmap result(_assignments.getDimensions());
    for(auto &p : result)
    {
        const SuperpixelCluster &s =  _superpixels[_assignments(p.x, p.y)];
        p.value = colorUtil::toVec4uc(s.color);
    }
	return result;
}
