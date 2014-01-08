#include "Main.h"

void LayerSet::Dump(const String &filename, const Vector<ColorCoordinate> &superpixelColors) const
{
	ofstream file(filename.CString());
	const UINT superpixelCount = layers[0].superpixelWeights.Length();

	file << "index\tcolor\treconstructed\tcolor diff magnitude\tweight sum\tindex\tweights" << endl;
	for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		file << superpixelIndex << '\t';
		file << superpixelColors[superpixelIndex].color.CommaSeparatedString() << '\t';

		Vec3f reconstructedColor = Vec3f::Origin;
		for(const Layer &l : layers) reconstructedColor += (float)l.superpixelWeights[superpixelIndex] * l.color;

		file << RGBColor(reconstructedColor).CommaSeparatedString() << '\t';

		file << (reconstructedColor - Vec3f(superpixelColors[superpixelIndex].color)).Length() * 255.0f << '\t';

		double weightSum = 0.0;
		for(const Layer &l : layers) weightSum += l.superpixelWeights[superpixelIndex];
		file << weightSum << '\t';

		file << superpixelIndex << '\t';
		for(const Layer &l : layers) file << l.superpixelWeights[superpixelIndex] << '\t';
		file << endl;
	}
}

void LayerExtractor::Init(const AppParameters &parameters, const Bitmap &bmp)
{
    ComputeSuperpixels(parameters, bmp);
    //VisualizeSuperpixels(parameters, bmp, NULL, "superpixels");

    ComputeNearestNeighbors(parameters, bmp);
    //VisualizeNearestNeighbors(parameters, bmp);

	ComputeNeighborWeights(parameters, bmp);
	//TestNeighborWeights(parameters, bmp);

	ComputeWeightMatrix(parameters);

	pass = 0;
	correctionPass = 0;
}

void LayerExtractor::AddNegativeConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result)
{
	String key = "negative";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());

	map<UINT64,UINT> activeConstraints;
	for(UINT constraintIndex = 0; constraintIndex < result.constraints[key].Length(); constraintIndex++)
	{
		const auto &c = result.constraints[key][constraintIndex];
		activeConstraints[UINT64(c.index) + (UINT64(c.layerIndex) << 32)] = constraintIndex;
	}

	const UINT layerCount = result.layers.Length();
	const UINT superpixelCount = superpixelColors.Length();
	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		Layer &curLayer = result.layers[layerIndex];
		for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
		{
			UINT64 constraintHash = UINT64(superpixelIndex) + (UINT64(layerIndex) << 32);
			if(curLayer.superpixelWeights[superpixelIndex] < 0.0)
			{
				if(activeConstraints.count(constraintHash) == 0)
				{
					result.constraints[key].PushEnd(SuperpixelLayerConstraint(superpixelIndex, layerIndex, 0.0, parameters.negativeSupressionWeight));
				}
				else
				{
					auto &c = result.constraints[key][activeConstraints[constraintHash]];
					if(c.target == 0.0) c.weight += parameters.negativeSupressionWeight;
				}
			}
		}
	}
	pass++;
}


