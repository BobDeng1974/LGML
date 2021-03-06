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

#include "EnumParameterUI.h"

#include "../UndoableHelper.h"

#include "../../../Engine.h"

#if JUCE_DEBUG && 0
#define DBGENUM(x) DBG(x)
#else
#define DBGENUM(x)
#endif

EnumParameterUI::EnumParameterUI ( ParameterBase* parameter) :
ParameterUI (parameter),
ep ((EnumParameter*)parameter),
lastId (NoneId),
isSorted(true),
hoveredByFile(false)
{
    addMouseListener(this, true);
    cb.addListener (this);
    cb.setTextWhenNoChoicesAvailable ("No choices for" + ep->niceName);
    cb.setTextWhenNothingSelected (ep->niceName);
    cb.setTooltip (ParameterUI::getTooltip());
    cb.setEditableText (ep->userCanEnterText);
    addAndMakeVisible (cb);
    ep->addAsyncEnumParameterListener (this);


    updateComboBox();
}

EnumParameterUI::~EnumParameterUI()
{
    if(parameter.get()){
        ep->removeAsyncEnumParameterListener (this);
    }
    cb.removeListener (this);
}

void EnumParameterUI::mouseDown(const MouseEvent & e){
    if (!parameter){jassertfalse; return;}
    ParameterUI::mouseDown(e);
    if(e.mods.isLeftButtonDown())
        ep->getModel()->refresh();
}

void EnumParameterUI::updateComboBox()
{
    cb.clear (dontSendNotification);


    keyIdMap.clear();
    idKeyMap.clear();

    if (EnumParameterModel* mod = ep->getModel())
    {
        int id = 1;
        cb.addItem (juce::translate("None"), NoneId);
        NamedValueSet map = mod->getProperties();
        StringArray keys ;
        for(auto & k:map){
            keys.add(k.name.toString());
        }
        if(isSorted){keys.sort(true);}


        for (auto& key : keys)
        {

            //            String key = kv.name.toString();
            String displayed = juce::translate(key);//+" ["+kv.value.toString()+"]";
            cb.addItem (displayed, id);
            idKeyMap.set (id, key);
            keyIdMap.set (key, id);
            id++;
        }

    }


    String sel = ep->getFirstSelectedId().toString();
    //    DBGENUMENUM("enum CB Select " << sel << " ; " << ep->shortName);
    selectString (sel);

    if (ep->isEditable)
    {
        cb.addItem (juce::translate("add")+" " + ep->niceName, addElementId);
        cb.addItem (juce::translate("remove")+" " + ep->niceName, removeElementId);
    }
}

String EnumParameterUI::getCBSelectedKey()
{
    if (cb.getSelectedId() < 0)
    {
        jassertfalse;
        return "";
    }

    return idKeyMap[cb.getSelectedId()];
}

void EnumParameterUI::resized()
{
    cb.setBounds (getLocalBounds());
}

void EnumParameterUI::enumOptionAdded (EnumParameter*, const Identifier&)
{
    updateComboBox();
}

void EnumParameterUI::enumOptionRemoved (EnumParameter*, const Identifier&)
{
    updateComboBox();
}
void EnumParameterUI::enumOptionSelectionChanged (EnumParameter*, bool _isSelected, bool isValid, const Identifier& name)
{
    DBGENUM ("enum change : " <<parameter->niceName <<"->"<< name.toString() << (!_isSelected ? " not" : "") << " selected " << (!isValid ? "in-" : "") << "valid");

    if (isValid)
    {
        if(!keyIdMap.contains (name.toString())){
            updateComboBox();
            jassert(keyIdMap.contains (name.toString()));
        }
        cb.setSelectedId (_isSelected ? keyIdMap[name.toString()] : 0, dontSendNotification);
    }
}

void EnumParameterUI::valueChanged (const var& value)
{
    if (value.isString())
    {
        selectString (value.toString());
    }
    else if (value.isObject())
    {
        updateComboBox();
    }
}

