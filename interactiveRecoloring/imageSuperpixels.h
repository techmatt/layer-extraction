
struct Superpixel
{
	Superpixel()
	{
		targetColorWeight = 0.0f;
		targetColor = vec3f::origin;
		
		constraintType = ConstraintType::Regularization;
		targetColorWeight = appParams().regularizationWeight;
	}

	enum class ConstraintType
	{
		Regularization,
		PassiveStasis,
		ActiveStasis,
		Edit,
	};

	/*void addColorTarget(const vec3f &c, float weight)
	{
		targetColorWeight += weight;
		targetColor += c * weight;
	}*/

	void recordConstraint(ConstraintType type, const vec3f &_targetColor)
	{
		if ((int)type <= (int)constraintType) return;
		constraintType = type;
		
		targetColor = _targetColor;
		if (constraintType == ConstraintType::PassiveStasis) targetColorWeight = appParams().passiveStasisWeight;
		if (constraintType == ConstraintType::ActiveStasis) targetColorWeight = appParams().activeStasisWeight;
		if (constraintType == ConstraintType::Edit) targetColorWeight = appParams().editWeight;
	}

	SuperpixelCoord coord;

	//
	// constraint data
	//
	float targetColorWeight;
	vec3f targetColor;
	ConstraintType constraintType;

	//
	// neighboring superpixel data
	//
	vector<unsigned int> neighborIndices;
	vector<double> neighborEmbeddingWeights;
	vector<double> neighborSimilarityWeights;

	float constraintDist;
	bool visited;
};

struct SuperpixelQueueEntry
{
	SuperpixelQueueEntry(Superpixel &_s, double _constraintDist)
	{
		s = &_s;
		constraintDist = _constraintDist;
	}
	Superpixel *s;
	double constraintDist;
};

inline bool operator < (const SuperpixelQueueEntry &a, const SuperpixelQueueEntry &b)
{
	return (a.constraintDist > b.constraintDist);
}

struct PixelNeighborhood
{
	// indices into the superpixel list
	vector<unsigned int> neighbors;

	vector<double> weights;
};

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator<<(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, const Superpixel &n) {
	s.writeData(n.coord);
	s << n.targetColorWeight << n.targetColor;
	s.writeData(n.constraintType);
	s << n.neighborIndices << n.neighborEmbeddingWeights << n.neighborSimilarityWeights;
	return s;
}

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator >> (BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, Superpixel &n) {
	s.readData(n.coord);
	s >> n.targetColorWeight >> n.targetColor;
	s.readData(n.constraintType);
	s >> n.neighborIndices >> n.neighborEmbeddingWeights >> n.neighborSimilarityWeights;
	return s;
}

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator<<(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, const PixelNeighborhood &n) {
	s << n.neighbors << n.weights;
	return s;
}

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator >> (BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, PixelNeighborhood &n) {
	s >> n.neighbors >> n.weights;
	return s;
}

struct ImageSuperpixels
{
	void loadCoords(const vector<SuperpixelCoord> &coords)
	{
		superpixels.resize(coords.size());
		for (auto &i : iterate(superpixels))
			i.value.coord = coords[i.index];
	}

	void loadEdits(const Bitmap &imgInput, const Bitmap &imgEdit);

	void computeNeighborhoods(const Bitmap &imgInput);
	void computeNeighborhoodWeights(const Bitmap &imgInput);

	vector<double> computeWeights(const vector<unsigned int>& indices, const float * pixelFeatures);

	void computeConstraintDists();
	
	vector<Superpixel> superpixels;
	Grid2<int> assignments;

	Grid2<PixelNeighborhood> pixelNeighborhoods;

	float maxConstraintDist;

	void saveToFile(const string &filename) const
	{
		BinaryDataStreamFile out(filename, true);
		out << superpixels << assignments << pixelNeighborhoods;
		out.closeStream();
	}

	void loadFromFile(const string &filename)
	{
		BinaryDataStreamFile in(filename, false);
		in >> superpixels;
		in >> assignments;
		in >> pixelNeighborhoods;
		in.closeStream();
	}
};
