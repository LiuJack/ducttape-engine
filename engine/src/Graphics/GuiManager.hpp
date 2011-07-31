
// ----------------------------------------------------------------------------
// This file is part of the Ducttape Project (http://ducttape-dev.org) and is
// licensed under the GNU LESSER PUBLIC LICENSE version 3. For the full license
// text, please see the LICENSE file in the root of this project or at
// http://www.gnu.org/licenses/lgpl.html
// ----------------------------------------------------------------------------

#ifndef DUCTTAPE_ENGINE_GRAPHICS_GUIMANAGER
#define DUCTTAPE_ENGINE_GRAPHICS_GUIMANAGER


#define MYGUI_DONT_USE_OBSOLETE


namespace dt {

/**
  * Manager class for the GUI System.
  * @see http://mygui.info
  */
class DUCTTAPE_API GuiManager : public Manager, public EventListener {
public:
    /**
      * Default constructor.
      */
    GuiManager();

    void Initialize();
    void Deinitialize();
    void HandleEvent(std::shared_ptr<Event> e);

    /**
      * Sets the scene manager to use for the GUI display.
      * @param scene_manager The scene manager to use.
      */
    void SetSceneManager(Ogre::SceneManager* scene_manager);

    /**
      * Returns MyGUI's GUI system.
      * @returns The GUI System.
      */
    MyGUI::Gui* GetGuiSystem();

    /**
      * Only for internal use. Sets the visibility of the mouse cursor.
      * @see void InputManager::SetMouseCursorMode(MouseCursorMode mode);
      * @param visible Whether the mouse cursor should be visible.
      * @internal
      */
    void SetMouseCursorVisible(bool visible);

    /**
      * Returns a pointer to the Manager instance.
      * @returns A pointer to the Manager instance.
      */
    static GuiManager* Get();

private:
    MyGUI::Gui* mGuiSystem;         //!< MyGUI's GUI system.
    MyGUI::OgrePlatform* mPlatform; //!< MyGUI's OgrePlatform.
    bool mMouseCursorVisible;       //!< Whether the GUI mouse cursor is visible.

};

}

#endif
