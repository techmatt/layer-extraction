#include "Main.h"

Descriptors::Descriptors(const Bitmap &bmp)
{
	//Convert the area around the bitmap to a Mat
	unsigned char* data = new unsigned char[bmp.Width()*bmp.Height()*3];
	int height = bmp.Height();
	int width = bmp.Width();

	for (int r=0; r<height; r++)
	{
		for (int c=0; c<width; c++)
		{
			int index = 3*(r*width+c);
			data[index++] = bmp[r][c].r;
			data[index++] = bmp[r][c].g;
			data[index++] = bmp[r][c].g;
		}
	}
	image = cv::Mat(bmp.Height(), bmp.Width(), CV_8UC3, data);

	vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);


	//convolve the image with a filter bank of gabor filters
	//Create the filter bank
	double U_h = 0.25; //highest frequency
	double U_l = 0.05; //lowest frequency
	double S = 4; //number of scales
	double K = 6; //number of orientations
	double ln2 = log(2);

	double a = pow(U_h/U_l, 1/(S-1));
	double sigma_u = (a-1)*U_h/((a+1)*sqrt(2*ln2));
	double sigma_v = tan(Math::PI/(2*K))*(U_h-2*log(2*sigma_u*sigma_u/U_h))/sqrt(2*ln2-4*ln2*ln2*sigma_u*sigma_u/(U_h*U_h));

	double sigma_x = 1.0/(2*Math::PI*sigma_u);
	double sigma_y = 1.0/(2*Math::PI*sigma_v);
	double gamma = sigma_x/sigma_y;

	double orientationStep =  Math::PI/K;
	double scaleStep = (U_h-U_l)/S;

	int idx=0;
	for (int scaleIndex=0; scaleIndex < S; scaleIndex++)
	{
		for (int orientationIndex=0; orientationIndex < K; orientationIndex++)
		{
			double theta = orientationIndex*orientationStep;
			double lambda = (scaleIndex+1)*scaleStep;

			cv::Mat filter = cv::getGaborKernel(cv::Size(-1,-1), sigma_x, theta, lambda, gamma); 
			cv::Mat filteredImage;
			cv::filter2D(image, filteredImage, -1, filter);

			//debug, output the filtered image
			cv::imwrite("GaborFiltered_"+std::string(String(idx).CString()), filteredImage, compression_params);
			cv::imwrite("GaborFilter_"+std::string(String(idx).CString()), filteredImage, compression_params);

			filteredImages.push_back(filteredImage);
			idx++;
		}
	}

}

VecNf Descriptors::ComputeSIFTDescriptor(Vec2i point)
{
	//Turn the point into a keypoint
	cv::Ptr<cv::DescriptorExtractor> extractor(new cv::SiftDescriptorExtractor);
	vector<cv::KeyPoint> keypoints;
	cv::KeyPoint keypoint(point.x, point.y, 1);
	keypoints.push_back(keypoint);

	cv::Mat descriptor;
	extractor->compute(image, keypoints, descriptor);
	
	VecNf vecDescriptor(descriptor.cols);
	for (int i=0; i<descriptor.cols; i++)
		vecDescriptor[i] = descriptor.at<float>(0, i);
	
	return vecDescriptor;
}

VecNf Descriptors::ComputeGaborFeatures(Vec2i point)
{
	return VecNf();
}