bool LayerExtractor::CorrectLayerSet(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers)
{
	//Operations: add color, remove color, or merge colors
	//Remove layer colors that have a contribution less than a threshold
	//If the mean negative weight is greater than a threshold, add the superpixel color that has the most negative weight from all layers
	//TODO: If two layers have an overlap greater than a threshold and have similar enough colors, merge the two layers
	Console::WriteLine("Correcting layers");

	Vector<int> toRemove;
	double weightThresh = 0.1;
	double negativeThresh = -0.07;//-0.05;

	double meanNegativity = 0;
	double worstNegWeight = 0;
	int superpixelIndexToAdd = -1;
	bool changed = false;


	int origLayerCount = layers.layers.Length();
	int* newLayerIndices = new int[origLayerCount];
	for (UINT i=0; i<layers.layers.Length(); i++)
		newLayerIndices[i] = i;


	Vector<Layer> newLayers;
	for (UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		double weight = layers.layers[layerIndex].MaxWeight();
		Console::WriteLine("Layer " + String(layerIndex) + ":" + String(weight));
		if (weight < weightThresh)
		{
			Console::WriteLine("Removing layer: "+String(layerIndex) + " " + layers.layers[layerIndex].color.CommaSeparatedString());
			toRemove.PushEnd(layerIndex);
			newLayerIndices[layerIndex] = -1;
			changed = true;
		} else
			newLayers.PushEnd(layers.layers[layerIndex]);
		meanNegativity = Math::Min(layers.layers[layerIndex].AverageNegative(), meanNegativity);
	}


	int negatives = 0;
	for (int i = 0; i < origLayerCount; i++)
	{
		if (newLayerIndices[i] == -1)
			negatives--;
		else
			newLayerIndices[i] += negatives;
	}

	//update the layer constraints and indices

	Dictionary<String, Vector<SuperpixelLayerConstraint>> newConstraints;

	//update the initial pixel layer constraints, recompute the midpoint and preference constraints
	/*if (layers.constraints.ContainsKey("initial"))
	{
	for (int constraintIndex = 0; constraintIndex < layers.constraints["initial"].Length(); constraintIndex++)
	{
	SuperpixelLayerConstraint& constraint = layers.constraints["initial"][constraintIndex];
	if (newLayerIndices[constraint.layerIndex] >= 0)
	{
	constraint.layerIndex = newLayerIndices[constraint.layerIndex];
	if (!newConstraints.ContainsKey("initial"))
	newConstraints.Add("initial", Vector<SuperpixelLayerConstraint>());
	newConstraints["initial"].PushEnd(constraint);
	}
	}
	}*/

	layers.layers = newLayers;

	Vector<double> sumNegWeights(superpixelColors.Length(), 0);

	//add the most negative color if necessary
	if (meanNegativity < negativeThresh)
	{
		Console::WriteLine("LayerSet too negative: "+String(meanNegativity));
		for (UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
		{
			for (UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
			{
				double weight = layers.layers[layerIndex].superpixelWeights[superpixelIndex];
				if (weight < 0)
					sumNegWeights[superpixelIndex] += weight;
			}
		}
		superpixelIndexToAdd = sumNegWeights.MinIndex();
	}


	if (superpixelIndexToAdd > -1)
	{
		Layer newLayer;
		newLayer.color = Vec3f(superpixelColors[superpixelIndexToAdd].color);
		Console::WriteLine("Adding color: "+newLayer.color.CommaSeparatedString());
		layers.layers.PushEnd(newLayer);

		/*if (!newConstraints.ContainsKey("initial"))
		newConstraints.Add("initial", Vector<SuperpixelLayerConstraint>());

		newConstraints["initial"].PushEnd(SuperpixelLayerConstraint(superpixelIndexToAdd, layers.layers.Length()-1, 1, parameters.pixelConstraintWeight));
		for (int layerIndex = 0; layerIndex < layers.layers.Length()-1; layerIndex++)
		newConstraints["initial"].PushEnd(SuperpixelLayerConstraint(superpixelIndexToAdd, layerIndex, 0, parameters.pixelConstraintWeight));*/
		changed = true;
	}

	layers.constraints = newConstraints;


	VisualizeLayerPalette(parameters, bmp, layers, correctionPass, superpixelIndexToAdd);

	//just re-do the constraints
	auto palette = layers.layers.Map(function<Vec3f(Layer)>([](Layer l){return l.color;}));

	InitLayersFromPalette(parameters, bmp, palette, layers);
	AddLayerPreferenceConstraints(parameters, bmp, layers);
	AddMidpointConstraints(parameters, bmp, layers);


	correctionPass++;

	return changed;

}



void LayerExtractor::InitLayersFromPixelConstraints(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, LayerSet &result)
{
	ComponentTimer timer("Making layers from pixel constraints");

	String key = "initial";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());

	Vector<RGBColor> uniqueColors;
	for(PixelConstraint p : targetPixelColors)
	{
		RGBColor c(p.targetColor);
		if(!uniqueColors.Contains(c)) uniqueColors.PushEnd(c);
	}

	const UINT layerCount = uniqueColors.Length();
	result.layers.Allocate(layerCount);

	set<UINT> constrainedSuperpixels;
	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		Layer &curLayer = result.layers[layerIndex];
		curLayer.color = Vec3f(uniqueColors[layerIndex]);

		for(PixelConstraint p : targetPixelColors)
		{
			if(RGBColor(p.targetColor) == uniqueColors[layerIndex])
			{
				UINT nearestSuperpixel = pixelNeighborhoods(p.coord.y, p.coord.x).indices[0];
				if(constrainedSuperpixels.count(nearestSuperpixel) == 0)
				{
					constrainedSuperpixels.insert(nearestSuperpixel);

					for(UINT layerIndexInner = 0; layerIndexInner < layerCount; layerIndexInner++)
					{
						double targetValue = 0.0;
						if(layerIndex == layerIndexInner) targetValue = 1.0;
						result.constraints[key].PushEnd(SuperpixelLayerConstraint(nearestSuperpixel, layerIndexInner, targetValue, parameters.pixelConstraintWeight));
					}
				}
			}
		}
	}

	VisualizeLayerConstraints(parameters, bmp, result);
}

