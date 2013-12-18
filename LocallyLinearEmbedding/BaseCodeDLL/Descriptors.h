#pragma once
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"

/**
 * Compute various descriptors
 *
 */

class Descriptors
{
	public:
		Descriptors(const Bitmap& bmp);
		//compute the SIFT descriptor at this point (current scale)
		VecNf ComputeSIFTDescriptor(Vec2i point);
		VecNf ComputeGaborFeatures(Vec2i point);

	private:
		cv::Mat image;
		vector<cv::Mat> filteredImages;
};