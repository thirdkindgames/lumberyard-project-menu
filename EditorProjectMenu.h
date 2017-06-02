/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non - commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain.We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors.We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to < http://unlicense.org/>
*/
#pragma once

#include <AzToolsFramework/ToolsMessaging/ProjectMenuBus.h>

class QAction;

class EditorProjectMenu 
    : public AzToolsFramework::ProjectMenuRequests::Handler
{
public:
    EditorProjectMenu();
    ~EditorProjectMenu();

    static EditorProjectMenu& Get() { return s_singleton;}

    // Action was selected in the menu system
    void ActionSelected(UINT id);
    // Creates the Qt project menu
    QMenu* CreateMenu(ActionManager* actionManager, QList< QMenu* >& topLevelMenus);
    // Overrides for the ProjectMenuRequests
    AZ::u32 RegisterItem(char const* name, AZ::u32 parentMenu, SelectedCallback onClicked) override;
    AZ::u32 RegisterSelectableItem(char const* name, AZ::u32 parentMenu, SelectedCallback onClicked, IsSelectedCallback isSelected) override;
    AZ::u32 RegisterMenu(char const* name, AZ::u32 parentMenu) override;

private:
    // Recursive function to instantiate each sub-menu
    void CreateMenu(ActionManager* actionManager, ActionManager::MenuWrapper& menu, AZ::u32 menuIndex);
    // Callback for updating a provided menu item
    void UpdateSelectableAction(QAction* action);
    
    static EditorProjectMenu s_singleton;

    struct Item
    {
        AZStd::string name;                 // Display name
        SelectedCallback handler;           // Callback on selection
        IsSelectedCallback isSelected;      // Callback to determine is selection
        QAction* action;                    // The Qt action for this item
        AZ::u32 parent;                     // The parent menu index
    };

    struct Menu
    {
        AZStd::string name;                 // Display name
        AZ::u32 parent;                     // The parent menu index
    };

    AZStd::vector<Item> m_items;
    AZStd::vector<Menu> m_menus;
};