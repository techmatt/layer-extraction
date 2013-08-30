#include "Main.h"

double MeshSampler::DirectionalSurfaceArea(const BaseMesh &m, const Vec3f &direction, float targetOrientation, float orientThresh)
{
    const UINT TriangleCount = m.FaceCount();
    const DWORD *MyIndices = m.Indices();
    const MeshVertex *MyVertices = m.Vertices();
    double Result = 0.0;
    for(UINT TriangleIndex = 0; TriangleIndex < TriangleCount; TriangleIndex++)
    {
        Vec3f V[3];
        for(UINT LocalVertexIndex = 0; LocalVertexIndex < 3; LocalVertexIndex++)
        {
            V[LocalVertexIndex] = MyVertices[MyIndices[TriangleIndex * 3 + LocalVertexIndex]].Pos;
        }
        Vec3f normal = Math::TriangleNormal(V[0], V[1], V[2]);
        if (fabs(Vec3f::Dot(normal, direction) - targetOrientation) <= orientThresh)
            Result += Math::TriangleArea(V[0], V[1], V[2]);
    }
    return Result;
}

void MeshSampler::Sample(const BaseMesh &m, UINT sampleCount, Vector<MeshSample> &samples)
{
    double totalArea = m.SurfaceArea();
    double areaScale = 1.0 / totalArea;
    if(totalArea == 0.0)
    {
        samples.FreeMemory();
        return;
    }

    samples.Allocate(sampleCount);

    UINT samplingTriangleIndex = 0;
    double samplingTriangleAreaRatio = GetTriangleArea(m, samplingTriangleIndex) * areaScale;
    double accumulatedAreaRatio = samplingTriangleAreaRatio;

    for(UINT sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
        double intervalMin = double(sampleIndex) / double(sampleCount);
        double intervalMax = double(sampleIndex + 1) / double(sampleCount);
        double sampleValue = intervalMin + rnd() * (intervalMax - intervalMin);

        while(accumulatedAreaRatio < sampleValue && samplingTriangleIndex < m.FaceCount() - 1)
        {
            samplingTriangleIndex++;
            samplingTriangleAreaRatio = GetTriangleArea(m, samplingTriangleIndex) * areaScale;
            accumulatedAreaRatio += samplingTriangleAreaRatio;
        }

        double triangleValue = Utility::Bound( Math::LinearMap(accumulatedAreaRatio - samplingTriangleAreaRatio, accumulatedAreaRatio, 0.0, 1.0, sampleValue), 0.0, 1.0 );
        samples[sampleIndex] = SampleTriangle(m, samplingTriangleIndex, triangleValue);
    }
}

void MeshSampler::Sample(const BaseMesh &m, UINT& sampleCount, float densityThresh, const Vec3f &direction, float targetOrientation, float orientThresh, Vector<MeshUVSample> &samples)
{
    double totalArea = DirectionalSurfaceArea(m, direction, targetOrientation, orientThresh);
    double areaScale = 1.0 / totalArea;
    if(totalArea == 0.0)
    {
        sampleCount = 0;
        samples.FreeMemory();
        return;
    }

    // Threshold sample count according to maximum density criterion
    sampleCount = Math::Min(sampleCount, UINT(totalArea / densityThresh));

    samples.Allocate(sampleCount);

    // Indices of triangles with good normals
    Vector<UINT> goodTriIndices;
    for (UINT i = 0; i < m.FaceCount(); i++)
    {
        Vec3f normal = GetTriangleNormal(m, i);
        if (fabs(Vec3f::Dot(normal, Vec3f::eZ) - targetOrientation) <= orientThresh)
            goodTriIndices.PushEnd(i);
    }

    UINT indexIntoGoodTris = 0;
    UINT samplingTriangleIndex = goodTriIndices[indexIntoGoodTris];
    double samplingTriangleAreaRatio = GetTriangleArea(m, samplingTriangleIndex) * areaScale;
    double samplingTriangleOrientation = Vec3f::Dot(GetTriangleNormal(m, samplingTriangleIndex), Vec3f::eZ);
    double accumulatedAreaRatio = samplingTriangleAreaRatio;

    for(UINT sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
        double intervalMin = double(sampleIndex) / double(sampleCount);
        double intervalMax = double(sampleIndex + 1) / double(sampleCount);
        double sampleValue = intervalMin + rnd() * (intervalMax - intervalMin);

        while(accumulatedAreaRatio < sampleValue && indexIntoGoodTris < goodTriIndices.Length() - 1)
        {
            indexIntoGoodTris++;
            samplingTriangleIndex = goodTriIndices[indexIntoGoodTris];
            samplingTriangleAreaRatio = GetTriangleArea(m, samplingTriangleIndex) * areaScale;
            samplingTriangleOrientation = Vec3f::Dot(GetTriangleNormal(m, samplingTriangleIndex), Vec3f::eZ);
            accumulatedAreaRatio += samplingTriangleAreaRatio;
        }

        double triangleValue = Utility::Bound( Math::LinearMap(accumulatedAreaRatio - samplingTriangleAreaRatio, accumulatedAreaRatio, 0.0, 1.0, sampleValue), 0.0, 1.0 );

        MeshUVSample &curSample = samples[sampleIndex];
        curSample = SampleTriangleUV(m, samplingTriangleIndex, triangleValue);
        curSample.triangleIndex = samplingTriangleIndex;
        curSample.meshIndex = 0;
    }
}

