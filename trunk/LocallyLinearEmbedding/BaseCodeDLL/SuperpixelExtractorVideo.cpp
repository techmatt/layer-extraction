#include "Main.h"

Vector<Vec3f> Video::ComputePaletteKMeans(UINT paletteSize) const
{
	Vector<Vec3f> colors;
	for(UINT sample = 0; sample < 1024 * 1024; sample++)
	{
		colors.PushEnd(Vec3f(frames.RandomElement()[rand() % Height()][rand() % Width()]));
	}

	KMeansClustering<Vec3f, Vec3fKMeansMetric> cluster;
	cluster.Cluster(colors, paletteSize);

	Vector<Vec3f> result;
	for(UINT clusterIndex = 0; clusterIndex < paletteSize; clusterIndex++)
	{
		result.PushEnd(cluster.ClusterCenter(clusterIndex));
	}
	return result;
}

Vector<ColorCoordinateVideo> SuperpixelExtractorVideoPeriodic::Extract(const AppParameters &parameters, const Video &video)
{
	Vector<ColorCoordinateVideo> result;
	for(int frame = 0; frame < parameters.periodicBasisCount; frame++)
	{
		for(UINT y = 0; y < video.Height(); y += parameters.periodicBasisCount)
		{
			for(UINT x = 0; x < video.Width(); x += parameters.periodicBasisCount)
			{
				ColorCoordinateVideo coord(parameters, video.frames[frame][y][x], Vec2i(x, y), frame, video.Width(), video.Height());
				result.PushEnd(coord);
			}
		}
	}

	if((int)result.Length() > parameters.superpixelCount)
	{
		result.Randomize();
		result.ReSize(parameters.superpixelCount);
	}

	return result;
}

void Superpixel3D::Reset(const Video &vid, const Vec3i &_seed)
{
	pixels.FreeMemory();
	seed = _seed;
	AddCoord(seed);
}

Vec3i Superpixel3D::MassCentroid() const
{
	Vec3f center = Vec3f::Origin;
	for(const Vec3i &p : pixels) center += Vec3f(p);
	center /= float(pixels.Length());

	Vec3i bestCoord = pixels[0];
	float bestDistSq = 100000000.0f;
	for(const Vec3i &p : pixels)
	{
		float curDistSq = Vec3f::DistSq(Vec3f(p), center);
		if(curDistSq < bestDistSq)
		{
			bestDistSq = curDistSq;
			bestCoord = p;
		}
	}
	return bestCoord;
}

double Superpixel3D::AssignmentError(const Video &vid, const Vec3i &coord) const
{
	return Vec3f::DistSq(color, Vec3f(vid.frames[coord.z][coord.y][coord.x]));
}

void Superpixel3D::ResetColor( const RGBColor &_color )
{
	color = Vec3f(_color);
}

void Superpixel3D::ComputeColor( const Video &vid )
{
	color = Vec3f::Origin;
	for(Vec3i p : pixels) color += Vec3f(vid.frames[p.z][p.y][p.x]);
	color /= (float)pixels.Length();
}

Vector<ColorCoordinateVideo> SuperpixelExtractorVideoSuperpixel::Extract(const AppParameters &parameters, const Video &vid)
{
	Vector<Superpixel3D> superpixelsOut;
	Vector< Grid<UINT> > assignmentsOut;
	Extract(parameters, vid, superpixelsOut, assignmentsOut);

	Vector<ColorCoordinateVideo> result;
	for(const Superpixel3D &p : superpixelsOut)
	{
		result.PushEnd(ColorCoordinateVideo(parameters, RGBColor(p.color), Vec2i(p.seed.x, p.seed.y), p.seed.z, vid.Width(), vid.Height()));
	}
	return result;
}

