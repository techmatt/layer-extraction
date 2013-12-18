#include "Main.h"
#include "BagOfWords.h"
#include "opencv2/imgproc/imgproc.hpp"


BagOfWords::BagOfWords(const AppParameters& params, Vector<PixelLayerSet> images, UINT numWords, bool loadIfNeeded)
{
	//sample a bunch of points in each layer, in each image
	//extract the patches, and do K-means clustering on them
	Console::WriteLine("Creating words");

	_drawScale = 16;

	//load if needed
	if (loadIfNeeded && Utility::FileExists("../BoW/dictionary.yml"))
	{ 
		cv::FileStorage fs("../BoW/dictionary.yml", cv::FileStorage::READ);
		fs["vocabulary"] >> _words;
		fs.release();   
	} else
	{
		cv::Mat samples;
		Vector<KeyPointInfo> allKeyPoints;

		Console::WriteLine("Sampling neighborhoods");
		int idx = 0;
		for (const PixelLayerSet &layerSet:images)
		{
			for (const PixelLayer &layer:layerSet)
			{
				samples.push_back(SampleLayer(layer, idx, allKeyPoints));
				idx++;
			}
		}

		Console::WriteLine("Clustering words");
		cv::TermCriteria tc(CV_TERMCRIT_ITER,100,0.001);
		int retries=1;
		int flags=cv::KMEANS_PP_CENTERS;
		cv::BOWKMeansTrainer bowTrainer(numWords,tc,retries,flags);
		_words = bowTrainer.cluster(samples);

		Console::WriteLine("Find representative patches");
		//find representative keypoints in the image for each word
		Vector<const float*> floatsamples;
		for (int r=0; r<samples.rows; r++)
		{
			float* sample = new float[samples.cols];
			memcpy(sample, (float*)samples.row(r).data, samples.cols*sizeof(float));
			floatsamples.PushEnd(sample);
		}
		Console::WriteLine("Build KD-Tree");
		_tree.BuildTree(floatsamples, samples.cols, 1);

		Console::WriteLine("Searching");
		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(9);

		for (int r=0; r<_words.rows; r++)
		{
			Console::WriteLine("Word "+String(r));
			float* word = new float[_words.cols];
			memcpy(word, (float*)_words.row(r).data, sizeof(float)*_words.cols);
			Vector<unsigned int> indices;
			_tree.KNearest(word, 1, indices, 0.0f);

			//get the word image, and save it as an image
			KeyPointInfo &info = allKeyPoints[indices[0]];
			cv::Point2f point = info.keypoint.pt;

			//just magnify it
			//TODO: not sure how to calculate actual patch size
			float diameter = info.keypoint.size*_drawScale;
			
			Console::WriteLine("Get word image");
			const PixelLayer& layer = *info.parent;
			int width = layer.Width();
			int height = layer.Height();
			unsigned char* data = new unsigned char[width*height];
			for (int i=0; i<height; i++)
				for (int j=0; j<width; j++)
					data[i*width+j] = MAX(0, MIN(1, layer.pixelWeights(i, j)))*255;

			cv::Mat image(height, width, CV_8UC1, data);
			cv::Rect rect(Utility::Bound(Math::Round(point.x-diameter/2.0),0,Math::Max(0, Math::Round(width-diameter))), Utility::Bound(Math::Round(point.y-diameter/2.0), 0, Math::Max(0, Math::Round(height-diameter))), Utility::Bound(Math::Round(diameter), 0, width), Utility::Bound(Math::Round(diameter), 0, height));
			
			Console::WriteLine(String(rect.x)+" "+String(rect.y)+ " " + String(rect.width)+" " + String(rect.height));
		
			cv::Size size(_drawScale, _drawScale);
			cv::Mat resized(size, CV_8UC1, 0);
		
			Console::WriteLine("Cropping");
			cv::Mat source = image(rect);
			Console::WriteLine("Resize");
			cv::resize(source, resized, size);

			Console::WriteLine("Save");
			imwrite("../BoW/Word_"+std::string(String(r).CString())+".png", resized, compression_params);

			delete[] data;
			delete[] word;

		}

		//Save the words
		cv::FileStorage fs("../BoW/dictionary.yml", cv::FileStorage::WRITE);
		fs << "vocabulary" << _words;
		fs.release();
	}

	Console::WriteLine("Visualizing words");
	VisualizeWords();
}

