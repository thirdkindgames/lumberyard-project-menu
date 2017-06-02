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
#include "StdAfx.h"

#include "ActionManager.h"
#include "EditorProjectMenu.h"

#include <AzCore/std/algorithm.h>

EditorProjectMenu EditorProjectMenu::s_singleton;

EditorProjectMenu::EditorProjectMenu()
{
    AzToolsFramework::ProjectMenuRequests::Handler::BusConnect();
}

EditorProjectMenu::~EditorProjectMenu()
{
    AzToolsFramework::ProjectMenuRequests::Handler::BusDisconnect();
}

void EditorProjectMenu::ActionSelected(UINT id)
{
    AZ_Assert(id >= ID_PROJECT_MENU_FIRST && id <= ID_PROJECT_MENU_LAST, "Invalid id passed to OnProjectMenuSelected");
    if (id >= ID_PROJECT_MENU_FIRST && id <= ID_PROJECT_MENU_LAST)
    {
        AZ::u32 projectMenuIndex = id - static_cast<AZ::u32>(ID_PROJECT_MENU_FIRST);
        if (projectMenuIndex < m_items.size())
        {
            m_items[projectMenuIndex].handler(id, m_items[projectMenuIndex].name.c_str());
        }
    }
}

QMenu* EditorProjectMenu::CreateMenu(ActionManager* actionManager, QList< QMenu* >& topLevelMenus)
{
    QMenu* result = nullptr;

    if (!m_items.empty())
    {
        // Try and use the sys_game_folder var to name the menu
        QString name = "Project";
        ICVar* pCvar = gEnv->pConsole->GetCVar("sys_game_folder");
        if (pCvar)
        {
            name = pCvar->GetString();
        }

        // Find which menu shortcuts are already in use by Lumberyard (in case new menus are added)
        QList< QChar > usedShortcuts;
        for (auto menuIter = topLevelMenus.begin(); menuIter != topLevelMenus.end(); ++menuIter)
        {
            QString menuTitle = (*menuIter)->title();
            int ampersandIndex = menuTitle.indexOf('&');
            if (ampersandIndex >= 0)
            {
                usedShortcuts.push_back(menuTitle[ampersandIndex + 1].toUpper());
            }
        }

        // Find the first character in the menu name which is not used as a menu shortcut, make that the shortcut for this menu
        for (auto menuNameIter = name.begin(); menuNameIter != name.end(); )
        {
            if (!usedShortcuts.contains(menuNameIter->toUpper()))
            {
                name.insert(menuNameIter - name.begin(), '&');
                menuNameIter = name.end();
            }
            else
            {
                ++menuNameIter;
            }
        }

        // Create the menu
        ActionManager::MenuWrapper menu = actionManager->AddMenu(name);
        CreateMenu(actionManager, menu, s_noParent);

        // Now that we've registered all the actions, clean up the action strings (we remove ampersands as that's only for shortcuts)
        for (AZ::u32 i = 0; i < m_items.size(); ++i)
        {
            m_items[i].name.erase(AZStd::remove(m_items[i].name.begin(), m_items[i].name.end(), '&'), m_items[i].name.end());
        }

        result = menu;
    }

    return result;
}

void EditorProjectMenu::CreateMenu(ActionManager* actionManager, ActionManager::MenuWrapper& menu, AZ::u32 menuIndex)
{
    // Sub-menus
    for (AZ::u32 i = 0; i < m_menus.size(); ++i)
    {
        if (m_menus[i].parent == menuIndex)
        {
            ActionManager::MenuWrapper subMenu = menu.AddMenu(m_menus[i].name.c_str());
            CreateMenu(actionManager, subMenu, i);
        }
    }

    // Items
    for (AZ::u32 i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].parent == menuIndex)
        {
            AZ::u32 id = ID_PROJECT_MENU_FIRST + i;

            auto action = actionManager->AddAction(id, m_items[i].name.c_str());;
            if (m_items[i].isSelected)
            {
                action.SetCheckable(true).RegisterUpdateCallback(this, &EditorProjectMenu::UpdateSelectableAction);
            }
            m_items[i].action = action;
            menu.AddAction(id);
        }
    }
}

AZ::u32 EditorProjectMenu::RegisterItem(char const* name, AZ::u32 parentMenu, SelectedCallback onClicked)
{
    return RegisterSelectableItem(name, parentMenu, onClicked, nullptr);
}

AZ::u32 EditorProjectMenu::RegisterSelectableItem(char const* name, AZ::u32 parentMenu, SelectedCallback onClicked, IsSelectedCallback isSelected)
{
    AZ::u32 result = 0;

    if (m_items.size() < PROJECT_MENU_ACTION_COUNT)
    {
        result = static_cast<AZ::u32>(ID_PROJECT_MENU_FIRST) + m_items.size();

        Item item;
        item.name = name;
        item.handler = onClicked;
        item.isSelected = isSelected;
        item.parent = parentMenu;

        m_items.push_back(item);
    }
    else
    {
        AZ_Error("EditorProjectMenu", false, "Out of space for project menu actions, increase PROJECT_MENU_ACTION_COUNT.");
    }

    return result;
}

AZ::u32 EditorProjectMenu::RegisterMenu(char const* name, AZ::u32 parentMenu)
{
    AZ::u32 result = s_noParent;

    // Attempt to find an existing menu which matches this criteria
    for (AZ::u32 menuIndex = 0; menuIndex < m_menus.size() && result == s_noParent; ++menuIndex)
    {
        if (m_menus[menuIndex].parent == parentMenu && m_menus[menuIndex].name == name)
        {
            result = menuIndex;
        }
    }

    // If we didn't find a menu, add a menu
    if (result == s_noParent)
    {
        result = m_menus.size();

        Menu menu;
        menu.name = name;
        menu.parent = parentMenu;
        m_menus.push_back(menu);
    }

    return result;
}

void EditorProjectMenu::UpdateSelectableAction(QAction* action)
{
    for (AZ::u32 i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i].action == action)
        {
            bool checked = m_items[i].isSelected(i + ID_PROJECT_MENU_FIRST, m_items[i].name.c_str());
            action->setChecked(checked);
        }
    }
}