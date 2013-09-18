#include "Main.h"

/*

S?(5,5)  = RGB(100, 50, 25)
T1(5,5) -= 100 / 255
T2(5,5) -= 50 / 255
T3(5,5) -= 25 / 255

T1 T2
^  ^
|  |
v  v
R1 R2

(0, 1)
(0, 1)
(0.5, 0)
(0, 1)
(0, 1)
(0.5, 0)

Update(T1=0,T2=1) Update(Vector<double>(0,1))
Update(T1=0,T2=1)
Update(T1=0.5,T2=1)
Update(T1=0,T2=1)
Update(T1=0,T2=1)

T1,T2,T3
C1*T1 + C2*T2 + C3*T3 = T
T1 + T2 + T3 = 1

C1*T1 + C2*T2 + C3*T3 + CS*S = T'

solve for: T1',T2',T3'
T1' + T2' + T3' + S = 1
C1*T1' + C2*T2' + C3*T3' + CS*S = T'

T1 T2 T3 T4
^  ^  ^  ^
|  |  |  |
v  v  v  v
R1 R2 R3 R4

0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1
0.1 0.1 0.1 1

0 0 0 1
0 0 0 1
0.4 0.4 0.4 0
0 0 0 1
0 0 0 1
0.4 0.4 0.4 0

Proposal #1: Update S and T iteratively

UpdateS(1.0)
UpdateS(1.0)
UpdateT(0.5)
UpdateS(1.0)
UpdateS(1.0)
UpdateT(0.5)
...

Proposal #2: Update S and T simultaneously

UpdateST()
UpdateST()
UpdateST()
...

Proposal #3: Update T after the fact
SynthesizeS() -> UpdateS() UpdateS() UpdateS()
FixTheWeights(T1,T2,T3,S?)

All the target layers
face_0.png
face_1.png
face_2.png
face_3.png
face_4_hint.png -> this is the hint for the new layer

The relevant synthesis layers
face_1.png,reference_A.png
face_2.png,reference_B.png

The hint for the newly synthesized layer
synthesis_mask.png

*/

void LayerSynthesis::Init(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
    InitPCA(layers, generator);
    InitKDTree(layers, generator, reducedDimension);
}

void LayerSynthesis::InitPCA(const PixelLayerSet &layers, const NeighborhoodGenerator &generator)
{
    
}

void LayerSynthesis::InitKDTree(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{

}

PixelLayerSet LayerSynthesis::Synthesize(const AppParameters &parameters, const PixelLayerSet &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule)
{
	//original - the original (incomplete) set of layers
	//pixels - the region drawn by the user, where a new layer(s) should be synthesized (including color information).
	//returns a new set of layers (the original layers might be modified a bit to take into account the new layer(s))

    //...

    PixelLayerSet target = original;
    for(int row = 0; row < updateSchedule.Rows(); row++)
    {
        SynthesizeStepInPlace(parameters, target, pixels, updateSchedule.ExtractRow(row));
    }
    return target;
}

void LayerSynthesis::SynthesizeStepInPlace(const AppParameters &parameters, const PixelLayerSet &target, const Vector<Vec2i> &pixels, const Vector<double> &updateSchedule)
{
    //Math::Lerp(start, end, 1.0);
}
