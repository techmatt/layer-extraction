
struct Layer
{
	Grid2f g;
	vec3f baseColor;
};

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator<<(BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, const Layer &l) {
	s << l.baseColor;
	s.writePrimitive(l.g);
	return s;
}

template<class BinaryDataBuffer, class BinaryDataCompressor>
inline BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& operator >> (BinaryDataStream<BinaryDataBuffer, BinaryDataCompressor>& s, Layer& l) {
	s >> l.baseColor;
	s.readPrimitive(l.g);
	return s;
}

struct ImageLayers
{
	void loadCSV(const string &baseDir);
	void loadDAT(const string &baseDir);
	void saveDAT(const string &baseDir) const;
	Bitmap compositeImage(const vector<vec3f> &layerColors) const;

	int dimX;
	int dimY;
	vector<Layer> layers;
};