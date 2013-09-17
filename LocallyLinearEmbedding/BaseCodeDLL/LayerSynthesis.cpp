#include "Main.h"
#include "LayerSynthesis.h"


LayerSynthesis::LayerSynthesis(void)
{
}


LayerSynthesis::~LayerSynthesis(void)
{
}

PixelLayerSet LayerSynthesis::Synthesize(const PixelLayerSet &original, const PixelLayer &mask)
{
	//original - the original (incomplete) set of layers
	//mask - the region drawn by the user, where a new layer(s) should be synthesized (including color information).
	//returns a new set of layers (the original layers might be modified a bit to take into account the new layer(s))



}
