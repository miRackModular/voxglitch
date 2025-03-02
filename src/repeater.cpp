//
// Voxglitch "repeater" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"

#include "Common/constants.h"
#include "Common/sample.hpp"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/SamplePlayer.hpp"
#include "Common/dsp/DeclickFilter.hpp"

#include "Repeater/defines.h"
#include "Repeater/Repeater.hpp"
#include "Repeater/Readout.hpp"
#include "Repeater/MenuItemLoadSample.hpp"
#include "Repeater/RepeaterWidget.hpp"

Model* modelRepeater = createModel<Repeater, RepeaterWidget>("repeater");
