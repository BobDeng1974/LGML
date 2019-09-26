/* Copyright © Organic Orchestra, 2017
*
* This file is part of LGML.  LGML is a software to manipulate sound in realtime
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation (version 3 of the License).
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#if !ENGINE_HEADLESS

#include "ControllerUI.h"
#include "ControllerEditor.h"
#include "ControllerManagerUI.h"
#include "../../UI/Style.h"
#include "../../Controllable/Parameter/UI/TriggerBlinkUI.h"
#include "../../Controllable/Parameter/UI/StringParameterUI.h"
#include "../../Controllable/Parameter/UI/ParameterUIFactory.h"
#include "../../UI/Outliner.h"


static DrawableButton* getArrowButton(const String& name){
    static bool inited(false);
    static DrawablePath arrow;
    static DrawablePath  downArrow;
    static DrawablePath hoverArrow;
    static DrawablePath hoverDownArrow;
    if(!inited){
        Path p;
        p.addTriangle(0, 0, 1, 0.5, 0, 1);
        Path dp;
        dp.addTriangle(0, 0, 1, 0, 0.5, 1);
        arrow.setPath(p);
        downArrow.setPath(dp);
        hoverArrow.setPath(p);
        hoverDownArrow .setPath(dp);
        Colour hoverColour = arrow.getFill().colour.contrasting(.8f);
        hoverArrow.setFill(hoverColour);
        hoverDownArrow.setFill(hoverColour);
        inited = true;
    }

    DrawableButton* res = new DrawableButton (name,DrawableButton::ButtonStyle::ImageStretched);

    res->setImages(&arrow,
                   &hoverArrow,// over,
                   &downArrow,// down,
                   nullptr,// disabled,
                   &downArrow,// normalOn,
                   &hoverDownArrow,// overOn,
                   &downArrow,// downOn,
                   nullptr// disabledOn
                   );
//    res->setOpaque(false);
//    res->setPaintingIsUnclipped(false);
    return res;

}
ControllerUI::ControllerUI (Controller* _controller) :
    InspectableComponent (_controller, "controller"),
    controller (_controller)
{
    LGMLUIUtils::markHasNewBackground(this,2);
    canInspectChildContainersBeyondRecursion = true;
    {
        MessageManagerLock ml;
        addMouseListener (this, true);
    }
    nameTF = std::make_unique< StringParameterUI> (controller->nameParam);
    nameTF->valueLabel.setEditable(false,true);
    nameTF->setBackGroundIsTransparent (true);
    addAndMakeVisible (nameTF.get());

    enabledBT = ParameterUIFactory::createDefaultUI (controller->enabledParam);
    addAndMakeVisible (enabledBT.get());

    Image removeImage = ImageCache::getFromMemory (BinaryData::removeBT_png, BinaryData::removeBT_pngSize);

    removeBT.setImages (false, true, true, removeImage,
                        0.7f, Colours::transparentBlack,
                        removeImage, 1.0f, Colours::transparentBlack,
                        removeImage, 1.0f, Colours::white.withAlpha (.7f),
                        0.5f);
    removeBT.addListener (this);
    addAndMakeVisible (removeBT);

    if(auto *c = controller->isConnected){
        isConnectedUI = ParameterUIFactory::createDefaultUI(( ParameterBase*)c);
        isConnectedUI->showLabel = false;
        addAndMakeVisible(isConnectedUI.get());
    }

    inActivityBlink = std::make_unique< TriggerBlinkUI> (controller->inActivityTrigger);
    outActivityBlink = std::make_unique< TriggerBlinkUI> (controller->outActivityTrigger);

    inActivityBlink->showLabel = false;
    addAndMakeVisible (inActivityBlink.get());
    outActivityBlink->showLabel = false;
    addAndMakeVisible (outActivityBlink.get());
    userParamsUI = std::make_unique< Outliner>("usr_"+controller->shortName,&controller->userContainer,false);
    
    addAndMakeVisible(userParamsUI.get());
    userParamsUI->treeView.getViewport()->getViewedComponent()->addComponentListener(this);
    userParamsUI->treeView.setOpenCloseButtonsVisible(false);
    showUserParams.reset(getArrowButton("showParams"));
    addAndMakeVisible(showUserParams.get());
    showUserParams->setTooltip(juce::translate("show this controller registered parameters"));
    showUserParams->setToggleState(false, dontSendNotification);
    showUserParams->addListener(this);
    showUserParams->setClickingTogglesState(true);
    setPaintingIsUnclipped(false);
}

ControllerUI::~ControllerUI()
{
    userParamsUI->treeView.getViewport()->getViewedComponent()->removeComponentListener(this);
}

void ControllerUI::paint (Graphics& g)
{
    LGMLUIUtils::fillBackground(this, g);
    g.setColour (LGMLUIUtils::getCurrentBackgroundColor(this));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 4);

}

constexpr int headerHeight = 20;
constexpr int maxUsrParamHeight = 200;

void ControllerUI::componentMovedOrResized (Component& component,
                              bool wasMoved,
                              bool wasResized) {
    if(&component==userParamsUI->treeView.getViewport()->getViewedComponent()){
        setSize(getWidth(),getTargetHeight());
    }
}
void ControllerUI::resized()
{
    Rectangle<int> area = getLocalBounds();
    Rectangle<int> r = area.removeFromTop(headerHeight);
    r.removeFromRight (15);
    removeBT.setBounds (r.removeFromRight (20));
    r.removeFromRight (2);
    outActivityBlink->setBounds (r.removeFromRight (r.getHeight()/2).reduced (2));
    inActivityBlink->setBounds (r.removeFromRight (r.getHeight()/2).reduced (2));
    if(isConnectedUI){
        isConnectedUI->setBounds(r.removeFromRight(r.getHeight()).reduced(2));
    }
    enabledBT->setBounds (r.removeFromLeft (r.getHeight()));
    showUserParams->setBounds(r.removeFromLeft (r.getHeight()).reduced (4));
    r.removeFromLeft (5);
    nameTF->setBounds (r);

    if(area.getHeight()){
        userParamsUI->setBounds(area);
    }

}

int ControllerUI::getTargetHeight(){
    return   headerHeight + (showUserParams->getToggleState()?jmin(maxUsrParamHeight,userParamsUI->getTargetHeight()):0);
}





void ControllerUI::buttonClicked (Button* b)
{
    if (b == &removeBT)
    {
        auto cmui = findParentComponentOfClass<ControllerManagerUI>();
        cmui->removeControllerUndoable(controller);
    }
    else if(b==showUserParams.get()){
        setSize(getWidth(),getTargetHeight());
        
    }
}

bool ControllerUI::keyPressed (const KeyPress& key)
{
    if (!isSelected) return false;

    if (key.getKeyCode() == KeyPress::deleteKey || key.getKeyCode() == KeyPress::backspaceKey)
    {
        auto cmui = findParentComponentOfClass<ControllerManagerUI>();
        cmui->removeControllerUndoable(controller);
        return true;
    }

    return InspectableComponent::keyPressed(key);
}


//TODO add factory to handle custom Editor
std::unique_ptr<InspectorEditor> ControllerUI::createEditor()
{
    return std::make_unique< GenericParameterContainerEditor>(controller);
}

#endif