void SuperpixelExtractorVideoSuperpixel::Extract(const AppParameters &parameters, const Video &vid, Vector<Superpixel3D> &superpixelsOut, Vector< Grid<UINT> > &assignmentsOut)
{
	ComponentTimer timer( "Segmenting video: " + String(vid.Width()) + "x" + String(vid.Height()) + ", " + String(vid.frames.Length()) + " frames" );

	_dimensions = Vec3i(vid.Width(), vid.Height(), vid.frames.Length());
	_assignments.Allocate(vid.frames.Length());
	for (UINT i = 0; i < vid.frames.Length(); i++)
		_assignments[i].Allocate(_dimensions.y, _dimensions.x);
	_superpixels.Allocate(parameters.superpixelCount);

	InitializeSuperpixels(parameters, vid);

	const UINT iterationCount = parameters.superpixelIterations;
	for(UINT iterationIndex = 0; iterationIndex < iterationCount; iterationIndex++)
	{
		Console::WriteLine("Starting superpixel iteration " + String(iterationIndex));
		//ComponentTimer timer( "Iteration " + String(iterationIndex) );
		GrowSuperpixels(parameters, vid);

		const bool dumpIntermediateResults = true;
		if(dumpIntermediateResults)
		{
			Video clusterVid0, clusterVid1;
			DrawSuperpixelIDs(_assignments, clusterVid0, 0, 5);
			DrawSuperpixelColors(vid, clusterVid1, 0, 5);
			// save first couple frames
			for (int f = 0; f < 5; f++) {
				clusterVid0.frames[f].SavePNG("clustersIteration" + String(iterationIndex) + "_f" + String(f) + ".png");
				clusterVid1.frames[f].SavePNG("colorsIteration" + String(iterationIndex) + "_f" + String(f) + ".png");
			}
		}

		RecenterSuperpixels(parameters, vid);
	}

	GrowSuperpixels(parameters, vid);
	for(Superpixel3D &p : _superpixels)
	{
		p.ComputeColor(vid);
	}

	superpixelsOut = _superpixels;
	assignmentsOut = _assignments;

}

void SuperpixelExtractorVideoSuperpixel::InitializeSuperpixels(const AppParameters &parameters, const Video &vid)
{
	Vector<Vec3i> seeds;
	for(Superpixel3D &p : _superpixels)
	{
		Vec3i randomSeed(rand() % _dimensions.x, rand() % _dimensions.y, rand() % _dimensions.z);
		while(seeds.Contains(randomSeed))
		{
			randomSeed = Vec3i(rand() % _dimensions.x, rand() % _dimensions.y, rand() % _dimensions.z);
		}

		p.ResetColor(vid.frames[randomSeed.z][randomSeed.y][randomSeed.x]);
		p.Reset(vid, randomSeed);
		seeds.PushEnd(randomSeed);
	}
}

void SuperpixelExtractorVideoSuperpixel::AssignPixel(const AppParameters &parameters, const Video &vid, const Vec3i &coord, UINT superpixelIndex)
{
	if(_assignments[coord.z](coord.y, coord.x) != 0xFFFFFFFF)
	{
		return;
	}
	_assignments[coord.z](coord.y, coord.x) = superpixelIndex;
	_superpixels[superpixelIndex].AddCoord(coord);

	const UINT neighborCount = 6;
	const UINT XOffsets[neighborCount] = {-1, 1, 0, 0,  0, 0};
	const UINT YOffsets[neighborCount] = {0, 0, -1, 1,  0, 0};
	const UINT FOffsets[neighborCount] = {0, 0,  0, 0, -1, 1};
	for(UINT neighborIndex = 0; neighborIndex < neighborCount; neighborIndex++)
	{
		Vec3i finalCoord(coord.x + XOffsets[neighborIndex], coord.y + YOffsets[neighborIndex], coord.z + FOffsets[neighborIndex]);
		if(finalCoord.z >= 0 && finalCoord.z < _assignments.Length() && _assignments[finalCoord.z].ValidCoordinates(finalCoord.y, finalCoord.x) && 
			_assignments[finalCoord.z](finalCoord.y, finalCoord.x) == 0xFFFFFFFF) // bounds check
		{
			QueueEntry newEntry;
			newEntry.superpixelIndex = superpixelIndex;
			newEntry.coord = finalCoord;
			newEntry.priority = 1.0 - _superpixels[superpixelIndex].AssignmentError(vid, finalCoord);
			_queue.push(newEntry);
		}
	}
}

