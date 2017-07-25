/*
 ==============================================================================

 Trigger.cpp
 Created: 8 Mar 2016 1:09:29pm
 Author:  bkupe

 ==============================================================================
 */

#include "Trigger.h"



#include "JsHelpers.h"


const var Trigger::triggerVar;

Trigger::Trigger(const String & niceName, const String &description, bool enabled) :
Parameter(TRIGGER, niceName, description, enabled,false,false,true)
{
  Controllable::isSavable = false;
}






DynamicObject * Trigger::createDynamicObject()
{
	DynamicObject* dObject = Controllable::createDynamicObject();
	dObject->setMethod(jsTriggerIdentifier, setControllableValueFromJS);
	dObject->setProperty(jsPtrIdentifier, (int64)this);

	return dObject;
}
void Trigger::setFromVarObject(DynamicObject & ob){
  jassertfalse;
}

var Trigger::getVarObject(){
  var res = new DynamicObject();
  return res;
}
var Trigger::getVarState(){
  // isSavable should be false
  jassertfalse;
  return var::undefined();
}
