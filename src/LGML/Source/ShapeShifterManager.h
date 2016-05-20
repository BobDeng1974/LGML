/*
  ==============================================================================

    ShapeShifterManager.h
    Created: 2 May 2016 3:11:35pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef SHAPESHIFTERMANAGER_H_INCLUDED
#define SHAPESHIFTERMANAGER_H_INCLUDED

#include "ShapeShifterContainer.h"
#include "ShapeShifterWindow.h"
#include "ShapeShifterFactory.h"

class ShapeShifterManager :
	public ShapeShifterPanel::Listener
{
public:
	juce_DeclareSingleton(ShapeShifterManager,true);
	ShapeShifterManager();
	virtual ~ShapeShifterManager();

	ShapeShifterContainer mainContainer;

	OwnedArray<ShapeShifterPanel> openedPanels;
	OwnedArray<ShapeShifterWindow> openedWindows;

	ShapeShifterPanel * currentCandidatePanel;
	void setCurrentCandidatePanel(ShapeShifterPanel *);

	ShapeShifterPanel * getPanelForContent(ShapeShifterContent * content);
	ShapeShifterPanel * getPanelForContentName(const String & name);
	ShapeShifterPanel * createPanel(ShapeShifterContent * content, ShapeShifterPanelTab * sourceTab = nullptr);
	void removePanel(ShapeShifterPanel * panel);

	ShapeShifterWindow * showPanelWindow(ShapeShifterPanel * _panel, Rectangle<int> bounds);
	ShapeShifterWindow * showPanelWindowForContent(PanelName panelName);
	void closePanelWindow(ShapeShifterWindow * window, bool doRemovePanel);

	ShapeShifterContent * getContentForName(PanelName contentName);

	ShapeShifterPanel * checkCandidateTargetForPanel(ShapeShifterPanel * panel);
	bool checkDropOnCandidateTarget(ShapeShifterPanel * panel);

	ShapeShifterWindow * getWindowForPanel(ShapeShifterPanel * panel);

	void loadLayout(var layoutObject);
	var getCurrentLayout();
	void loadLayoutFromFile(int fileIndexInLayoutFolder =-1);
	void loadDefaultLayoutFile();
	void saveCurrentLayout();
	Array<File> getLayoutFiles();

	void clearAllPanelsAndWindows();

	const int baseMenuCommandID = 0x31000;
	const int baseSpecialMenuCommandID = 0x32000;
	PopupMenu getPanelsMenu();

	void handleMenuPanelCommand(int commandID);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeShifterManager)
};

#endif  // SHAPESHIFTERMANAGER_H_INCLUDED
