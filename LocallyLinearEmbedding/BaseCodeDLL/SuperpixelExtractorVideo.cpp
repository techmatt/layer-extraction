#include "Main.h"

Vector<ColorCoordinateVideo> SuperpixelExtractorVideoPeriodic::Extract(const AppParameters &parameters, const Video &video)
{
    Vector<ColorCoordinateVideo> result;
    for(UINT frame = 0; frame < parameters.periodicBasisCount; frame++)
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
    return result;
}
