/*
  ==============================================================================

    Controller.h
    Created: 2 Mar 2016 8:49:50pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED


#include "ControllableContainer.h"
#include "ControlVariable.h"

class ControllerUI;
class ControllerEditor;


class Controller :
	public ControllableContainer,
	public ControlVariableListener
{
public:
    Controller(const String &name = "[Controller]");
    virtual ~Controller();

    int controllerTypeEnum;

    BoolParameter * enabledParam;

	Trigger * activityTrigger;

	OwnedArray<ControlVariable> variables; // These are values that can be set only by the external controller (osc, midi, serial...).
										   // they are stored so they can be used by other mechanisms in the software, such as rules.

	ControlVariable * addVariable(Parameter * p);
	void removeVariable(ControlVariable * variable);

	ControlVariable * getVariableForAddress(const String &address);
	ControlVariable * getVariableForName(const String &name);
    virtual void internalVariableAdded(ControlVariable * ){};
    virtual void internalVariableRemoved(ControlVariable * ){};
    void remove(); //will dispatch askForRemoveController
    virtual void onContainerParameterChanged(Parameter * p) override;
  virtual void onContainerTriggerTriggered(Trigger * ) override;

	void askForRemoveVariable(ControlVariable * variable)override;

	var getJSONData() override;
	void loadJSONDataInternal(var data) override;


	//helper
	String getUniqueVariableNameFor(const String &baseName, int index = 1);


	virtual ControllerUI * createUI();
  virtual ControllerEditor *  createEditor() ;



  struct VariableChangeMessage{
    VariableChangeMessage(Controller * c,ControlVariable * v,bool a):controller(c),variable(v),isAdded(a){};
    Controller * controller;
    ControlVariable* variable;
    bool isAdded;
  };

  class  ControllerListener:public QueuedNotifier<VariableChangeMessage>::Listener
    {
    public:
        /** Destructor. */
        virtual ~ControllerListener() {}

      void newMessage(const VariableChangeMessage & message) override{
        if(message.isAdded){
          variableAddedAsync(message.controller, message.variable);
        }
        else{
          variableRemovedAsync(message.controller, message.variable);
        }
      };

		virtual void askForRemoveController(Controller *) {}

		virtual void variableAdded(Controller *, ControlVariable * ) {}
		virtual void variableRemoved(Controller *, ControlVariable * ) {}
      virtual void variableAddedAsync(Controller *, ControlVariable * ) {}
      virtual void variableRemovedAsync(Controller *, ControlVariable * ) {}

    };

    ListenerList<ControllerListener> controllerListeners;
    QueuedNotifier<VariableChangeMessage> variableChangeNotifier;
  void addControllerListener(ControllerListener* newListener) { controllerListeners.add(newListener);variableChangeNotifier.addListener(newListener); }
  void removeControllerListener(ControllerListener* listener) { controllerListeners.remove(listener);variableChangeNotifier.removeListener(listener); }

  
    // identifiers
    static const Identifier controllerTypeIdentifier;
  static const Identifier variableNameIdentifier;
    static const Identifier variableMinIdentifier;
    static const Identifier variableMaxIdentifier;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Controller)
};



#endif  // CONTROLLER_H_INCLUDED