void LayerExtractor::InitLayersFromPalette(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> &palette, LayerSet &result)
{
	ComponentTimer timer("Making layers from palette");
	String key = "initial";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());

	const double distScale = 1.0 / Math::Max(bmp.Width(), bmp.Height());

	const UINT layerCount = palette.Length();
	const UINT superpixelCount = superpixelColors.Length();
	result.layers.Allocate(layerCount);

	unordered_set<const ColorCoordinate*> constrainedSuperpixels;
	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		Layer &curLayer = result.layers[layerIndex];
		curLayer.color = palette[layerIndex];

		Vector<UINT> superpixelsToConstrain;

		const UINT bestSuperpixel = superpixelColors.MinIndex(
			[&curLayer](const ColorCoordinate &c)
		{
			return Vec3f::Dist(Vec3f(c.color), curLayer.color);
		});
		superpixelsToConstrain.PushEnd(bestSuperpixel);
		constrainedSuperpixels.insert(&superpixelColors[bestSuperpixel]);

		bool done = false;
		while(!done)
		{
			const auto &colors = superpixelColors;
			auto valueFunction = [&superpixelsToConstrain,&constrainedSuperpixels,&colors,&curLayer,distScale](const ColorCoordinate &c)
			{
				const double similarityThreshold = 0.03;
				const double minDistThreshold = 0.2;

				if(constrainedSuperpixels.count(&c) == 1) return 1.0;

				for(int i : superpixelsToConstrain)
				{
					if(Vec2i::Dist(colors[i].coord, c.coord) * distScale < minDistThreshold) return 1.0;
				}

				double dist = Vec3f::Dist(Vec3f(c.color), curLayer.color);
				if(dist > similarityThreshold) return 1.0;
				return dist;
			};

			const UINT bestSuperpixel = superpixelColors.MinIndex(valueFunction);
			if(valueFunction(superpixelColors[bestSuperpixel]) < 1.0)
			{
				superpixelsToConstrain.PushEnd(bestSuperpixel);
				constrainedSuperpixels.insert(&superpixelColors[bestSuperpixel]);
			} else done = true;
		}

		Console::WriteLine("Layer " + String(layerIndex) + " constraints: " + String(superpixelsToConstrain.Length()));
		for(int i : superpixelsToConstrain)
		{
			for(UINT layerIndexInner = 0; layerIndexInner < layerCount; layerIndexInner++)
			{
				double targetValue = 0.0;
				if(layerIndex == layerIndexInner) targetValue = 1.0;
				result.constraints[key].PushEnd(SuperpixelLayerConstraint(i, layerIndexInner, targetValue, parameters.pixelConstraintWeight));
			}
		}
	}

	VisualizeLayerConstraints(parameters, bmp, result);
}


void LayerExtractor::AddMidpointConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result)
{
	Console::WriteLine("Adding midpoint constraints");
	String key = "midpoint";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());

	//add a constraint to all pixels, encouraging them not to stray too far from 0.5
	const UINT layerCount = result.layers.Length();
	const UINT superpixelCount = superpixelColors.Length();

	for (UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
		for (UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
		{
			result.constraints[key].PushEnd(SuperpixelLayerConstraint(superpixelIndex, layerIndex, 0.5, parameters.midpointWeight));
		}
		Console::WriteLine("Done adding midpoint constraints");
}


void LayerExtractor::AddLayerPreferenceConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result)
{
	Console::WriteLine("Initializing pixel-layer preferences");
	String key = "preference";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());

	//make superpixel prefer the closest palette color
	const UINT layerCount = result.layers.Length();
	const UINT superpixelCount = superpixelColors.Length();

	for (UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		int bestLayerIndex = 0;
		double bestDist = std::numeric_limits<double>::max();

		//find the best/worst layer
		for (UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
		{
			double dist = Vec3f::Dist(result.layers[layerIndex].color, Vec3f(superpixelColors[superpixelIndex].color));
			if (dist < bestDist)
			{
				bestDist = dist;
				bestLayerIndex = layerIndex;
			}
		}

		//add the constraint
		result.constraints[key].PushEnd(SuperpixelLayerConstraint(superpixelIndex, bestLayerIndex, 1.0, parameters.preferenceWeight*(1-bestDist)));

	}
	VisualizeLayerPreferences(parameters, bmp, result);

}