void MeshSampler::Sample(const Vector< pair<const BaseMesh *, Matrix4> > &meshList, UINT sampleCount, Vector<MeshSample> &samples)
{
    Mesh allMeshes;
    allMeshes.LoadMeshList(meshList);
    Sample(allMeshes, sampleCount, samples);
}

void MeshSampler::Sample(const Vector< pair<const BaseMesh *, Matrix4> > &meshList, UINT sampleCount, float densityThresh, const Vec3f &direction, float targetOrientation, float orientThresh, Vector<MeshUVSample> &samples)
{
    Mesh allMeshes;
    allMeshes.LoadMeshList(meshList);
    Sample(allMeshes, sampleCount, densityThresh, direction, targetOrientation, orientThresh, samples);

    Vector<UINT> triangleToMeshIndex(allMeshes.FaceCount());
    Vector<UINT> meshIndexToBaseTriangleIndex(meshList.Length());
    UINT curIndex = 0;
    for(UINT meshIndex = 0; meshIndex < meshList.Length(); meshIndex++)
    {
        meshIndexToBaseTriangleIndex[meshIndex] = curIndex;
        if(meshList[meshIndex].first != NULL)
        {
            const UINT triangleCount = meshList[meshIndex].first->FaceCount();
            for(UINT triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
            {
                triangleToMeshIndex[curIndex++] = meshIndex;
            }
        }
    }
    
    for(UINT sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
        MeshUVSample &curSample = samples[sampleIndex];
        curSample.meshIndex = triangleToMeshIndex[curSample.triangleIndex];
        curSample.triangleIndex = curSample.triangleIndex - meshIndexToBaseTriangleIndex[curSample.meshIndex];
    }
}

float MeshSampler::GetTriangleArea(const BaseMesh &m, UINT triangleIndex)
{
    const DWORD *indices = m.Indices();
    const MeshVertex *vertices = m.Vertices();

    Vec3f V[3];
    for(UINT vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
        V[vertexIndex] = vertices[indices[triangleIndex * 3 + vertexIndex]].Pos;
    }
    return Math::TriangleArea(V[0], V[1], V[2]);
}

Vec3f MeshSampler::GetTriangleNormal(const BaseMesh &m, UINT triangleIndex)
{
    const DWORD *indices = m.Indices();
    const MeshVertex *vertices = m.Vertices();

    Vec3f V[3];
    for(UINT vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
        V[vertexIndex] = vertices[indices[triangleIndex * 3 + vertexIndex]].Pos;
    }
    return Math::TriangleNormal(V[0], V[1], V[2]);
}

MeshSample MeshSampler::SampleTriangle(const BaseMesh &m, UINT triangleIndex, double sampleValue)
{
    const DWORD *indices = m.Indices();
    const MeshVertex *vertices = m.Vertices();

    Vec3f V[3];
    for(UINT vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
        V[vertexIndex] = vertices[indices[triangleIndex * 3 + vertexIndex]].Pos;
    }

    MeshSample result;
    
    result.normal = Math::TriangleNormal(V[0], V[1], V[2]);

    Vec2f uv = StratifiedSample2D(sampleValue);
    if(uv.x + uv.y > 1.0f)
    {
        uv = Vec2f(1.0f - uv.y, 1.0f - uv.x);
    }

    result.pos = V[0] + (V[1] - V[0]) * uv.x + (V[2] - V[0]) * uv.y;

    return result;
}

MeshUVSample MeshSampler::SampleTriangleUV(const BaseMesh &m, UINT triangleIndex, double sampleValue)
{
    const DWORD *indices = m.Indices();
    const MeshVertex *vertices = m.Vertices();

    Vec3f V[3];
    for(UINT vertexIndex = 0; vertexIndex < 3; vertexIndex++)
    {
        V[vertexIndex] = vertices[indices[triangleIndex * 3 + vertexIndex]].Pos;
    }

    Vec2f uv = StratifiedSample2D(sampleValue);
    if(uv.x + uv.y > 1.0f)
    {
        uv = Vec2f(1.0f - uv.y, 1.0f - uv.x);
    }

    MeshUVSample result;
    result.uv = uv;
    result.pos = V[0] + (V[1] - V[0]) * uv.x + (V[2] - V[0]) * uv.y;
    result.normal = Math::TriangleNormal(V[0], V[1], V[2]);

    return result;
}

Vec2f MeshSampler::StratifiedSample2D(double s, UINT depth)
{
    if(depth == 10)
    {
        return Vec2f(rnd(), rnd());
    }

    Vec2f basePoint;
    double baseValue;
    if(s < 0.25)
    {
        baseValue = 0.0;
        basePoint = Vec2f(0.0f, 0.0f);
    }
    else if(s < 0.5)
    {
        baseValue = 0.25;
        basePoint = Vec2f(0.5f, 0.0f);
    }
    else if(s < 0.75)
    {
        baseValue = 0.5;
        basePoint = Vec2f(0.0f, 0.5f);
    }
    else
    {
        baseValue = 0.75;
        basePoint = Vec2f(0.5f, 0.5f);
    }

    return basePoint + StratifiedSample2D((s - baseValue) * 4.0, depth + 1) * 0.5;
}
