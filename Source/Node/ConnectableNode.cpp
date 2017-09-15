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


#include "ConnectableNode.h"

#include "../Audio/AudioHelpers.h"
#include "NodeContainer/NodeContainer.h"


bool isEngineLoadingFile();


ConnectableNode::ConnectableNode(const String & name, bool _hasMainAudioControl):
parentNodeContainer(nullptr),
hasMainAudioControl(_hasMainAudioControl),
canBeRemovedByUser(true),
ParameterContainer(name),
userCanAccessInputs(true),
userCanAccessOutputs(true),
audioNode(nullptr),
outputVolume(nullptr)
{
  //set Params
  ParameterContainer::nameParam->isEditable=true;
  descriptionParam = addNewParameter<StringParameter>("Description", "Set a custom description for this node.", "Description");
  enabledParam = addNewParameter<BoolParameter>("Enabled", "Enable processing of the node.\nDisable will bypass the node and pass the audio flux to the output", true);
  
  nodePosition = addNewParameter<Point2DParameter<int>>("nodePosition", "position on canvas",0,0,Array<var>{0,0});

  nodeSize = addNewParameter<Point2DParameter<int>>("nodeSize", "Node Size", 180,100,Array<var>{0,0});

  miniMode = addNewParameter<BoolParameter>("miniMode", "Mini Mode", false);
  
  nodePosition->isControllableExposed = false;
  nodeSize->isControllableExposed = false;

  nodePosition->isPresettable = false;
  nodeSize->isPresettable = false;

  nodePosition->hideInEditor = true;
	 nodeSize->hideInEditor = true;

  descriptionParam->isPresettable = false;
  enabledParam->isPresettable = false;


  //Audio
  if(hasMainAudioControl){
		  outputVolume = addNewParameter<FloatParameter>("masterVolume", "master volume for this node", DB0_FOR_01);
    //  bypass = addNewParameter<BoolParameter>("Bypass", "by-pass current node, letting audio pass thru", false);
  }

  /*
	  setInputChannelName(0, "Main Left");
	  setInputChannelName(1, "Main Right");
	  setOutputChannelName(0, "Main Left");
	  setOutputChannelName(1, "Main Right");
	  */

  //allow for all nested container to have all parameters save in node's preset (except node container, see nodecontainer's constructor)
  presetSavingIsRecursive = true;
}

ConnectableNode::~ConnectableNode()
{
  //@Martin :: must do this here (doubling with the one ControllableContainer::clear) to get right preset filter, because getPresetFilter is overriden and when calling getPresetFilter() from ControllableContainer::clear, it doesn't return the overriden method..)
	cleanUpPresets();

  masterReference.clear();
  parentNodeContainer = nullptr;
}


void ConnectableNode::setParentNodeContainer(NodeContainer * _parentNodeContainer)
{

  addToAudioGraph(_parentNodeContainer->getAudioGraph());
  parentNodeContainer = _parentNodeContainer;


}

AudioProcessorGraph::Node * ConnectableNode::getAudioNode(bool)
{
  jassert(audioNode->getProcessor() == getAudioProcessor());
  return audioNode;
}




bool ConnectableNode::hasAudioInputs()
{
  //to override
  if (getAudioNode(true) == nullptr) return false;
  return getAudioNode(true)->getProcessor()->getTotalNumInputChannels() > 0;
}

bool ConnectableNode::hasAudioOutputs()
{
  //to override
  if (getAudioNode(false) == nullptr) return false;
  return getAudioNode(false)->getProcessor()->getTotalNumOutputChannels() > 0;
}

bool ConnectableNode::hasDataInputs()
{
  //to override
  return false;
}

bool ConnectableNode::hasDataOutputs()
{
  //to override
  return false;
}




void ConnectableNode::onContainerParameterChanged(Parameter * p)
{
  nodeListeners.call(&ConnectableNodeListener::nodeParameterChanged, this, p);
}


void ConnectableNode::remove()
{
  nodeListeners.call(&ConnectableNode::ConnectableNodeListener::askForRemoveNode, this);
}

void ConnectableNode::clear()
{
  //  ControllableContainer::clear();
  //to override
}







/////////////////////////////  AUDIO


void ConnectableNode::addToAudioGraph(AudioProcessorGraph* g)
{
  audioNode = g->addNode(getAudioProcessor());
  getAudioProcessor()->setRateAndBufferSizeDetails (g->getSampleRate(), g->getBlockSize());

}

void ConnectableNode::removeFromAudioGraph()
{
  const ScopedLock lk(parentNodeContainer->getAudioGraph()->getCallbackLock());
  parentNodeContainer->getAudioGraph()->removeNode(getAudioNode());
  parentNodeContainer->updateAudioGraph(false);

}



void ConnectableNode::setInputChannelNames(int startChannel, StringArray names)
{
  for (int i = startChannel; i < startChannel + names.size(); i++)
  {
    setInputChannelName(i, names[i]);
  }
}

void ConnectableNode::setOutputChannelNames(int startChannel, StringArray names)
{
  for (int i = startChannel; i < startChannel + names.size(); i++)
  {
    setOutputChannelName(i, names[i]);
  }
}

void ConnectableNode::setInputChannelName(int channelIndex, const String & name)
{
  while (inputChannelNames.size() < (channelIndex + 1))
  {
    inputChannelNames.add(String::empty);
  }

  inputChannelNames.set(channelIndex, name);
}

void ConnectableNode::setOutputChannelName(int channelIndex, const String & name)
{
  while (outputChannelNames.size() < (channelIndex + 1))
  {
    outputChannelNames.add(String::empty);
  }

  outputChannelNames.set(channelIndex, name);
}

String ConnectableNode::getInputChannelName(int channelIndex)
{
  String defaultName = "Input " + String(channelIndex+1);
  if (channelIndex < 0 || channelIndex >= inputChannelNames.size()) return defaultName;

  String s = inputChannelNames[channelIndex];
  if (s.isNotEmpty()) return s;
  return defaultName;
}

String ConnectableNode::getOutputChannelName(int channelIndex)
{
  String defaultName = "Output " + String(channelIndex+1);
  if (channelIndex < 0 || channelIndex >= outputChannelNames.size()) return defaultName;

  String s = outputChannelNames[channelIndex];
  if (s.isNotEmpty()) return s;
  return defaultName;
}

AudioProcessor * ConnectableNode::getAudioProcessor() {
  if(audioNode){
    jassert(audioNode->getProcessor() == dynamic_cast<NodeBase * >(this));
    return audioNode->getProcessor();
  }
  return dynamic_cast<NodeBase * >(this);
};

/////////////////////////////  DATA

Data * ConnectableNode::getInputData(int)
{
  //to override
  return nullptr;
}

Data * ConnectableNode::getOutputData(int)
{
  //to override
  return nullptr;
}

int ConnectableNode::getTotalNumInputData()
{
  //to override
  return 0;
}

int ConnectableNode::getTotalNumOutputData()
{
  //to override
  return 0;
}

StringArray ConnectableNode::getInputDataInfos()
{
  return StringArray();
}

StringArray ConnectableNode::getOutputDataInfos()
{
  return StringArray();
}

Data::DataType ConnectableNode::getInputDataType(const String & , const String & )
{
  //to override
  return Data::DataType::Unknown;
}

Data::DataType ConnectableNode::getOutputDataType(const String & , const String & )
{
  //to override
  return Data::DataType::Unknown;
}

Data * ConnectableNode::getOutputDataByName(const String & )
{
  //to override
  return nullptr;
}

Data * ConnectableNode::getInputDataByName(const String & )
{
  //to override
  return nullptr;
}
