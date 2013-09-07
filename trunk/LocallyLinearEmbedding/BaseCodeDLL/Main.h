/*
Main.h
Written by Matthew Fisher

Main.h is included by all source files and includes every header file in the correct order.
*/
#pragma once

//
// Config.h includes a series of #defines used to control compiling options
//
#include "Config.h"

//
// Engine.h includes everything that rarely changes between applications, such as vector/Matrix4 libraries,
// OpenGL/DirectX graphics devices, software rasterizers, etc.
//
#include "Engine.h"

#include "Constants.h"

#include "BaseCodeDLL.h"

#include "AppParameters.h"

#include "Eigen3_2/Sparse"
#include "EigenSolver.h"

#include "BasisColorExtractor.h"
#include "Recolorizer.h"
#include "LayerExtractor.h"
#include "Utility.h"

#include "App.h"