void SuperpixelExtractorVideoSuperpixel::GrowSuperpixels(const AppParameters &parameters, const Video &vid)
{
	for (UINT i = 0; i < _assignments.Length(); i++)
		_assignments[i].Clear(0xFFFFFFFF);

	//
	// Insert all seeds
	//
	for(UINT superpixelIndex = 0; superpixelIndex < _superpixels.Length(); superpixelIndex++)
	{
		AssignPixel(parameters, vid, _superpixels[superpixelIndex].seed, superpixelIndex);
	}

	while(!_queue.empty())
	{
		QueueEntry curEntry = _queue.top();
		_queue.pop();
		AssignPixel(parameters, vid, curEntry.coord, curEntry.superpixelIndex);
	}
}

void SuperpixelExtractorVideoSuperpixel::RecenterSuperpixels(const AppParameters &parameters, const Video &vid)
{
	const UINT clusterSizeCutoff = 20;

	UINT teleportCount = 0;
	Vector<Vec3i> seeds;
	for(Superpixel3D &p : _superpixels)
	{
		Vec3i newSeed;
		if(p.pixels.Length() < clusterSizeCutoff)
		{
			newSeed = Vec3i(rand() % _dimensions.x, rand() % _dimensions.y, rand() % _dimensions.z);
			while(seeds.Contains(newSeed))
			{
				newSeed = Vec3i(rand() % _dimensions.x, rand() % _dimensions.y, rand() % _dimensions.z);
			}
			teleportCount++;
			p.ResetColor(vid.frames[newSeed.z][newSeed.y][newSeed.x]);
		}
		else
		{
			newSeed = p.MassCentroid();
			p.ComputeColor(vid);
		}
		p.Reset(vid, newSeed);
		seeds.PushEnd(newSeed);
	}
}

void SuperpixelExtractorVideoSuperpixel::DrawSuperpixelIDs(const Vector< Grid<UINT> > &superpixelIDs, Video &vid, int startframeid, int nframes)
{
	UINT superpixelmaxvalue = 0;
	for (UINT i = 0; i < superpixelIDs.Length(); i++) {
		UINT mx = superpixelIDs[i].MaxValue();
		if (mx > superpixelmaxvalue) superpixelmaxvalue = mx;
	}
	const UINT clusterCount = superpixelmaxvalue + 1;
	Vector<RGBColor> colors(clusterCount);
	for(RGBColor &c : colors) c = RGBColor::RandomColor();
	//ColorGenerator::Generate(colors);
	vid.frames.Allocate(nframes);
	const UINT height = superpixelIDs[0].Rows();
	const UINT width = superpixelIDs[0].Cols();
	for (int f = startframeid; f < startframeid+nframes; f++) {
		vid.frames[f-startframeid].Allocate(width, height);

		for(UINT y = 0; y < height; y++)
		{
			for(UINT x = 0; x < width; x++)
			{
				vid.frames[f-startframeid][y][x] = colors[superpixelIDs[f](y, x)];
			}
		}
	}
}

void SuperpixelExtractorVideoSuperpixel::DrawSuperpixelColors(const Video &inputVid, Video &outputVid, int startframeid, int nframes)
{
	const UINT height = _assignments[0].Rows();
	const UINT width = _assignments[0].Cols();
	outputVid.frames.Allocate(nframes);
	for (int f = 0; f < nframes; f++) {
		outputVid.frames[f].Allocate(width, height);
		for(UINT y = 0; y < height; y++)
		{
			for(UINT x = 0; x < width; x++)
			{
				const Superpixel3D &p =  _superpixels[_assignments[f](y, x)];
				outputVid.frames[f][y][x] = RGBColor(p.color);
			}
		}
	}
}