void LayerExtractor::VisualizeLayerPreferences(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const
{	
	//make superpixel prefer the closest palette color
	const UINT layerCount = layers.layers.Length();
	const UINT superpixelCount = superpixelColors.Length();

	Bitmap result(bmp.Width(), bmp.Height(), RGBColor::Black);
	for (UINT y=0; y<bmp.Height(); y++) for (UINT x=0; x<bmp.Width(); x++)  result[y][x] = RGBColor(Vec3f(bmp[y][x])*0.2f);

	AliasRender render;

	for (UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		int bestLayerIndex = 0;
		double bestDist = std::numeric_limits<double>::max();

		//find the best/worst layer
		for (UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
		{
			double dist = Vec3f::Dist(layers.layers[layerIndex].color, Vec3f(superpixelColors[superpixelIndex].color));
			if (dist < bestDist)
			{
				bestDist = dist;
				bestLayerIndex = layerIndex;
			}
		}

		RGBColor color = RGBColor(layers.layers[bestLayerIndex].color * (1.0f-(float)bestDist));
		RGBColor border = RGBColor(layers.layers[bestLayerIndex].color);
		render.DrawSquare(result, superpixelColors[superpixelIndex].coord, 2, color, border);

	}
	result.SavePNG("../Results/preferences.png");
}

void LayerExtractor::ExtractLayers(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers)
{
	ComponentTimer timer("Extracting layers");

	//
	// There are five types of constraints going on:
	//
	// *** Linear embedding constraints
	// Mi = (I - W)^T * (I - W) -> These are the constraints that values should be a linear combination of their neighborhood. There is one of these for each set of layer weights
	// This forms a matrix which looks like (for 3 layers):
	// (M0 0  0 )(w0) = (0)
	// (0  M1 0 )(w1) = (0)
	// (0  0  M2)(w2) = (0)
	// Call this super-matrix Q
	//
	// *** Explicit layer constraints
	// Some superpixels in each layer are constrained directly to have specific weights.
	// lhs: C = lp x lp (diagonal matrix)
	// rhs: C * target
	// There may be multiple C matrices for different sets of constraints
	//
	// *** Sum-to-one constraints
	// The sum of weights for each superpixel should be 1.
	// lhs: S = p x lp
	// rhs: S^T * s
	//
	// *** Layer reconstruction constraints
	// The linear combination of colors should reconstruct the superpixel colors.
	// lhs: R = 3p x lp
	// rhs: R^T * r
	//
	// *** Regularization constraints
	// All else being equal, weights should be as small as possible to encourage near-zero regions.
	//
	// *** Least squares solutions
	// Ax = b
	// A^TAx = A^Tb
	//
	// *** Final matrix
	// (Q + sum(wC C) + wS S^T S + wR R^T R) x = sum(wC Crhs) + wS S^T Srhs + wR R^T Rrhs
	// (WBase + wC C)x = wC Crhs
	// where wC, wS, wR are three weights
	//

	//Vector<Eigen::Triplet<double> > weightMatrixTriplets;
	const UINT superpixelCount = superpixelColors.Length();
	const UINT layerCount = layers.layers.Length();

	//
	// Build the linear embedding constraint matrix Q
	//
	SparseMatrix<double> Q(layerCount * superpixelCount);
	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		const UINT layerOffset = layerIndex * superpixelCount;
		for(UINT row = 0; row < superpixelCount; row++)
		{
			const SparseRow<double> &curRow = WBase[row];
			for(const SparseElement<double> &e : curRow.Data)
			{
				Q.PushElement(row + layerOffset, e.Col + layerOffset, parameters.manifoldWeight*e.Entry);
			}
		}
	}

	//
	// Add simple (diagonal) constraints to lhs and lambda * target to rhs
	//
	Vector<double> Crhs(layerCount * superpixelCount, 0.0);
	Vector<String> constraintKeys = layers.constraints.Keys();
	for (String key : constraintKeys)
	{
		for(UINT constraintIndex = 0; constraintIndex < layers.constraints[key].Length(); constraintIndex++)
		{
			const SuperpixelLayerConstraint &constraint = layers.constraints[key][constraintIndex];
			int row = constraint.index + constraint.layerIndex * superpixelCount;

			Q.PushDiagonalElement(row, constraint.weight);
			Crhs[row] += constraint.weight * constraint.target;
		}
	}
	for(UINT row = 0; row < layerCount * superpixelCount; row++)
	{
		Q.PushDiagonalElement(row, parameters.regularizationWeight);
	}

	//
	// Construct the sum-to-one matrix
	//
	SparseMatrix<double> S(superpixelCount, layerCount * superpixelCount);
	Vector<double> Srhs(superpixelCount, parameters.sumToOneWeight);
	for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
		{
			S.PushElement(superpixelIndex, superpixelIndex + layerIndex * superpixelCount, 1.0);
		}
	}
	SparseMatrix<double> STranspose = S.Transpose();
	SparseMatrix<double> STS = STranspose * S;
	STS *= parameters.sumToOneWeight;
	Q += STS;

	//
	// Construct the reconstruction matrix
	//
	SparseMatrix<double> R(3 * superpixelCount, layerCount * superpixelCount);
	Vector<double> Rrhs(3 * superpixelCount, 0.0);
	for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		Vec3f superpixelColor = Vec3f(superpixelColors[superpixelIndex].color);

		for(UINT channelIndex = 0; channelIndex < 3; channelIndex++)
		{
			UINT constraintRow = superpixelIndex * 3 + channelIndex;
			Rrhs[constraintRow] = parameters.reconstructionWeight * superpixelColor[channelIndex];
			for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
			{
				R.PushElement(constraintRow, superpixelIndex + layerIndex * superpixelCount, layers.layers[layerIndex].color[channelIndex]);
			}
		}
	}
	SparseMatrix<double> RTranspose = R.Transpose();
	SparseMatrix<double> RTR = RTranspose * R;
	RTR *= parameters.reconstructionWeight;
	Q += RTR;

	Vector<double> b = Crhs + STranspose * Srhs + RTranspose * Rrhs;

	//dump the Q and b matrices to a text file (for MATLAB)
	//if this is the first pass (no negative constraints yet)
	if (pass == 0 && parameters.useMatlab)
	{
		Console::WriteLine("Dumping Matlab matrix files...");
		ofstream QFile("../Matlab/Q.txt");
		PersistentAssert(!QFile.fail(), "Failed to open file");

		for (int r=0; r<Q.RowCount(); r++)
			for (int c=0; c<Q.ColCount(); c++)
			{
				double val = Q.GetElement(r,c);
				if (val != 0)
					QFile << String(r+1) << " " << String(c+1) << " " << String(val) << endl;
			}
		QFile.close();

		ofstream bFile("../Matlab/b.txt");
		PersistentAssert(!bFile.fail(), "Failed to open file");
		for (int r=0; r<b.Length(); r++)
			bFile << String(b[r]) << endl;
		bFile.close();
		Console::WriteLine("Done dumping matrix files");
		Console::WriteLine("Waiting for Matlab...Press num pad 0 when done");
		while(GetAsyncKeyState(VK_NUMPAD0) == 0)
		{
			Sleep(1000);
		}
	}

	//Solve using EigenSolver
	Vector<double> x;
	if (!parameters.useMatlab)
	{
		EigenSolver solver;
		x = solver.Solve(Q, b, EigenSolver::ConjugateGradient_Diag);
	} else
	{
		Console::WriteLine("Reading solution file");
		//read the solution file produced by Matlab
		Vector<String> xLines = Utility::GetFileLines("../Matlab/x.txt");
		for (String line:xLines)
			x.PushEnd(line.ConvertToDouble());
	}

    for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        Layer &curLayer = layers.layers[layerIndex];
        curLayer.superpixelWeights.Allocate(superpixelCount);
        for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
        {
            curLayer.superpixelWeights[superpixelIndex] = x[superpixelIndex + layerIndex * superpixelCount];
        }
    }

	if(Constants::dumpLayerImages)
	{
		VisualizeLayers(parameters, bmp, layers);
		VisualizeReconstruction(parameters, bmp, layers);
		VisualizeLayerGrid(parameters, bmp, layers);
	}

	//output the sum of each superpixel if not to one
	for (UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		double total = 0;
		for (UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
		{
			Layer &curLayer = layers.layers[layerIndex];
			total += curLayer.superpixelWeights[superpixelIndex];
		}
		if (total > 1.1 || total < 0.9)
			Console::WriteLine(String(total) + " " + String(superpixelIndex));
	}
}

