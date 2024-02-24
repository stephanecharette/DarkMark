// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::Notebook::Notebook() :
		TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop)
{
	return;
}


dm::Notebook::~Notebook()
{
	return;
}


void dm::Notebook::currentTabChanged(int newCurrentTabIndex, const String & newCurrentTabName)
{
	for (int idx = 0; idx < getNumTabs(); idx ++)
	{
		if (idx != newCurrentTabIndex)
		{
			setTabBackgroundColour(idx, Colours::darkgrey);
		}
	}

	setTabBackgroundColour(newCurrentTabIndex, Colours::lightgrey);

	return;
}
