/*
  ==============================================================================

    ControllerUI.h
    Created: 8 Mar 2016 10:46:01pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLERUI_H_INCLUDED
#define CONTROLLERUI_H_INCLUDED


#include "Controller.h"//keep
#include "InspectableComponent.h"

#include "TriggerBlinkUI.h"
class StringParameterUI;


class ControllerUI : public InspectableComponent, public ButtonListener
{
public:
    ControllerUI(Controller * controller);
    virtual ~ControllerUI();

	ScopedPointer<StringParameterUI> nameTF;
	ScopedPointer<ParameterUI> enabledBT;
	ImageButton removeBT;

	ScopedPointer<TriggerBlinkUI> activityBlink;

    Controller * controller;

    virtual void paint(Graphics &g) override;
    virtual void resized() override;

	virtual void mouseDown(const MouseEvent &e) override;
	virtual void buttonClicked(Button *) override;
	bool keyPressed(const KeyPress &e) override;

	virtual InspectorEditor * createEditor() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllerUI)
};


#endif  // CONTROLLERUI_H_INCLUDED