Bitmap LayerExtractor::RecolorSuperpixels(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const
{
	Bitmap result = bmp;

	for(UINT y = 0; y < bmp.Height(); y++)
	{
		for(UINT x = 0; x < bmp.Width(); x++)
		{
			const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
			const UINT k = curPixel.indices.Length();

			Vec3f sum = Vec3f::Origin;
			for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
			{
				sum += Vec3f(newSuperpixelColors[curPixel.indices[neighborIndex]]) * (float)curPixel.weights[neighborIndex];
			}
			result[y][x] = RGBColor(sum);
		}
	}
	return result;
}

Bitmap LayerExtractor::RecolorLayers(const Bitmap &bmp, const LayerSet &layers, const Vector<RGBColor> &newLayerColors) const
{
	const UINT superpixelCount = superpixelColors.Length();
	Vector<Vec3f> newColors(superpixelCount, Vec3f::Origin);
	for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
		{
			newColors[superpixelIndex] += (float)layers.layers[layerIndex].superpixelWeights[superpixelIndex] * Vec3f(newLayerColors[layerIndex]);
		}
	}

	return RecolorSuperpixels(bmp, newColors);
}

Vector<PixelLayer> LayerExtractor::GetPixelLayers(const Bitmap &bmp, const LayerSet &layers) const
{
	//for each pixel, compute it's layer membership
	Vector<PixelLayer> result;
	for (UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		PixelLayer layer;
		layer.color = layers.layers[layerIndex].color;
		layer.pixelWeights = Grid<double>(bmp.Height(), bmp.Width(), 0);
		result.PushEnd(layer);
	}
	for (UINT y=0; y<bmp.Height(); y++)
	{
		for (UINT x=0; x<bmp.Width(); x++)
		{
			const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
			const UINT k = curPixel.indices.Length();

			for (UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
			{
				for (UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
				{
					UINT superpixelIndex = curPixel.indices[neighborIndex];
					double superpixelWeight = curPixel.weights[neighborIndex];
					result[layerIndex].pixelWeights(y,x) += layers.layers[layerIndex].superpixelWeights[superpixelIndex]*superpixelWeight;
				}
			}

		}
	}
	return result;
}

void LayerExtractor::ComputeSuperpixels(const AppParameters &parameters, const Bitmap &bmp)
{
	ComponentTimer timer("Computing superpixels");

	//SuperpixelExtractorPeriodic extractor;
	SuperpixelExtractorSuperpixel extractor;
	superpixelColors = extractor.Extract(parameters, bmp);
}

void LayerExtractor::ComputeWeightMatrix(const AppParameters &parameters)
{
	ComponentTimer timer("Computing weight matrix");
	const UINT n = superpixelNeighbors.Length();
	SparseMatrix<double> W(n, n);

	for(UINT superpixelIndex = 0; superpixelIndex < n; superpixelIndex++)
	{
		W.PushDiagonalElement(superpixelIndex, 1.0);
		const SuperpixelNeighborhood &neighborhood = superpixelNeighbors[superpixelIndex];
		for(UINT neighborIndex = 0; neighborIndex < neighborhood.indices.Length(); neighborIndex++)
		{
			W.PushElement(superpixelIndex, neighborhood.indices[neighborIndex], -neighborhood.embeddingWeights[neighborIndex]);
		}
	}

	Console::WriteLine("Building W-matrix");
	WBase = W.Transpose() * W;
}

void LayerExtractor::ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp)
{
	ComponentTimer timer("Computing nearest neighbors");

	KDTree tree;

	Vector<const float*> points;
	for(const ColorCoordinate &superpixel : superpixelColors)
	{
		points.PushEnd(superpixel.features);
	}

	tree.BuildTree(points, 5, Math::Max(parameters.pixelNeighborCount, parameters.superpixelNeighborCount) + 1);

	pixelNeighborhoods.Allocate(bmp.Height(), bmp.Width());

	for(UINT y = 0; y < bmp.Height(); y++)
	{
		for(UINT x = 0; x < bmp.Width(); x++)
		{
			ColorCoordinate curCoord(parameters, bmp[y][x], Vec2i(x, y), bmp.Width(), bmp.Height());
			tree.KNearest(curCoord.features, parameters.pixelNeighborCount, pixelNeighborhoods(y, x).indices, 0.0f);
		}
	}

	superpixelNeighbors.Allocate(superpixelColors.Length());

	for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
	{
		SuperpixelNeighborhood &b = superpixelNeighbors[superpixelIndex];
		tree.KNearest(superpixelColors[superpixelIndex].features, parameters.superpixelNeighborCount + 1, b.indices, 0.0f);
		if(b.indices.Contains(superpixelIndex))
		{
			b.indices.RemoveSwap(b.indices.FindFirstIndex(superpixelIndex));
		}
		else
		{
			b.indices.PopEnd();
		}
	}
	Console::WriteLine("Done");
}


void LayerExtractor::TestLayerRecoloring(const Bitmap &bmp, const LayerSet &layers) const
{
	Vector<RGBColor> targetColors;
	targetColors.PushEnd(RGBColor::Black);
	targetColors.PushEnd(RGBColor::White);
	targetColors.PushEnd(RGBColor::Red);
	targetColors.PushEnd(RGBColor::Green);
	targetColors.PushEnd(RGBColor::Blue);

	const UINT layerCount = layers.layers.Length(); 

	Vector<RGBColor> baseLayerColors(layerCount);
	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++) baseLayerColors[layerIndex] = RGBColor(layers.layers[layerIndex].color);

	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		const Layer &l = layers.layers[layerIndex];
		for(UINT targetColorIndex = 0; targetColorIndex < targetColors.Length(); targetColorIndex++)
		{
			Vector<RGBColor> newLayerColors = baseLayerColors;
			newLayerColors[layerIndex] = RGBColor::Interpolate(RGBColor(layers.layers[layerIndex].color), targetColors[targetColorIndex], 0.6f);

			RecolorLayers(bmp, layers, newLayerColors).SavePNG("../Results/Recoloring/layer" + String(layerIndex) + "_t" + String(targetColorIndex) + ".png");
		}
	}
}

