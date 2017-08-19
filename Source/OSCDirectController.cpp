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


#include "OSCDirectController.h"
#include "NodeManager.h"
#include "DebugHelpers.h"
#include "TimeManager.h"
#include "ParameterProxy.h"
#include "ControlManager.h"

OSCDirectController::OSCDirectController() :
OSCDirectController("OSC Direct Controller")
{

}

OSCDirectController::OSCDirectController(const String & name) :
OSCController(name)
{
  NodeManager::getInstance()->addControllableContainerListener(this);
  userContainer.addControllableContainerListener(this);
  sendTimeInfo = addNewParameter<BoolParameter>("sendTimeInfo", "send time information", false);
  if(sendTimeInfo->boolValue()){
    TimeManager::getInstance()->addControllableContainerListener(this);
  }

}

OSCDirectController::~OSCDirectController()
{
  if(NodeManager * nm = NodeManager::getInstanceWithoutCreating())nm->removeControllableContainerListener(this);
  if(TimeManager * tm = TimeManager::getInstanceWithoutCreating()){tm->removeControllableContainerListener(this);}

}

Result OSCDirectController::processMessageInternal(const OSCMessage & msg)
{
  Result result = Result::ok();
  String addr = msg.getAddressPattern().toString();
  auto addrArray = OSCAddressToArray(addr);


  if(auto *up = (Parameter*)userContainer.getControllableForAddress(addrArray)){
        if(!setParameterFromMessage(up,msg)){
          result=  Result::fail("Controllable type not handled in user Parameter");
        }
  }

  String controller = addrArray[0];


  if (controller == "node" || controller=="time" || controller=="control")
  {
    addrArray.remove(0);
    Controllable * c = nullptr;
    if(controller=="node")c=NodeManager::getInstance()->getControllableForAddress(addrArray);
    else if (controller=="time")c=TimeManager::getInstance()->getControllableForAddress(addrArray);
    else if (controller=="control")c=ControllerManager::getInstance()->getControllableForAddress(addrArray);

    if (c != nullptr)
    {
      if (c->isControllableExposed && c->getParameter()->isEditable)
      {
        if(!setParameterFromMessage(c->getParameter(),msg)){
          result = Result::fail("Controllable type not handled");
        }
      }
    }
    else
    {
      result = Result::fail("Controllable not found");

      DBG("No Controllable for address : " + addr);
    }
  }
  else
  {
    result = Result::fail("address other than /node, not handler for now");
  }

  return result;
}




String getValidOSCAddress(const String &s){
  String targetName = s;
  if(!s.startsWithChar('/')){
    targetName = '/'+s;
  }
  targetName.replace(" ", "");

  return targetName;
}

void OSCDirectController::controllableAdded(ControllableContainer *,Controllable * ){}
void OSCDirectController::controllableRemoved(ControllableContainer *,Controllable *)
{

}
void OSCDirectController::onContainerParameterChanged(Parameter * p) {
  OSCController::onContainerParameterChanged(p);
  if(p==sendTimeInfo){
    if(sendTimeInfo->boolValue()){
      TimeManager::getInstance()->addControllableContainerListener(this);
    }
    else{
      TimeManager::getInstance()->removeControllableContainerListener(this);
    }

  }

  

};

void OSCDirectController::sendOSCForAddress(Controllable* c,const String & cAddress){
#if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES

  Controllable::Type targetType = c->type;
  if (targetType == Controllable::Type::PROXY) targetType = ((ParameterProxy *)c)->linkedParam->type;

  switch (targetType)
  {
    case Controllable::Type::TRIGGER:
      sendOSC(cAddress);
      break;

    case Controllable::Type::BOOL:
      sendOSC(cAddress,((Parameter *)c)->intValue());
      break;

    case Controllable::Type::FLOAT:
      sendOSC(cAddress, ((Parameter *)c)->floatValue());
      break;

    case Controllable::Type::INT:
      sendOSC(cAddress, ((Parameter *)c)->intValue());
      break;

    case Controllable::Type::STRING:
    case Controllable::Type::ENUM:
      sendOSC(cAddress, ((Parameter *)c)->stringValue());
      break;


    default:
      DBG("Type not supported " << targetType);
      jassertfalse;
      break;
  }


#else

  OSCMessage msg(cAddress);
  switch (c->type)
  {
    case Controllable::Type::TRIGGER:
      msg.addInt32(1);
      break;

    case Controllable::Type::BOOL:
      msg.addInt32(((Parameter *)c)->intValue());
      break;

    case Controllable::Type::FLOAT:
      msg.addFloat32(((Parameter *)c)->floatValue());
      break;

    case Controllable::Type::INT:
      msg.addInt32(((Parameter *)c)->intValue());
      break;

    case Controllable::Type::STRING:
      msg.addString(((Parameter *)c)->stringValue());
      break;

    default:
      DBG("OSC : unknown Controllable");
      jassertfalse;
      break;
  }

  sendOSC(msg);

#endif
}
void OSCDirectController::controllableFeedbackUpdate(ControllableContainer * /*originContainer*/,Controllable * c)
{

  if(enabledParam->boolValue()){
    if(c->isChildOf(&userContainer)){
      sendOSCForAddress(c,c->getControlAddress(&userContainer));
    }
    else{
    sendOSCForAddress(c,c->controlAddress);
    }
  }


}

void OSCDirectController::controllableContainerAdded(ControllableContainer *,ControllableContainer *)
{
}

void OSCDirectController::controllableContainerRemoved(ControllableContainer *,ControllableContainer *)
{
}