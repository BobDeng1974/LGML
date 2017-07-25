/*
 ==============================================================================

 Parameter.h
 Created: 8 Mar 2016 1:08:19pm
 Author:  bkupe

 ==============================================================================
 */

#ifndef PARAMETER_H_INCLUDED
#define PARAMETER_H_INCLUDED

#include "Controllable.h"
#include "QueuedNotifier.h"


class Parameter : public Controllable,public AsyncUpdater
{
public:
  Parameter(const Type &type, const String & niceName, const String &description, var initialValue, var minValue, var maxValue, bool enabled = true);

  virtual ~Parameter() {Parameter::masterReference.clear();cancelPendingUpdate();}

  void setFromVarObject(DynamicObject & ob) override;

  var defaultValue;
  var value;
  var lastValue;
  var minimumValue;
  var maximumValue;
  void setRange(var,var);

  bool isEditable;
  
  bool isPresettable;
  bool isOverriden;
  // if true each set value doesn't do nothing until som reader call pushValue
  // useful for thread syncronization
  bool isCommitableParameter;

  void resetValue(bool silentSet = false);
  void setValue(var _value, bool silentSet = false, bool force = false,bool defferIt=false);

  // helpers to coalesce value until a reader pushes it
  // useful for threadSyncronization
  virtual void commitValue(var _value);
  virtual void pushValue(bool defered=true,bool force = false);

  var commitedValue;
  bool hasCommitedValue;

  virtual void setValueInternal(var & _value);

  virtual bool checkValueIsTheSame(var newValue, var oldValue); //can be overriden to modify check behavior

  //For Number type parameters
  void setNormalizedValue(const float &normalizedValue, bool silentSet = false, bool force = false);
  float getNormalizedValue();

  //helpers for fast typing
  float floatValue() const{ return (float)value; }
  double doubleValue() const{return (double)value;}
  int intValue() const { return (int)value; }
  bool boolValue() const{ return (bool)value; }
  virtual String stringValue() const { return value.toString(); }

  void notifyValueChanged(bool defferIt=false);

  virtual DynamicObject * createDynamicObject() override;


  //Listener
  class  Listener
  {
  public:
    /** Destructor. */
    virtual ~Listener() {}
    virtual void parameterValueChanged(Parameter * p) = 0;
    virtual void parameterRangeChanged(Parameter * ){};
  };



  ListenerList<Listener> listeners;
  void addParameterListener(Listener* newListener) { listeners.add(newListener); }
  void removeParameterListener(Listener* listener) { listeners.remove(listener); }

  bool isSettingValue;
  
  // ASYNC
  class  ParamWithValue{
  public:
    ParamWithValue(Parameter * p,var v):parameter(p),value(v){}
    Parameter * parameter;
    var value;
    bool isRange() const{return value.isArray();}

  };

  typedef QueuedNotifier<ParamWithValue>::Listener AsyncListener;
  QueuedNotifier<ParamWithValue> queuedNotifier;
  void handleAsyncUpdate()override;


  void addAsyncParameterListener(AsyncListener* newListener) { queuedNotifier.addListener(newListener); }
  void addAsyncCoalescedListener(AsyncListener* newListener) { queuedNotifier.addAsyncCoalescedListener(newListener); }
  void removeAsyncParameterListener(AsyncListener* listener) { queuedNotifier.removeListener(listener); }


  //JS Helper
  virtual var getVarObject() override;
  virtual var getVarState() override;


  virtual void tryToSetValue(var _value, bool silentSet , bool force ,bool defferIt);

private:



  void checkVarIsConsistentWithType();

  WeakReference<Parameter>::Master masterReference;
  friend class WeakReference<Parameter>;


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameter)

};


#endif  // PARAMETER_H_INCLUDED