void LayerExtractor::TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const
{
	ComponentTimer timer("Testing neighborhood weights");

	for(UINT y = 0; y < bmp.Height(); y++)
	{
		for(UINT x = 0; x < bmp.Width(); x++)
		{
			const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
			const UINT k = curPixel.indices.Length();

			Vec3f sum = Vec3f::Origin;
			for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
			{
				sum += Vec3f(superpixelColors[curPixel.indices[neighborIndex]].color) * (float)curPixel.weights[neighborIndex];
			}

			float error = Vec3f::Dist(Vec3f(bmp[y][x]), sum);
			Console::WriteLine("e=" + String(error));
		}
	}
}

Vector<double> LayerExtractor::ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures)
{
	//
	// Remember that for forming linear combinations, we consider only the color terms (3 dimensions) and not the spatial ones!
	//

	const UINT k = indices.Length();
	DenseMatrix<double> z(k, 3);
	for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
	{
		const ColorCoordinate &neighbor = superpixelColors[indices[neighborIndex]];
		for(UINT dimensionIndex = 0; dimensionIndex < 3; dimensionIndex++)
		{
			z(neighborIndex, dimensionIndex) = neighbor.features[dimensionIndex] - pixelFeatures[dimensionIndex];
		}
	}

	DenseMatrix<double> G = z * z.Transpose();

	//
	// Add weight regularization term
	//
	for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
	{
		G(neighborIndex, neighborIndex) += parameters.weightRegularizationTerm;
	}

	G.InvertInPlace();

	Vector<double> columnVector(k, 1.0);
	Vector<double> result;

	DenseMatrix<double>::Multiply(result, G, columnVector);

	result.Scale(1.0 / result.Sum());

	return result;
}

void LayerExtractor::ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp)
{
	ComponentTimer timer("Computing neighborhood weights");

	Console::WriteLine("Computing pixel weights");
	for(UINT y = 0; y < bmp.Height(); y++)
	{
		for(UINT x = 0; x < bmp.Width(); x++)
		{
			PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
			const UINT k = curPixel.indices.Length();

			ColorCoordinate curPixelCoordinate(parameters, bmp[y][x], Vec2i(x, y), bmp.Width(), bmp.Height());
			curPixel.weights = ComputeWeights(parameters, curPixel.indices, curPixelCoordinate.features);
		}
	}

	Console::WriteLine("Computing superpixel weights");
	for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
	{
		SuperpixelNeighborhood &n = superpixelNeighbors[superpixelIndex];

		n.embeddingWeights = ComputeWeights(parameters, superpixelNeighbors[superpixelIndex].indices, superpixelColors[superpixelIndex].features);

		n.similarityWeights.Allocate(n.indices.Length());
		for(UINT neighborIndex = 0; neighborIndex < n.similarityWeights.Length(); neighborIndex++)
		{
			n.similarityWeights[neighborIndex] = Vec3f::Dist(Vec3f(superpixelColors[superpixelIndex].color), Vec3f(superpixelColors[n.indices[neighborIndex]].color));
		}
	}
}

void LayerExtractor::VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const
{
	Bitmap result(bmp.Width(), bmp.Height());
	result.Clear(RGBColor::Black);

	for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
	{
		const ColorCoordinate &superpixel = superpixelColors[superpixelIndex];
		if(newSuperpixelColors == NULL)
			result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
		else
			result[superpixel.coord.y][superpixel.coord.x] = RGBColor((*newSuperpixelColors)[superpixelIndex]);
	}
	result.SavePNG("../Results/" + filename + ".png");
}

