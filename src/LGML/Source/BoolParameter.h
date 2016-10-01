/*
  ==============================================================================

    BoolParameter.h
    Created: 8 Mar 2016 1:22:15pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef BOOLPARAMETER_H_INCLUDED
#define BOOLPARAMETER_H_INCLUDED

#include "Parameter.h"

class BoolToggleUI;

class BoolParameter : public Parameter
{
public:
    BoolParameter(const String &niceName, const String &description, const bool &initialValue, bool enabled = true);
    ~BoolParameter() {}

    //ui creation
    BoolToggleUI * createToggle(BoolParameter * target = nullptr);
    ControllableUI * createDefaultUI(Controllable * targetControllable = nullptr) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoolParameter)
};



#endif  // BOOLPARAMETER_H_INCLUDED
