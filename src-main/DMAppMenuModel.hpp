#pragma once

#include "DarkMark.hpp"


namespace dm
{

class DMAppMenuModel : public juce::MenuBarModel
{
public:
    enum ItemId
    {
        aboutDarkMark = 1,
    };

    DMAppMenuModel() = default;
    ~DMAppMenuModel() override = default;

    juce::StringArray getMenuBarNames() override
    {
        return {
            // "Help"
        };
    }

    juce::PopupMenu getMenuForIndex (int, const juce::String&) override
    {
        return {};
    }

    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override
    {
        // Items from extraAppleMenuItems come with topLevelMenuIndex == -1
        if (topLevelMenuIndex != -1)
            return;

        switch (menuItemID)
        {
            case aboutDarkMark:
                if (onAbout) onAbout();
                break;

            default:
                break;
        }
    }

    std::function<void()> onAbout;
};

} // namespace dm