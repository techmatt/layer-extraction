class DistanceTransform
{
public:
    static Grid<double> Transform(const Grid<double> &g);
    static Grid<double> Transform(const Grid<double> &g, double quartile);

private:
    static double GetThreshold(const Grid<double> &g, double quartile);
    static Vector<Vec2i> GetNeighbors(const Grid<double> &g, const Vec2i &p);

};