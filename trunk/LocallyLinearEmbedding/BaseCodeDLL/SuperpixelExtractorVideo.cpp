#include "Main.h"

Vector<Vec3f> Video::ComputePaletteKMeans(UINT paletteSize) const
{
    Vector<Vec3f> colors;
    for(UINT sample = 0; sample < 512 * 512; sample++)
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