cv::Mat BagOfWords::SampleLayer(const PixelLayer& layer, int idx, Vector<KeyPointInfo>& allKeyPoints)
{
	cv::Mat samples;
	int width = layer.Width();
	int height = layer.Height();


    cv::Ptr<cv::FeatureDetector> detector(new cv::SiftFeatureDetector());
	//cv::Ptr<cv::FeatureDetector> detector(new cv::DenseFeatureDetector(1.f, 1));
    cv::Ptr<cv::DescriptorExtractor> extractor(new cv::SiftDescriptorExtractor);

	//use SIFT to sample features
	vector<cv::KeyPoint> keypoints;

	Console::WriteLine("Converting layer to OpenCV Mat format, and convert to 255bit grayscale");
	unsigned char* data = new unsigned char[width*height];
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
			data[i*width+j] = MAX(0, MIN(1, layer.pixelWeights(i, j)))*255;

	Console::WriteLine("Constructing...");
	//cv::Mat image(height, width, CV_64FC1, data);
	cv::Mat image(height, width, CV_8UC1, data);

	vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);

	cv::imwrite("matImage.png", image, compression_params);

	Console::WriteLine("Detecting keypoints");
	detector->detect(image, keypoints);
	Console::WriteLine("Extracting keypoints");
	extractor->compute(image, keypoints,samples); 

	Console::WriteLine("Num of keypoints " + String(keypoints.size()));
	cv::Mat output;
	cv::drawKeypoints(image, keypoints, output, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	imwrite("keypoints_"+std::string(String(idx).CString())+".png", output, compression_params);

	for (cv::KeyPoint &k:keypoints)
	{
		KeyPointInfo info;
		info.keypoint = cv::KeyPoint(k);
		info.parent = &layer;
		allKeyPoints.PushEnd(info);
	}

	//cleanup
	Console::WriteLine("Cleanup");
	delete[] data;

	return samples;

	/*
	const UINT dimension = _generator.Dimension();
	const UINT width = layer.Width();
	const UINT height = layer.Height();

	Vector<VecNf> samples;

	for (int sampleIndex=0; sampleIndex < numSamples; sampleIndex++)
	{
		double *neighborhood = new double[dimension];
		bool success = false;
		while(!success)
		{
			Vec2i centerPt = Vec2i(rand() % width, rand() % height);
			int xCenter = centerPt.x;
			int yCenter = centerPt.y;

			success = _generator.Generate(layer, xCenter, yCenter, neighborhood);
		}

		VecNf vec(dimension);
		for (int i=0; i<dimension; i++) vec[i] = (float)neighborhood[i];
		samples.PushEnd(vec);

		delete[] neighborhood;
	}
	return samples;
	*/
}

void BagOfWords::VisualizeWords()
{
	
	int clusterCount = _words.rows;
	int wordSize = _drawScale;
	int padding = 10;
	int rowSize = 10;

	Console::WriteLine("clusterCount " + String(clusterCount));
	Console::WriteLine("wordSize " + String(wordSize));

	Bitmap result(rowSize*(wordSize+padding), (wordSize+padding)*(clusterCount+1)/rowSize, RGBColor(255,255,255));

	for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
	{
		int startX = (clusterIndex%rowSize)*(wordSize+padding);
		int startY = (clusterIndex/rowSize)*(wordSize+padding);
		
		Bitmap word;
		word.LoadPNG("../BoW/Word_"+String(clusterIndex)+".png");

		for (int x=0; x<wordSize; x++)
			for (int y=0; y<wordSize; y++)
				result[startY+y][startX+x] = word[y][x];
	}

	result.SavePNG("Words.png");

}

VecNf BagOfWords::Wordify(const PixelLayer &layer)
{
	Console::WriteLine("Wordify");
	//For the given layer, create a histogram over the words
	//then normalize
	int width = layer.Width();
	int height = layer.Height();

	Console::WriteLine("Set vocab");
    cv::Ptr<cv::FeatureDetector> detector(new cv::SiftFeatureDetector());
	//cv::Ptr<cv::FeatureDetector> detector(new cv::DenseFeatureDetector(1.f, 1));
    cv::Ptr<cv::DescriptorExtractor> extractor(new cv::SiftDescriptorExtractor);
	
	cv::Ptr<cv::DescriptorMatcher> matcher(new cv::FlannBasedMatcher);
    cv::BOWImgDescriptorExtractor bowDE(extractor,matcher);
	bowDE.setVocabulary(_words);

	Console::WriteLine("Converting");
	//convert layer to opencv mat format
	unsigned char* data = new unsigned char[width*height];
	for (int i=0; i<height; i++)
		for (int j=0; j<width; j++)
			data[i*width+j] = MAX(0, MIN(1, layer.pixelWeights(i, j)))*255;

	Console::WriteLine("Constructing...");
	cv::Mat image(height, width, CV_8UC1, data);

	vector<cv::KeyPoint> keypoints;
	detector->detect(image, keypoints);

	cv::Mat descriptor;
	bowDE.compute(image, keypoints, descriptor);

	delete[] data;

	//convert descriptor to VecNF
	VecNf bag(_words.rows);
	for(int i=0; i<_words.rows; i++)
	{
		if (i >= descriptor.cols)
			bag[i] = 0;
		else
			bag[i] = (float)descriptor.at<float>(0,i);
	}
	

	return bag;
}

void BagOfWords::VisualizeBag(const VecNf &bag, String filename)
{
	Console::WriteLine("Visualize Bag");
	int clusterCount = _words.rows;
	int wordSize = _drawScale;
	int padding = 10;
	int rowSize = clusterCount;
	int barHeight = 80;

	Bitmap result(clusterCount*(wordSize+padding), (wordSize+padding)+barHeight, RGBColor(255,255,255));

	for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
	{
		int startX = (clusterIndex%rowSize)*(wordSize+padding);
		int startY = (clusterIndex/rowSize)*(wordSize+padding)+barHeight;
		
		//Read the word images
		Bitmap word;
		word.LoadPNG("../BoW/Word_"+String(clusterIndex)+".png");

		for (int x=0; x<wordSize; x++)
		{
			for (int y=0; y<wordSize; y++)
			{
				result[startY+y][startX+x] = word[y][x];
			}
		}

		int height = barHeight * bag[clusterIndex];

		//draw the bar
		for (int y=startY-1; y>=startY-height; y--)
		{
			result[y][startX+(int)(0.5*wordSize)-1] = RGBColor::Blue;
			result[y][startX+(int)(0.5*wordSize)] = RGBColor::Blue;
			result[y][startX+(int)(0.5*wordSize)+1] = RGBColor::Blue;
		}

	}

	result.SavePNG(filename);
}

float BagOfWords::GaussianKernel(float x, float sigma)
{
	//ignoring the constant in front
	float u = x/sigma;
	return exp(-0.5*u*u);
}