void LayerExtractor::VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const
{
	const UINT debugPixelCount = 5;
	for(UINT debugPixelIndex = 0; debugPixelIndex < debugPixelCount; debugPixelIndex++)
	{
		int x = rand() % (bmp.Width() - 2) + 1;
		int y = rand() % (bmp.Height() - 2) + 1;

		const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);

		Bitmap result(bmp.Width(), bmp.Height());
		result.Clear(RGBColor::Black);

		for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]) * 0.2f);

		result[y + 0][x + 0] = bmp[y][x];
		result[y + 1][x + 0] = bmp[y][x];
		result[y - 1][x + 0] = bmp[y][x];
		result[y + 0][x + 1] = bmp[y][x];
		result[y + 0][x - 1] = bmp[y][x];

		for(UINT neighborIndex : curPixel.indices)
		{
			const ColorCoordinate &superpixel = superpixelColors[neighborIndex];
			result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
		}

		result.SavePNG("../Results/pixel" + String(debugPixelIndex) + ".png");
	}

	for (UINT debugSuperpixelIndex=0; debugSuperpixelIndex < debugPixelCount; debugSuperpixelIndex++)
	{
		int randIdx = rand() % superpixelColors.Length();
		const SuperpixelNeighborhood &curSPixel = superpixelNeighbors[randIdx];

		Bitmap result(bmp.Width(), bmp.Height());
		result.Clear(RGBColor::Black);

		for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]) * 0.2f);

		int x = superpixelColors[randIdx].coord.x;
		int y = superpixelColors[randIdx].coord.y;

		result[y + 0][x + 0] = bmp[y][x];
		result[y + 1][x + 0] = bmp[y][x];
		result[y - 1][x + 0] = bmp[y][x];
		result[y + 0][x + 1] = bmp[y][x];
		result[y + 0][x - 1] = bmp[y][x];

		for (UINT neighborIndex : curSPixel.indices)
		{
			const ColorCoordinate &superpixel = superpixelColors[neighborIndex];
			result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
		}
		result.SavePNG("../Results/superpixel"+String(debugSuperpixelIndex)+".png");

	}
}

void LayerExtractor::VisualizeLayerPalette(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers, int index, int addedIndex) const
{
	const UINT paletteHeight = 40;
	Bitmap result(bmp.Width(), bmp.Height() + paletteHeight);

	result.Clear(RGBColor::Black);

	for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x])*0);

	AliasRender render;


	int layerWidth = Math::Ceiling((double)bmp.Width() / (double)layers.layers.Length());
	for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		for(int x = layerIndex * layerWidth; x < (int)(layerIndex + 1) * layerWidth; x++)
		{
			for(int y = (int)bmp.Height(); y < (int)result.Height(); y++)
			{
				if(result.ValidCoordinates(x, y)) result[y][x] = RGBColor(layers.layers[layerIndex].color);
			}
		}
	}

	//draw out the negative weights
	Vector<double> sumNegWeights(superpixelColors.Length(), 0);
	for (UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
	{
		for (UINT layerIndex=0; layerIndex < layers.layers.Length(); layerIndex++)
		{
			if (layers.layers[layerIndex].superpixelWeights.Length() == 0)
				continue;
			double weight = layers.layers[layerIndex].superpixelWeights[superpixelIndex];
			if (weight < 0)
			{
				sumNegWeights[superpixelIndex] += fabs(weight);
			}
		}
		double weight = sumNegWeights[superpixelIndex];
		Vec2i coord = superpixelColors[superpixelIndex].coord;

		render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(coord, Vec2i(2,2)), RGBColor(weight*255,weight*255,weight*255),  result[coord.y][coord.x]);
	}

	if (addedIndex >= 0)
	{
		Vec2i coord = superpixelColors[addedIndex].coord;
		render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(coord, Vec2i(2, 2)), result[coord.y][coord.x], RGBColor::Green);
		Console::WriteLine("Added neg weight " + String(sumNegWeights[addedIndex]));
		Console::WriteLine("Most neg weight " + String(sumNegWeights.MaxValue()));
	}
	result.SavePNG("../Results/LayerPalette" + String(index) + ".png");
}

void LayerExtractor::VisualizeLayerConstraints(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const
{
	String key = "initial";

	const UINT paletteHeight = 40;
	Bitmap result(bmp.Width(), bmp.Height());

	result.Clear(RGBColor::Black);

	/*for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]) * 0.2f);

	AliasRender render;
	const Vector<SuperpixelLayerConstraint>& constraints = layers.constraints[key];
	for(const auto &constraint : constraints)
	{
		if(constraint.target > 0.0)
		{
			RGBColor color = RGBColor(layers.layers[constraint.layerIndex].color);
			RGBColor borderColor = color;
			if(Vec3f(color).Length() < 0.3) borderColor = RGBColor::White;

			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(superpixelColors[constraint.index].coord, Vec2i(2, 2)), color, borderColor);
		}
	}

	int layerWidth = Math::Ceiling((double)bmp.Width() / (double)layers.layers.Length());
	for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		for(int x = layerIndex * layerWidth; x < (int)(layerIndex + 1) * layerWidth; x++)
		{
			for(int y = (int)bmp.Height(); y < (int)result.Height(); y++)
			{
				if(result.ValidCoordinates(x, y)) result[y][x] = RGBColor(layers.layers[layerIndex].color);
			}
		}
	}

	result.SavePNG("../Results/LayerConstraints.png");*/

	for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]));

	AliasRender render;
	const Vector<SuperpixelLayerConstraint>& constraints = layers.constraints[key];
	for(const auto &constraint : constraints)
	{
		if(constraint.target > 0.0)
		{
			RGBColor color = RGBColor::Magenta;
			RGBColor borderColor = RGBColor::Magenta;

			render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(superpixelColors[constraint.index].coord, Vec2i(2, 2)), color, borderColor);
		}
	}

	result.SavePNG("../Results/LayerConstraints.png");
}

