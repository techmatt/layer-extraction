class DistanceTransform
{
public:
    static Grid<double> Transform(const Grid<double> &g);
    static Grid<double> Transform(const Grid<double> &g, double quartile, double power, bool flip);

private:
    static double GetThreshold(const Grid<double> &g, double quartile, bool flip);
    static Vector<Vec2i> GetNeighbors(const Grid<double> &g, const Vec2i &p);

};