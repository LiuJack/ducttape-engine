
// ----------------------------------------------------------------------------
// This file is part of the Ducttape Project (http://ducttape-dev.org) and is
// licensed under the GNU LESSER PUBLIC LICENSE version 3. For the full license
// text, please see the LICENSE file in the root of this project or at
// http://www.gnu.org/licenses/lgpl.html
// ----------------------------------------------------------------------------

#ifndef DUCTTAPE_ENGINE_SCENE_SCENE
#define DUCTTAPE_ENGINE_SCENE_SCENE

#include <Config.hpp>

#include <Event/EventListener.hpp>
#include <Scene/Node.hpp>

#include <OgreSceneManager.h>

#include <memory>
#include <string>

namespace dt {

class Event;
class PhysicsWorld;

/**
  * A class to represent a whole scene of the game world.
  */
class DUCTTAPE_API Scene : public EventListener, public Node {
public:
    /**
      * Default constructor.
      */
    Scene(const std::string& name);

    void OnInitialize();

    void OnDeinitialize();

    /**
      * Returns the Ogre::SceneManager of this Scene.
      * @returns The Ogre::SceneManager of this Scene.
      */
    Ogre::SceneManager* GetSceneManager();

    void HandleEvent(std::shared_ptr<Event> e);

    /**
      * Returns the PhysicsWorld of this Scene. Creates the PhysicsWorld on-demand.
      * @returns The PhysicsWorld of this Scene.
      */
    PhysicsWorld* GetPhysicsWorld();

protected:
    bool _IsScene();

};

} // namespace dt

#endif
