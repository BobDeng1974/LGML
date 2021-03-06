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


#ifndef POINT2DPARAMETER_H_INCLUDED
#define POINT2DPARAMETER_H_INCLUDED

#include "MinMaxParameter.h"
#include "NumericParameter.h"
#include <juce_graphics/juce_graphics.h>

template<typename T>
class ParameterList : public MinMaxParameter,protected ParameterBase::Listener{
public:
    ParameterList (const String& niceName, const String& description = "", var startPoint=var::undefined(), var minPoint = var::undefined(), var maxPoint = var::undefined(), bool enabled = true);
    virtual ~ParameterList() {}

    void setMinMax (var min, var max,ParameterBase::Listener * notifier=nullptr) override;

    void parameterValueChanged ( ParameterBase* p,ParameterBase::Listener * notifier=nullptr) override;

    OwnedArray<NumericParameter<T>> paramsList;
    int size(){return paramsList.size();}

    var getMinForIdx(int i);
    var getMaxForIdx(int i);
private:
    bool checkValueIsTheSame (const var& v1, const var& v2) override;
    void setValueInternal (const var& _value) override;
    void updateParamsFromValue();
};




template<typename T>
class Point2DParameter : public ParameterList<T>
{
public:

    
    Point2DParameter (const String& niceName, const String& description = "", T x = T (0), T y = T (0), var minPoint = var::undefined(), var maxPoint = var::undefined(), bool enabled = true);
    ~Point2DParameter() {}

    void setPoint (const Point<T>& value,ParameterBase::Listener * notifier = nullptr);
    void setPoint (const T x, const T y,ParameterBase::Listener * notifier = nullptr);

    Point<T> getPoint() const;


    T getX() const ;
    T getY() const ;
    NumericParameter<T>* getXParam();
    NumericParameter<T>* getYParam();



    DECLARE_OBJ_TYPE (Point2DParameter<T>,"2D Point Parameter");
private:


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Point2DParameter)
};





#endif  // POINT2DPARAMETER_H_INCLUDED
