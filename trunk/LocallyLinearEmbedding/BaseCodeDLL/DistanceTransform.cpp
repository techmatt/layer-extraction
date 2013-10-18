#include "Main.h"

Grid<double> DistanceTransform::Transform(const Grid<double> &g)
{
    Vector< Grid<double> > grids;

    grids.PushEnd(Transform(g, 0.1, 3.0, false));
    grids.PushEnd(Transform(g, 0.25, 3.0, false));
    grids.PushEnd(Transform(g, 0.5, 3.0, false));
    grids.PushEnd(Transform(g, 0.6, 3.0, false));
    grids.PushEnd(Transform(g, 0.7, 3.0, false));
    grids.PushEnd(Transform(g, 0.8, 3.0, false));
    grids.PushEnd(Transform(g, 0.9, 3.0, false));
    grids.PushEnd(Transform(g, 0.95, 3.0, false));

    grids.PushEnd(Transform(g, 0.3, 1.0, true));
    grids.PushEnd(Transform(g, 0.4, 1.0, true));
    grids.PushEnd(Transform(g, 0.5, 1.0, true));
    grids.PushEnd(Transform(g, 0.6, 1.0, true));
    
    Grid<double> r(g.Rows(), g.Cols(), 0.0);
    for(UINT row = 0; row < g.Rows(); row++) for(UINT col = 0; col < g.Cols(); col++)
        for(auto &i : grids)
        {
            r(row, col) += i(row, col) / grids.Length();
        }
    return r;
}

Grid<double> DistanceTransform::Transform(const Grid<double> &g, double quartile, double power, bool flip)
{
    double threshold = GetThreshold(g, quartile, flip);

    Grid<bool> visited(g.Rows(), g.Cols(), false);
    Grid<double> dist(g.Rows(), g.Cols(), 1000000.0);
    
    auto comp = []( const pair<Vec2i, double> &a, const pair<Vec2i, double> &b ) { return a.second > b.second; };

    priority_queue< pair<Vec2i, double>, vector< pair<Vec2i, double> >, decltype( comp ) > queue(comp);

    for(UINT row = 0; row < g.Rows(); row++) for(UINT col = 0; col < g.Cols(); col++)
    {
        double v = g(row, col);
        if( (!flip && v >= threshold) ||
            ( flip && v <= threshold) )
        {
            dist(row, col) = 0.0;
            queue.push(make_pair(Vec2i(row, col), 0.0));
        }
    }

    while(!queue.empty())
    {
        auto e = queue.top();
        queue.pop();

        if(!visited(e.first.x, e.first.y))
        {
            visited(e.first.x, e.first.y) = true;
            Vector<Vec2i> neighbors = GetNeighbors(g, e.first);
            for(Vec2i n : neighbors)
            {
                double newDist = dist(e.first.x, e.first.y) + Vec2i::Dist(e.first, n);
                if(newDist < dist(n.x, n.y))
                {
                    dist(n.x, n.y) = newDist;
                    queue.push(make_pair(n, newDist));
                }
            }
        }
    }

    double max = dist.MaxValue();

    for(UINT row = 0; row < g.Rows(); row++) for(UINT col = 0; col < g.Cols(); col++)
    {
        double x = dist(row, col) / max;
        if(flip) dist(row, col) = pow(x, power);
        else dist(row, col) = pow(1.0 - x, power);
        //dist(row, col) = 1.0 - dist(row, col) / max;
        
    }

    return dist;
}

Vector<Vec2i> DistanceTransform::GetNeighbors(const Grid<double> &g, const Vec2i &p)
{
    Vector<Vec2i> result;
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            if(g.ValidCoordinates(p.x + x, p.y + y))
            {
                result.PushEnd(p + Vec2i(x, y));
            }
        }
    }
    return result;
}

double DistanceTransform::GetThreshold(const Grid<double> &g, double quartile, bool flip)
{
    Vector<double> values;
    for(UINT row = 0; row < g.Rows(); row++) for(UINT col = 0; col < g.Cols(); col++)
    {
        double v = g(row, col);
        if(v > 0.05) values.PushEnd(v);
    }
    values.Sort();
    if(values.Length() == 0) return 0.5;

    if(flip) return values[Math::Floor(values.Length() * (1.0 - quartile))];
    else return values[Math::Floor(values.Length() * quartile)];
}
