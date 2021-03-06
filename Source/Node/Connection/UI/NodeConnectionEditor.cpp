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


#include "NodeConnectionEditor.h"

#include "../../../UI/Style.h"

#include "../../../Utils/DebugHelpers.h"
#include "NodeConnectionUI.h"

//==============================================================================
NodeConnectionEditor::NodeConnectionEditor (NodeConnectionUI* nodeConnectionUI) :
    InspectorEditor(),
    currentConnection (nullptr),
    editingLink (nullptr),
    selectedLink (nullptr)
{
    addAndMakeVisible (&outputsContainer);
    addAndMakeVisible (&inputsContainer);
    addAndMakeVisible (&linksContainer);

    setCurrentConnection (nodeConnectionUI->connection);
}

NodeConnectionEditor::~NodeConnectionEditor()
{
    if (currentConnection)currentConnection->removeConnectionListener (this);

    setCurrentConnection (nullptr);
}

void NodeConnectionEditor::setSelectedLink (NodeConnectionEditorLink* link)
{
    if (selectedLink != nullptr)
    {
        selectedLink->setSelected (false);
    }

    selectedLink = link;

    if (selectedLink != nullptr)
    {
        selectedLink->setSelected (true);
    }
}

void NodeConnectionEditor::setCurrentConnection (NodeConnection* _connection)
{
    if (currentConnection == _connection) return;

    selectedLink = nullptr;

    if (currentConnection != nullptr)
    {
        currentConnection->removeConnectionListener (this);
        clearContent();
    }

    currentConnection = _connection;

    if (currentConnection != nullptr)
    {
        currentConnection->addConnectionListener (this);

        if (currentConnection->isAudio()) generateContentForAudio();
        
    }


}

void NodeConnectionEditor::resized()
{
    InspectorEditor::resized();


    int panelWidth = (int) (getWidth() / 3.f);

    Rectangle<int> r = getLocalBounds();
    outputsContainer.setBounds (r.removeFromLeft (panelWidth));
    inputsContainer.setBounds (r.removeFromRight (panelWidth));
    linksContainer.setBounds (r);

    r = outputsContainer.getLocalBounds().withTrimmedTop (5).withTrimmedLeft (5).withTrimmedBottom (5);

    for (int i = 0; i < outputsContainer.getNumChildComponents(); i++)
    {
        Component* c = outputsContainer.getChildComponent (i);
        c->setBounds (r.removeFromTop (30));
        r.removeFromTop (5);
    }

    r = inputsContainer.getLocalBounds().withTrimmedTop (5).withTrimmedRight (5).withTrimmedBottom (5);

    for (int i = 0; i < inputsContainer.getNumChildComponents(); i++)
    {
        Component* c = inputsContainer.getChildComponent (i);
        c->setBounds (r.removeFromTop (30));
        r.removeFromTop (5);
    }

    r = linksContainer.getLocalBounds();

    for (int i = 0; i < linksContainer.getNumChildComponents(); i++)
    {
        Component* c = linksContainer.getChildComponent (i);
        c->setBounds (r);
    }
}

int NodeConnectionEditor::getContentHeight() const
{
    return  20 + jmax (outputSlots.size() * 35, inputSlots.size() * 35);
}

void NodeConnectionEditor::mouseEnter (const MouseEvent&)
{
    //DBG("Editor mouse enter " << e.eventComponent->getName());
}

void NodeConnectionEditor::mouseExit (const MouseEvent&)
{

}

void NodeConnectionEditor::clearContent()
{

    for (auto& s : inputSlots)
    {
        s->removeSlotListener (this);
    }

    for (auto& s : outputSlots)
    {
        s->removeSlotListener (this);
    }

    inputSlots.clear();
    outputSlots.clear();
    links.clear();

    inputsContainer.removeAllChildren();
    outputsContainer.removeAllChildren();
    linksContainer.removeAllChildren();
}