void EnumParameterUI::comboBoxChanged (ComboBox* c)
{
    jassert (c == &cb);
    int id = cb.getSelectedId();

    if (id <= 0)
    {
        if (id == 0 )
        {
            String v = cb.getText();

            if (v.isNotEmpty() && (v != cb.ComboBox::getTextWhenNothingSelected()))
            {
                jassert (ep->userCanEnterText);
                ep->addOrSetOption (v, v, true);
                UndoableHelpers::setValueUndoable(ep, v);
            }
            else
            {
                UndoableHelpers::setValueUndoable(ep, var::undefined());

            }
        }
        else if (id == addElementId )
        {
            if (auto addFunction = ep->getModel()->addFunction)
            {

                auto res = addFunction (ep);

                if (std::get<0> (res))
                {
                    ep->addOption (std::get<1> (res), std::get<2> (res));
                    ep->selectId (std::get<1> (res), true, false);
                }
                else
                {
                    cb.setSelectedId (lastId, dontSendNotification);
                }
            }
            else
            {
                AlertWindow nameWindow ("which element should be added ?", "type the elementName", AlertWindow::AlertIconType::NoIcon, this);
                nameWindow.addTextEditor ("paramToAdd", parameter->stringValue());
                nameWindow.addButton ("OK", 1, KeyPress (KeyPress::returnKey));
                nameWindow.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

                int result = nameWindow.runModalLoop();
                bool changedFromUI = false;

                if (result)
                {
                    String res = nameWindow.getTextEditorContents ("paramToAdd");

                    if (res.isNotEmpty())
                    {
                        Identifier elemToAdd = res;
                        ep->getModel()->addOption (elemToAdd, elemToAdd.toString(), false);
                        ep->selectId (elemToAdd, true, false);
                        changedFromUI = true;
                    }
                }

                if (!changedFromUI)
                {
                    cb.setSelectedId (lastId, dontSendNotification);
                }


            }
        }
        else if (id == removeElementId)
        {

            AlertWindow nameWindow ("which element should be removed ?", "type the elementName", AlertWindow::AlertIconType::NoIcon, this);
            nameWindow.addTextEditor ("paramToRemove", ep->getFirstSelectedId().toString());
            nameWindow.addButton ("OK", 1, KeyPress (KeyPress::returnKey));
            nameWindow.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));

            int result = nameWindow.runModalLoop();
            bool changedFromUI = false;

            if (result)
            {
                String elemToRemove = nameWindow.getTextEditorContents ("paramToRemove");

                if (elemToRemove.isNotEmpty())
                {
                    ep->getModel()->removeOption (elemToRemove, false);
                    changedFromUI = true;
                }
            }

            if (!changedFromUI)
            {
                cb.setSelectedId (lastId, dontSendNotification);
            }
            else{

                cb.setTextWhenNothingSelected (ep->niceName);
                cb.setSelectedId(NoneId);

            }
        }
        else if ( id == NoneId)
        {
            cb.setSelectedId (0);
        }

    }
    else
    {
        UndoableHelpers::startNewTransaction(ep,true);
        UndoableHelpers::setValueUndoable(ep,getCBSelectedKey());
    }
};

void EnumParameterUI::selectString (const juce::String& s)
{
    if (keyIdMap.contains (s))
    {
        lastId = keyIdMap[s];
        cb.setSelectedId (lastId, dontSendNotification);
        cb.setTextWhenNothingSelected (ep->niceName);
    }
    else if (s != "")
    {
        // not valid
        cb.setTextWhenNothingSelected ("[" + s + "]");
    }
    else
    {
        lastId = NoneId;
    }
}

bool EnumParameterUI::isInterestedInFileDrag (const StringArray& files) {
    return ep->getModel()->isFileBased;
};
void EnumParameterUI::fileDragEnter (const StringArray& files, int x, int y) {
    hoveredByFile = true;
    repaint();
};
void EnumParameterUI::fileDragMove (const StringArray& files, int x, int y) {

};
void EnumParameterUI::fileDragExit (const StringArray& files) {
    hoveredByFile = false;
    repaint();
};
void EnumParameterUI::filesDropped (const StringArray& files, int x, int y) {
    String fname;
    bool needRemove = ep->getFirstSelectedId()!=Identifier::null;
    for(auto & fp :files){
        File f(fp);
        fname = f.getFileNameWithoutExtension();
        needRemove &= (fname!=ep->getFirstSelectedId().toString());
        String fpath = getEngine()->getNormalizedFilePath(f.getFullPathName());
        ep->addOrSetOption(fname, fpath,false);
    }
    if(fname.isNotEmpty()){
        if( needRemove){
            ep->removeOption(ep->getFirstSelectedId());
        }
        ep->selectId (fname, true, false);
    }
    hoveredByFile = false;
    repaint();

};

void EnumParameterUI::paintOverChildren(Graphics & g) {
    if(hoveredByFile){
        auto r = getLocalBounds();
        bool willReplace = ep->getFirstSelectedId()!=Identifier::null;
        g.setColour(willReplace?Colours::red:findColour(TextButton::buttonOnColourId));
        g.drawRect(r);
    }
    
    
}



#endif
