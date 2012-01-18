
// ----------------------------------------------------------------------------
// This file is part of the Ducttape Project (http://ducttape-dev.org) and is
// licensed under the GNU LESSER PUBLIC LICENSE version 3. For the full license
// text, please see the LICENSE file in the root of this project or at
// http://www.gnu.org/licenses/lgpl.html
// ----------------------------------------------------------------------------

#include <Logic/AdvancedPlayerComponent.hpp>

#include <Scene/Node.hpp>
#include <Scene/Scene.hpp>
#include <Input/InputManager.hpp>
#include <Logic/InteractionComponent.hpp>

#include <BtOgreGP.h>

namespace dt {

AdvancedPlayerComponent::AdvancedPlayerComponent(const QString& name)
    : Component(name),
      mBtController(nullptr),
      mBtGhostObject(nullptr),
      mMouseEnabled(true),
      mMouseSensitivity(1.0),
      mMouseYInversed(false),
      mMove(0.0, 0.0, 0.0),
      mMoveSpeed(5.0f),
      mWASDEnabled(true),
      mArrowsEnabled(true),
      mJumpEnabled(true),
      mIsLeftOneShot(true),
      mIsRightOneShot(true),
      mIsMoving(false),
      mIsLeftMouseDown(false),
      mIsRightMouseDown(false) {}

void AdvancedPlayerComponent::OnCreate() {
    btTransform  start_trans;
    start_trans.setIdentity();
    start_trans.setOrigin(BtOgre::Convert::toBullet(GetNode()->GetPosition(Node::SCENE)));
    start_trans.setRotation(BtOgre::Convert::toBullet(GetNode()->GetRotation(Node::SCENE)));

    btScalar character_height = 1.75;
    btScalar character_width = 0.44;
    btConvexShape* capsule = new btCapsuleShape(character_width, character_height);

    mBtGhostObject = new btPairCachingGhostObject();
    mBtGhostObject->setWorldTransform(start_trans);
    mBtGhostObject->setCollisionShape(capsule);
    mBtGhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    /////////////////////////////////////////////////////////////////////////////////////////////////
    //TODO: Make a better way to distinguish between AdvancedPlayerComponent and PhysicsBodyComponent.
    //For now, it's just a null pointer check to do this.
    mBtGhostObject->setUserPointer(nullptr);
    /////////////////////////////////////////////////////////////////////////////////////////////////
    mBtController = new btKinematicCharacterController(mBtGhostObject, capsule, 1);
    GetNode()->GetScene()->GetPhysicsWorld()->GetBulletWorld()->addCollisionObject(mBtGhostObject);
    GetNode()->GetScene()->GetPhysicsWorld()->GetBulletWorld()->addAction(mBtController);

    if(!QObject::connect(InputManager::Get(), SIGNAL(sKeyPressed(const OIS::KeyEvent&)), 
        this, SLOT(_HandleKeyDown(const OIS::KeyEvent&)))) {
            Logger::Get().Error("Cannot connect the key pressed signal with " + GetName()
                + "'s keyboard input handling slot.");
    }
    if(!QObject::connect(InputManager::Get(), SIGNAL(sKeyReleased(const OIS::KeyEvent&)), 
        this, SLOT(_HandleKeyUp(const OIS::KeyEvent&)))) {
            Logger::Get().Error("Cannot connect the key released signal with " + GetName()
                + "'s keyboard input handling slot.");
    }
    if(!QObject::connect(InputManager::Get(), SIGNAL(sMouseMoved(const OIS::MouseEvent&)), 
        this, SLOT(_HandleMouseMove(const OIS::MouseEvent&)))) {
            Logger::Get().Error("Cannot connect the mouse moved signal with " + GetName()
                + "'s mouse input handling slot.");
    }
    if(!QObject::connect(InputManager::Get(), SIGNAL(sMousePressed(const OIS::MouseEvent&, OIS::MouseButtonID)), 
        this, SLOT(_HandleMouseDown(const OIS::MouseEvent&, OIS::MouseButtonID)))) {
            Logger::Get().Error("Cannot connect the mouse moved signal with " + GetName()
                + "'s mouse input handling slot.");
    }
    if(!QObject::connect(InputManager::Get(), SIGNAL(sMouseReleased(const OIS::MouseEvent&, OIS::MouseButtonID)), 
        this, SLOT(_HandleMouseUp(const OIS::MouseEvent&, OIS::MouseButtonID)))) {
            Logger::Get().Error("Cannot connect the mouse moved signal with " + GetName()
                + "'s mouse input handling slot.");
    }
}

void AdvancedPlayerComponent::OnDestroy() {
    if(mBtController)
        delete mBtController;
    if(mBtGhostObject)
        delete mBtGhostObject;
}

void AdvancedPlayerComponent::OnEnable() {
    SetWASDEnabled(true);
    SetArrowsEnabled(true);
    SetJumpEnabled(true);

    //Re-sychronize it.
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(BtOgre::Convert::toBullet(GetNode()->GetPosition(Node::SCENE)));
    transform.setRotation(BtOgre::Convert::toBullet(GetNode()->GetRotation(Node::SCENE)));

    mBtGhostObject->setWorldTransform(transform);
    GetNode()->GetScene()->GetPhysicsWorld()->GetBulletWorld()->addCollisionObject(mBtGhostObject);
}

void AdvancedPlayerComponent::OnDisable() {
    SetWASDEnabled(false);
    SetArrowsEnabled(false);
    SetJumpEnabled(false);

    mMove.setZero();

    GetNode()->GetScene()->GetPhysicsWorld()->GetBulletWorld()->removeCollisionObject(mBtGhostObject);
}

void AdvancedPlayerComponent::OnUpdate(double time_diff) {
    static Ogre::Vector3 move;
    static Ogre::Quaternion quaternion;
    static btTransform trans;

    quaternion = Ogre::Quaternion(GetNode()->GetRotation().getYaw(), Ogre::Vector3(0.0, 1.0, 0.0));
    move = quaternion * BtOgre::Convert::toOgre(mMove) * mMoveSpeed;
    mBtController->setVelocityForTimeInterval(BtOgre::Convert::toBullet(move), 0.5);

    trans = mBtGhostObject->getWorldTransform();

    GetNode()->SetPosition(BtOgre::Convert::toOgre(trans.getOrigin()), Node::SCENE);

    if(!move.isZeroLength() && mBtController->onGround() && !mIsMoving) {
        //Emit sMove when the player starts to move when he's stopping or has done jumpping.
        emit sMove();
        mIsMoving = true;
    }
    else if((move.isZeroLength() || !mBtController->onGround()) && mIsMoving) {
        //Emit sStop when the player starts to stop or jump while he's moving.
        emit sStop();
        mIsMoving = false;
    }

    if(mIsLeftMouseDown || mIsRightMouseDown) {
        _OnMousePressed();

        if(mIsLeftOneShot) {
            mIsLeftMouseDown = false;
        }
        if(mIsRightOneShot) {
            mIsRightMouseDown = false;
        }
    }

}

void AdvancedPlayerComponent::SetWASDEnabled(bool wasd_enabled) {
    mWASDEnabled = wasd_enabled;
}

bool AdvancedPlayerComponent::GetWASDEnabled() const {
    return mWASDEnabled;
}

void AdvancedPlayerComponent::SetArrowsEnabled(bool arrows_enabled) {
    mArrowsEnabled = arrows_enabled;
}

bool AdvancedPlayerComponent::GetArrowsEnabled() const {
    return mArrowsEnabled;
}

void AdvancedPlayerComponent::SetMoveSpeed(float move_speed) {
    mMoveSpeed = move_speed;
}

float AdvancedPlayerComponent::GetMoveSpeed() const {
    return mMoveSpeed;
}

void AdvancedPlayerComponent::SetMouseEnabled(bool mouse_enabled) {
    mMouseEnabled = mouse_enabled;
}

bool AdvancedPlayerComponent::GetMouseEnabled() const {
    return mMouseEnabled;
}

void AdvancedPlayerComponent::SetMouseSensitivity(float mouse_sensitivity) {
    mMouseSensitivity = mouse_sensitivity;
}

float AdvancedPlayerComponent::GetMouseSensitivity() const {
    return mMouseSensitivity;
}

void AdvancedPlayerComponent::SetMouseYInversed(bool mouse_y_inversed) {
    mMouseYInversed = mouse_y_inversed;
}

bool AdvancedPlayerComponent::GetMouseYInversed() const {
    return mMouseYInversed;
}

void AdvancedPlayerComponent::SetJumpEnabled(bool jump_enabled) {
    mJumpEnabled = jump_enabled;
}

bool AdvancedPlayerComponent::GetJumpEnabled() const{
    return mJumpEnabled;
}

void AdvancedPlayerComponent::_HandleKeyDown(const OIS::KeyEvent& event) {
    if(mWASDEnabled || mArrowsEnabled) {
        if((event.key == OIS::KC_W && mWASDEnabled) || (event.key == OIS::KC_UP && mArrowsEnabled)) {
            mMove.setZ(mMove.getZ() - 1.0f);
        }
        if((event.key == OIS::KC_S && mWASDEnabled) || (event.key == OIS::KC_DOWN && mArrowsEnabled)) {
            mMove.setZ(mMove.getZ() + 1.0f);
        }
        if((event.key == OIS::KC_A && mWASDEnabled) || (event.key == OIS::KC_LEFT && mArrowsEnabled)) {
            mMove.setX(mMove.getX() - 1.0f);
        }
        if((event.key == OIS::KC_D && mWASDEnabled) || (event.key == OIS::KC_RIGHT && mArrowsEnabled)) {
            mMove.setX(mMove.getX() + 1.0f);
        }
    }

    if(mJumpEnabled && event.key == OIS::KC_SPACE && mBtController->onGround()) {
        mBtController->jump();

        emit sStop();
        emit sJump();
    }
}

void AdvancedPlayerComponent::_HandleMouseMove(const OIS::MouseEvent& event) {
    if(mMouseEnabled) {
        float factor = mMouseSensitivity * -0.01;

        float dx = event.state.X.rel * factor;
        float dy = event.state.Y.rel * factor * (mMouseYInversed ? -1 : 1);

        if(dx != 0 || dy != 0) {
            // watch out for da gimbal lock !!

            Ogre::Matrix3 orientMatrix;
            GetNode()->GetRotation().ToRotationMatrix(orientMatrix);

            Ogre::Radian yaw, pitch, roll;
            orientMatrix.ToEulerAnglesYXZ(yaw, pitch, roll);

            pitch += Ogre::Radian(dy);
            yaw += Ogre::Radian(dx);

            // do not let it look completely vertical, or the yaw will break
            if(pitch > Ogre::Degree(89.9))
                pitch = Ogre::Degree(89.9);

            if(pitch < Ogre::Degree(-89.9))
                pitch = Ogre::Degree(-89.9);

            orientMatrix.FromEulerAnglesYXZ(yaw, pitch, roll);

            Ogre::Quaternion rot;
            rot.FromRotationMatrix(orientMatrix);
            GetNode()->SetRotation(rot);
        }
    }
}

void AdvancedPlayerComponent::_HandleKeyUp(const OIS::KeyEvent& event) {
    if(mWASDEnabled || mArrowsEnabled) {
        if((event.key == OIS::KC_W && mWASDEnabled) || (event.key == OIS::KC_UP && mArrowsEnabled)) {
            mMove.setZ(mMove.getZ() + 1.0f);
        }
        if((event.key == OIS::KC_S && mWASDEnabled) || (event.key == OIS::KC_DOWN && mArrowsEnabled)) {
            mMove.setZ(mMove.getZ() - 1.0f);
        }
        if((event.key == OIS::KC_A && mWASDEnabled) || (event.key == OIS::KC_LEFT && mArrowsEnabled)) {
            mMove.setX(mMove.getX() + 1.0f);
        }
        if((event.key == OIS::KC_D && mWASDEnabled) || (event.key == OIS::KC_RIGHT && mArrowsEnabled)) {
            mMove.setX(mMove.getX() - 1.0f);
        }
    }
}

void AdvancedPlayerComponent::_HandleMouseDown(const OIS::MouseEvent& event, OIS::MouseButtonID button) {
    if(mMouseEnabled) {
        if(button == OIS::MB_Left) {
            mIsLeftMouseDown = true;
        }
        else if(button == OIS::MB_Right) {
            mIsRightMouseDown = true;

        }
    }
}

void AdvancedPlayerComponent::_OnMousePressed() {}

void AdvancedPlayerComponent::_HandleMouseUp(const OIS::MouseEvent& event, OIS::MouseButtonID button) {
    if(mMouseEnabled) {
        if(button == OIS::MB_Left) {
            mIsLeftMouseDown = false;
        }
        else if(button == OIS::MB_Right) {
            mIsRightMouseDown = false;
        }
    }
}

bool AdvancedPlayerComponent::GetIsOneShot(OIS::MouseButtonID mouse_button) const {
    if(mouse_button == OIS::MB_Left) {
        return mIsLeftOneShot;
    }
    else {
        return mIsRightOneShot;
    }
}

void AdvancedPlayerComponent::SetIsOneShot(bool is_one_shot, OIS::MouseButtonID mouse_button) {
    if(mouse_button == OIS::MB_Left) {
        mIsLeftOneShot = is_one_shot;
    }
    else {
        mIsRightOneShot = is_one_shot;
    }
}

}