void NodeConnectionEditor::generateContentForAudio()
{
    clearContent();

    if (currentConnection == nullptr) return;

    if (currentConnection->sourceNode == nullptr || currentConnection->destNode == nullptr) return;

    //if (currentConnection->sourceNode->getAudioNode() == nullptr || currentConnection->destNode->getAudioNode()) return;

    int numOutputChannels = ((NodeBase*)currentConnection->sourceNode.get())->getTotalNumOutputChannels();
    int numInputChannels = ((NodeBase*)currentConnection->destNode.get())->getTotalNumInputChannels();

    for (int i = 0; i < numOutputChannels; i++)
    {
        String oName = currentConnection->sourceNode->ConnectableNode::getOutputChannelName (i);
        NodeConnectionEditorDataSlot* s = new NodeConnectionEditorDataSlot (oName, i, currentConnection->connectionType, NodeConnectionEditorDataSlot::IOType::OUTPUT);
        outputSlots.add (s);
        s->addSlotListener (this);
        s->setName ("output" + String (i + 1));
        outputsContainer.addAndMakeVisible (s);
        //DBG("Add output slot");
    }

    for (int i = 0; i < numInputChannels; i++)
    {
        String iName = currentConnection->destNode->ConnectableNode::getInputChannelName (i);
        NodeConnectionEditorDataSlot* s = new NodeConnectionEditorDataSlot (iName, i, currentConnection->connectionType, NodeConnectionEditorDataSlot::IOType::INPUT);
        inputSlots.add (s);
        s->addSlotListener (this);
        s->setName ("input" + String (i + 1));
        inputsContainer.addAndMakeVisible (s);
        //DBG("Add input Slot");
    }

    //@Martin : How to define how many channels a node has (input and output), it seems that it's defaulted to 2,
    //but i can't find how to set it up anywhere. is it dynamically computed when sending or receiving the AudioBuffer in processBlock ?
    // should be fixed !!
    for (auto& pair : currentConnection->model.audioConnections)
    {
        addAudioLink (pair.first, pair.second);
    }


    resized();
}


void NodeConnectionEditor::addAudioLink (int sourceChannel, int destChannel)
{
    if (sourceChannel > outputSlots.size())
    {
        LOGW(juce::translate("Channel not found in output Slots"));
        return;
    }

    if (destChannel > inputSlots.size())
    {
        LOGW(juce::translate("Channel not found  in input Slots"));
        return;
    }


    NodeConnectionEditorDataSlot* os = outputSlots[sourceChannel];
    NodeConnectionEditorDataSlot* is = inputSlots[destChannel];
    NodeConnectionEditorLink* l = new NodeConnectionEditorLink (os, is);
    is->addConnectedSlot (os);
    os->addConnectedSlot (is);
    links.add (l);
    l->addLinkListener (this);
    linksContainer.addAndMakeVisible (l);
}

void NodeConnectionEditor::removeAudioLinkForChannels (int sourceChannel, int destChannel)
{
    //DBG("Remove audio Link for channels");
    NodeConnectionEditorLink* l = getLinkForChannels (sourceChannel, destChannel);

    if (l == nullptr) return;

    if (l == selectedLink) selectedLink = nullptr;

    l->outSlot->removeConnectedSlot (l->inSlot);
    l->inSlot->removeConnectedSlot (l->outSlot);

    linksContainer.removeChildComponent (l);
    l->removeLinkListener (this);
    links.removeObject (l);
}


NodeConnectionEditorLink* NodeConnectionEditor::getLinkForSlots (NodeConnectionEditorDataSlot* outSlot, NodeConnectionEditorDataSlot* inSlot)
{
    for (auto& l : links)
    {
        //DBG(String("check link ") << l->outSlot->getName() << String(" <> ") << l->inSlot->getName() << "// " << outSlot->getName() << " <> " << inSlot->getName());
        if (l->outSlot == outSlot && l->inSlot == inSlot) return l;
    }

    return nullptr;
}

NodeConnectionEditorLink* NodeConnectionEditor::getLinkForChannels (int sourceChannel, int destChannel)
{
    for (auto& l : links)
    {
        if (l->outSlot->channel == sourceChannel && l->inSlot->channel == destChannel) return l;
    }

    return nullptr;
}

void NodeConnectionEditor::createEditingLink (NodeConnectionEditorDataSlot* baseSlot)
{
    //DBG("Create Link from connector");

    if (editingLink != nullptr)
    {
        //DBG("Already editing a connection !");
        return;
    }

    if (baseSlot->ioType == NodeConnectionEditorDataSlot::IOType::OUTPUT)
    {
        editingLink = new NodeConnectionEditorLink (baseSlot, nullptr);
    }
    else
    {
        editingLink = new NodeConnectionEditorLink (nullptr, baseSlot);
    }

    addAndMakeVisible (editingLink);
}