void LayerExtractor::VisualizeReconstruction(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const
{
	const UINT superpixelCount = superpixelColors.Length();
	Vector<Vec3f> newColors(superpixelCount, Vec3f::Origin);
	for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
	{
		for(const Layer &l : layers.layers)
		{
			newColors[superpixelIndex] += (float)l.superpixelWeights[superpixelIndex] * l.color;
		}
	}

	RecolorSuperpixels(bmp, newColors).SavePNG("../Results/Reconstruction_P" + String(pass) + ".png");
}

void LayerExtractor::VisualizeLayerGrid(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const
{
	const Vec2i gridSize(3, 2);
	const Vec2i buffer(45, 45);
	const UINT paletteSize = 40;
	const Vec2i gridStride(bmp.Width() + paletteSize + buffer.x, bmp.Height() + buffer.y);
	Bitmap result(gridStride.x * gridSize.x - buffer.x, gridStride.y * gridSize.y - buffer.y, RGBColor::White);

	Vec2i gridPos = Vec2i::Origin;
	for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		const Layer &curLayer = layers.layers[layerIndex];

		const UINT superpixelCount = superpixelColors.Length();
		Vector<Vec3f> newColors(superpixelCount);
		for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
		{
			newColors[superpixelIndex] = Vec3f(RGBColor::Interpolate(RGBColor::Black, RGBColor::White, (float)curLayer.superpixelWeights[superpixelIndex]));
		}

		Bitmap smoothBmp = RecolorSuperpixels(bmp, newColors);

		Vec2i startPos = Vec2i(gridStride.x * gridPos.x, gridStride.y * gridPos.y);
		for(UINT y = 0; y < bmp.Height(); y++)
		{
			for(UINT x = 0; x < paletteSize; x++)
			{
				if(result.ValidCoordinates(x + startPos.x, y + startPos.y)) result[y + startPos.y][x + startPos.x] = RGBColor(curLayer.color);
			}
		}
		smoothBmp.BltTo(result, startPos + Vec2i(paletteSize, 0));

		gridPos.x++;
		if(gridPos.x == gridSize.x)
		{
			gridPos.x = 0;
			gridPos.y++;
		}
	}
	result.SavePNG("../Results/LayerGrid.png");
}

void LayerExtractor::VisualizeLayers(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const
{
	AliasRender render;
	for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		const Layer &curLayer = layers.layers[layerIndex];

		const UINT superpixelCount = superpixelColors.Length();
		Vector<Vec3f> newColors(superpixelCount);
		for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
		{
			newColors[superpixelIndex] = Vec3f(RGBColor::Interpolate(RGBColor::Black, RGBColor::White, (float)curLayer.superpixelWeights[superpixelIndex]));
		}

		Bitmap smoothBmp = RecolorSuperpixels(bmp, newColors);
		Bitmap discreteBmp = smoothBmp;
		discreteBmp.Clear(RGBColor::Black);

		for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
		{
			float weight = (float)curLayer.superpixelWeights[superpixelIndex];
			Vec3f c = Vec3f(RGBColor::Interpolate(RGBColor::Black, RGBColor::White, weight));

			if(weight > 1.02) c = Vec3f(RGBColor::Red);
			if(weight < -0.02) c = Vec3f(RGBColor::Blue);
			render.DrawRect(discreteBmp, Rectangle2i::ConstructFromCenterVariance(superpixelColors[superpixelIndex].coord, Vec2i(2, 2)), RGBColor(c), RGBColor(c));//RGBColor(0, 128, 0));
		}

		for(UINT y = 0; y < smoothBmp.Height(); y++) for(UINT x = 0; x < 10; x++)
		{
			smoothBmp[y][x] = RGBColor(curLayer.color);
			discreteBmp[y][x] = RGBColor(curLayer.color);
		}
		discreteBmp.SavePNG("../Results/LayerD" + String(layerIndex) + "_P" + String(pass) + ".png");
		smoothBmp.SavePNG("../Results/LayerS" + String(layerIndex) + "_P" + String(pass) + ".png");
	}
}

Vector<PixelConstraint> LayerExtractor::ComputePalette(const String &filename, const Bitmap &bmp) const
{
	Vector<PixelConstraint> result;
	Bitmap paletteimage;
	paletteimage.LoadPNG(filename);

	for(UINT y = 0; y < paletteimage.Height(); y++) {
		for(UINT x = 0; x < paletteimage.Width(); x++)
		{
			if(paletteimage[y][x] != bmp[y][x])
			{
				result.PushEnd(PixelConstraint(Vec2i(x,y), Vec3f(bmp[y][x])));
			}
		}
	}

	Console::WriteLine("User-selected palette: " + String(result.Length()));
	return result;
}