/*
  ==============================================================================

    ControllableContainer.cpp
    Created: 8 Mar 2016 1:15:36pm
    Author:  bkupe

  ==============================================================================
*/

#include "ControllableContainer.h"

ControllableContainer::ControllableContainer(const String & niceName) : 
	niceName(niceName), 
	parentContainer(nullptr), 
	hasCustomShortName(false),
	skipControllableNameInAddress(false)
{
	setNiceName(niceName);

}

ControllableContainer::~ControllableContainer()
{
	controllables.clear();
}

FloatParameter * ControllableContainer::addFloatParameter(const String & niceName, const String & description, const float & initialValue, const float & minValue, const float & maxValue, const bool & enabled)
{
	if (getControllableByName(niceName) != nullptr)
	{
		DBG("ControllableContainer::add parameter, short Name already exists : " + niceName);
		return nullptr;
	}

	FloatParameter * p = new FloatParameter(niceName, description, initialValue, minValue, maxValue, enabled);
	addParameterInternal(p);
	return p;
}

IntParameter * ControllableContainer::addIntParameter(const String & niceName, const String & description, const int & initialValue, const int & minValue, const int & maxValue, const bool & enabled)
{
	if (getControllableByName(niceName) != nullptr)
	{
		DBG("ControllableContainer::add parameter, short Name already exists : " + niceName);
		return nullptr;
	}

	IntParameter * p = new IntParameter(niceName, description, initialValue, minValue, maxValue, enabled);
	addParameterInternal(p);
	return p;
}

BoolParameter * ControllableContainer::addBoolParameter(const String & niceName, const String & description, const bool & value, const bool & enabled)
{
	if (getControllableByName(niceName) != nullptr)
	{
		DBG("ControllableContainer::add parameter, short Name already exists : " + niceName);
		return nullptr;
	}

	BoolParameter * p = new BoolParameter(niceName, description, value, enabled);
	addParameterInternal(p);
	return p;
}

StringParameter * ControllableContainer::addStringParameter(const String & niceName, const String & description, const String &value, const bool & enabled)
{
	if (getControllableByName(niceName) != nullptr)
	{
		DBG("ControllableContainer::add parameter, short Name already exists : " + niceName);
		return nullptr;
	}

	StringParameter * p = new StringParameter(niceName, description, value, enabled);
	addParameterInternal(p);
	return p;
}

Trigger * ControllableContainer::addTrigger(const String & niceName, const String & description, const bool & enabled)
{
	if (getControllableByName(niceName) != nullptr)
	{
		DBG("ControllableContainer::add trigger, short Name already exists : " + niceName);
		return nullptr;
	}

	Trigger * t = new Trigger(niceName, description, enabled);
	controllables.add(t);
	t->setParentContainer(this);
	t->addListener(this);

	listeners.call(&ControllableContainer::Listener::controllableAdded, t);
	return t;
}

void ControllableContainer::removeControllable(Controllable * c)
{
	listeners.call(&ControllableContainer::Listener::controllableRemoved, c);
	controllables.removeObject(c);
}

Controllable * ControllableContainer::getControllableByName(const String & niceName)
{
	for (auto &c : controllables)
	{
		if (c->niceName == niceName) return c;
	}

	return nullptr;
}

void ControllableContainer::addChildControllableContainer(ControllableContainer * container)
{
	controllableContainers.add(container);

	container->setParentContainer(this);
}

void ControllableContainer::removeChildControllableContainer(ControllableContainer * container)
{
	container->setParentContainer(nullptr);
	controllableContainers.removeAllInstancesOf(container);
}

void ControllableContainer::setParentContainer(ControllableContainer * container)
{
	this->parentContainer = container;
	for (auto &c : controllables) c->updateControlAddress();
	for (auto &cc : controllableContainers)cc->updateChildrenControlAddress();

}

void ControllableContainer::updateChildrenControlAddress()
{
	for (auto &c : controllables) c->updateControlAddress();

}

Array<Controllable*> ControllableContainer::getAllControllables(bool recursive)
{
	DBG("get All controllables");
	Array<Controllable*> result;
	for (auto &c : controllables) if(c->isControllableExposed) result.add(c);

	if (recursive)
	{
		for (auto &cc : controllableContainers) result.addArray(cc->getAllControllables(true));
	}

	return result;
}

Controllable * ControllableContainer::getControllableForAddress(Array<String> addressSplit, bool recursive, bool getNotExposed)
{

	bool isTargetControllable = addressSplit.size() == 1;
	if (isTargetControllable)
	{
		for (auto &c : controllables)
		{
			if (c->shortName == addressSplit[0])
			{
				if (c->isControllableExposed || getNotExposed) return c;
				else return nullptr;
			}
		}
	}
	else
	{
		
		for (auto &cc : controllableContainers)
		{
			if (cc->shortName == addressSplit[0])
			{
				addressSplit.remove(0);
				return cc->getControllableForAddress(addressSplit);
			}
		}
	}

	return nullptr;
}

void ControllableContainer::dispatchFeedback(Controllable * c)
{
	if (parentContainer != nullptr) parentContainer->dispatchFeedback(c);
	else listeners.call(&ControllableContainer::Listener::controllableFeedbackUpdate, c);
}



void ControllableContainer::parameterValueChanged(Parameter * p)
{
	if (p->isControllableExposed) dispatchFeedback(p);
}

void ControllableContainer::triggerTriggered(Trigger * t)
{
	if (t->isControllableExposed) dispatchFeedback(t);
}




void ControllableContainer::addParameterInternal(Parameter * p)
{
	p->setParentContainer(this);
	controllables.add(p);
	p->addListener(this);
	listeners.call(&ControllableContainer::Listener::controllableAdded, p);
}