void NodeConnectionEditor::updateEditingLink()
{
    if (editingLink == nullptr) return;

    Point<int> cPos = getLocalPoint (editingLink->getBaseSlot(), editingLink->getBaseSlot()->getLocalBounds().getCentre());
    Point<int> mPos = getMouseXYRelative();
    int minX = jmin<int> (cPos.x, mPos.x);
    int minY = jmin<int> (cPos.y, mPos.y);
    int tw = abs (cPos.x - mPos.x);
    int th = abs (cPos.y - mPos.y);
    int margin = 50;

    checkDropCandidates();

    editingLink->setBounds (minX - margin, minY - margin, tw + margin * 2, th + margin * 2);
}


bool NodeConnectionEditor::checkDropCandidates()
{
    NodeConnectionEditorDataSlot* baseSlot = editingLink->getBaseSlot();
    bool targetIsInput = baseSlot->ioType == NodeConnectionEditorDataSlot::IOType::OUTPUT;
    OwnedArray<NodeConnectionEditorDataSlot>* targetArray = targetIsInput ? &inputSlots : &outputSlots;



    for (auto& slot : *targetArray)
    {
        if (slot->isAudio() )
        {


                float dist = (float) (slot->getMouseXYRelative().getDistanceFrom (slot->getLocalBounds().getRelativePoint (targetIsInput ? 0 : 1.f, .5f)));

                if (dist < 20)
                {
                    return setCandidateDropSlot (slot);
                }
            
        }
    }

    cancelCandidateDropSlot();
    return false;
}

void NodeConnectionEditor::finishEditingLink()
{
    bool success = editingLink->finishEditing();

    //DBG("Finish Editing, sucess ? " + String(success));



    if (success)
    {

        if (editingLink->inSlot->isConnected())
        {
                NodeConnectionEditorLink* l = getLinkForSlots ( editingLink->inSlot->getFirstConnectedSlot(), editingLink->inSlot);
                if (l != nullptr) l->remove();
        }
        if (currentConnection->isAudio())
        {
            currentConnection->addAudioGraphConnection (editingLink->outSlot->channel, editingLink->inSlot->channel);
        }
        else
        {
            //currentConnection->addDataGraphConnection (editingLink->outSlot->data, editingLink->inSlot->data);
        }

    }

    removeChildComponent (editingLink);
    editingLink = nullptr;
}

bool NodeConnectionEditor::setCandidateDropSlot (NodeConnectionEditorDataSlot* slot)
{
    if (editingLink == nullptr) return false;

    bool result = editingLink->setCandidateDropSlot (slot);
    //editingLink->candidateDropSlot->addMouseListener(this, false);
    return result;
}

void NodeConnectionEditor::cancelCandidateDropSlot()
{
    if (editingLink == nullptr) return;

    //if (editingLink->candidateDropSlot != nullptr) editingLink->candidateDropSlot->removeMouseListener(this);
    editingLink->cancelCandidateDropSlot();


}


void NodeConnectionEditor::slotMouseEnter (NodeConnectionEditorDataSlot* target)
{

}


void NodeConnectionEditor::slotMouseUp (NodeConnectionEditorDataSlot*)
{
    finishEditingLink();
}

void NodeConnectionEditor::askForRemoveLink (NodeConnectionEditorLink* target)
{
    //DBG("Ask for remove link");
    if (currentConnection->isAudio())
    {
        currentConnection->removeAudioGraphConnection (target->outSlot->channel, target->inSlot->channel);
    }


    if (target == selectedLink) selectedLink = nullptr;
}

void NodeConnectionEditor::selectLink (NodeConnectionEditorLink* target)
{
    setSelectedLink (target);
}


void NodeConnectionEditor::connectionAudioLinkAdded (const NodeConnection::AudioConnection& audioConnection)
{
    //DBG("Audio link added " << audioConnection.first << " > " << audioConnection.second);
    addAudioLink (audioConnection.first, audioConnection.second);
    resized();
}

void NodeConnectionEditor::connectionAudioLinkRemoved (const NodeConnection::AudioConnection& audioConnection)
{
    //DBG("Audio link removed");
    removeAudioLinkForChannels (audioConnection.first, audioConnection.second);
}

void NodeConnectionEditor::slotMouseExit (NodeConnectionEditorDataSlot*)
{


}

void NodeConnectionEditor::slotMouseDown (NodeConnectionEditorDataSlot* target)
{
    createEditingLink (target);
}

void NodeConnectionEditor::slotMouseMove (NodeConnectionEditorDataSlot*)
{
    //updateEditingLink();
}

void NodeConnectionEditor::slotMouseDrag (NodeConnectionEditorDataSlot*)
{
    updateEditingLink();
}

#endif
