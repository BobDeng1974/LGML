/*
  ==============================================================================

    GenericControllableContainerEditor.h
    Created: 9 May 2016 6:41:59pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef GENERICCONTROLLABLECONTAINEREDITOR_H_INCLUDED
#define GENERICCONTROLLABLECONTAINEREDITOR_H_INCLUDED

#include "InspectorEditor.h"
#include "ControllableContainer.h"
#include "ControllableUI.h"
#include "PresetChooser.h"

class GenericControllableContainerEditor;

class CCInnerContainer :
	public Component,
	public ControllableContainerListener,
	public ButtonListener
{
public:
	class CCLinkBT : public TextButton
	{
	public:
		CCLinkBT(ControllableContainer * targetContainer);
		ControllableContainer * targetContainer;
	};


	CCInnerContainer(GenericControllableContainerEditor * editor,ControllableContainer * container, int level, int maxLevel, bool canAccessLowerContainers);
	virtual ~CCInnerContainer();

	Label containerLabel;

	ControllableContainer * container;
	ScopedPointer<PresetChooser> presetChooser;

	OwnedArray<NamedControllableUI> controllablesUI;
	OwnedArray<CCInnerContainer> innerContainers;
  ScopedPointer<Component> customEditor;
	OwnedArray<Component> lowerContainerLinks;

	GenericControllableContainerEditor * editor;

	int level;
	int maxLevel;
	bool canAccessLowerContainers;

	void paint(Graphics &g) override;
	void resized() override;
	void clear();


	void addControllableUI(Controllable * c);
	void removeControllableUI(Controllable *c);

	void addCCInnerUI(ControllableContainer * cc);
	void removeCCInnerUI(ControllableContainer * cc);

	void addCCLink(ControllableContainer * cc);
	void removeCCLink(ControllableContainer * cc);

	int getContentHeight();

	NamedControllableUI * getUIForControllable(Controllable * c);
	CCInnerContainer * getInnerContainerForCC(ControllableContainer * cc);
	CCLinkBT * getCCLinkForCC(ControllableContainer * cc);

	void controllableAdded(ControllableContainer *,Controllable *)override;
	void controllableRemoved(ControllableContainer *,Controllable *)override;
	void controllableContainerAdded(ControllableContainer *,ControllableContainer *)override;
	void controllableContainerRemoved(ControllableContainer *,ControllableContainer *)override;
	void childStructureChanged(ControllableContainer *,ControllableContainer *)override;

	void buttonClicked(Button * b)override;
};


class GenericControllableContainerEditor : public InspectorEditor, public ButtonListener, public ControllableContainerListener,Timer
{
public :
	GenericControllableContainerEditor(ControllableContainer * sourceComponent);
	virtual ~GenericControllableContainerEditor();

	TextButton parentBT;

	WeakReference<ControllableContainer> sourceContainer;
	ScopedPointer<CCInnerContainer> innerContainer;

	void setCurrentInspectedContainer(ControllableContainer *,bool forceUpdate = false,int recursiveInspectionLevel=0,bool canInspectChildContainersBeyondRecursion=true);

	virtual int getContentHeight() override;

	void resized() override;
	void clear() override;

	void buttonClicked(Button  *b)override;

	void childStructureChanged(ControllableContainer *,ControllableContainer *) override;

  void handleCommandMessage(int cID)override;

  void timerCallback()override;

  enum{
    CHILD_STRUCTURE_CHANGED = 0
  }commandMessageIDs;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericControllableContainerEditor)
};


#endif  // GENERICCONTROLLABLECONTAINEREDITOR_H_INCLUDED
