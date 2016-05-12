/*
  ==============================================================================

    FloatStepperUI.h
    Created: 28 Apr 2016 6:00:28pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef FLOATSTEPPERUI_H_INCLUDED
#define FLOATSTEPPERUI_H_INCLUDED

#include "ParameterUI.h"
#include "JuceHeader.h"

class FloatStepperUI : public ParameterUI, public Slider::Listener
{

public:
    FloatStepperUI(Parameter * _parameter);
    virtual ~FloatStepperUI();

    ScopedPointer<Slider> slider;

    void resized() override;
protected:
    void valueChanged(const var &) override;


    // Inherited via Listener
    virtual void sliderValueChanged(Slider * slider) override;
    void parameterRangeChanged(Parameter * p) override;
};


#endif  // FLOATSTEPPERUI_H_INCLUDED
