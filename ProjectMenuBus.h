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

#include <AzCore/EBus/EBus.h>
#include <AzCore/std/functional.h>

namespace AzToolsFramework
{
    struct ProjectMenuBus
        : public AZ::EBusTraits
    {
        // Called to check whether a menu item should be marked as selected
        //
        // AZ::u32 - id of the item being checked
        // char const* - display name of the item being checked
        //
        // return true to indicate selected, false otherwise
        using IsSelectedCallback = AZStd::function<bool(AZ::u32, char const*)>;
        
        // Called to indicate that a menu item was selected
        //
        // AZ::u32 - id of the item that was selected
        // char const* - display name of the item that was selected
        using SelectedCallback = AZStd::function<void(AZ::u32, char const*)>;
        
        // Used to indicate no parent menu option
        static AZ::u32 const s_noParent = (AZ::u32)-1;
        
        // Register an item with the project menu
        //
        // name - The name of the item for display purposes (add an '&' to indicate the keyboard shortcut)
        // parentMenu - The id of the parent menu for structuring
        // onClicked - The callback to call when selected
        //
        // returns - The id of the item, can be used in the callback to identify the option selected
        virtual AZ::u32 RegisterItem(char const* name, AZ::u32 parentMenu, SelectedCallback onClicked) = 0;
        
        // Same as RegisterItem except that you provide an additional IsSelectedCallback parameter to determine whether
        // the option is ticked or not.
        //
        // isSelected - The callback to call to determine whether the item is selected
        //
        // returns - The id of the item, can be used in the callback to identify the option selected
        virtual AZ::u32 RegisterSelectableItem(char const* name, AZ::u32 parentMenu, SelectedCallback onClicked, IsSelectedCallback isSelected) = 0;

        // Register a sub-menu with the project menu
        //
        // name - The name of the sub-menu for display purposes (add an '&' to indicate the keyboard shortcut)
        // parentMenu - The id of the parent menu for structuring
        //
        // returns - The id of the menu, can be passed in to other register functions to make them a child of this menu
        virtual AZ::u32 RegisterMenu(char const* name, AZ::u32 parentMenu) = 0;
    };
    using ProjectMenuRequests = AZ::EBus< ProjectMenuBus >;

    // Helper for binding SelectedCallback without having to add the placeholders
    template< typename T1 >
    inline ProjectMenuBus::SelectedCallback MenuItemSelectedCB(T1 f)
    {
        return AZStd::bind(f, AZStd::placeholders::_1, AZStd::placeholders::_2);
    }

    template< typename T1, typename T2 >
    inline ProjectMenuBus::SelectedCallback MenuItemSelectedCB(T1 f, T2 p)
    {
        return AZStd::bind(f, p, AZStd::placeholders::_1, AZStd::placeholders::_2);
    }

    // Helper for binding IsSelectedCallback without having to add the placeholders
    template< typename T1 >
    inline ProjectMenuBus::IsSelectedCallback MenuItemIsSelectedCB(T1 f)
    {
        return AZStd::bind(f, AZStd::placeholders::_1, AZStd::placeholders::_2);
    }

    template< typename T1, typename T2 >
    inline ProjectMenuBus::IsSelectedCallback MenuItemIsSelectedCB(T1 f, T2 p)
    {
        return AZStd::bind(f, p, AZStd::placeholders::_1, AZStd::placeholders::_2);
    }
}