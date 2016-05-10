/*
  ==============================================================================

    FloatStepperUI.cpp
    Created: 28 Apr 2016 6:00:28pm
    Author:  bkupe

  ==============================================================================
*/

#include "FloatStepperUI.h"

FloatStepperUI::FloatStepperUI(Parameter * _parameter) :
    ParameterUI(_parameter)

{
    slider = new Slider(Slider::SliderStyle::IncDecButtons, Slider::TextEntryBoxPosition::TextBoxLeft);
    addAndMakeVisible(slider);
    slider->setRange(parameter->minimumValue, parameter->maximumValue,1);
    slider->setValue(parameter->value);
    slider->addListener(this);
	slider->setTextBoxIsEditable(_parameter->isEditable);
}

FloatStepperUI::~FloatStepperUI()
{

}

void FloatStepperUI::resized()
{
    slider->setBounds(getLocalBounds());
}

void FloatStepperUI::valueChanged(const var & value)
{
    if ((float)value == slider->getValue()) return;

    slider->setValue(value,NotificationType::dontSendNotification);
}

void FloatStepperUI::sliderValueChanged(Slider * _slider)
{
	parameter->setValue(_slider->getValue());
}
