#pragma once

#ifdef USE_OPENCV
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"

struct KeyPointInfo
{
	cv::KeyPoint keypoint;
	const PixelLayer* parent;
};

class BagOfWords
{
public:
	BagOfWords(const AppParameters& params, Vector<PixelLayerSet> images, UINT numWords, bool loadIfNeeded=false);

	//void VisualizeBag();
	void VisualizeWords();
	VecNf Wordify(const PixelLayer &layer);
	void VisualizeBag(const VecNf &bag, String filename);

private:

	float GaussianKernel(float x, float sigma);
	cv::Mat SampleLayer(const PixelLayer& layer, int idx, Vector<KeyPointInfo>& allKeypoints);

	KDTree _tree;
	cv::Mat _words;
	int _drawScale;

};

#endif
