/* 
 * File:   game/gameState.h
 * Author: Miles Lacey
 *
 * Created on November 15, 2013, 8:45 PM
 */

#ifndef __LS_GAME_GAME_STATE_H__
#define	__LS_GAME_GAME_STATE_H__

#include "lightsky/game/system.h"

/*-------------------------------------
    Forward declarations
-------------------------------------*/
struct SDL_WindowEvent;
struct SDL_KeyboardEvent;
struct SDL_TextInputEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseWheelEvent;
struct SDL_ControllerDeviceEvent;
struct SDL_ControllerAxisEvent;
struct SDL_ControllerButtonEvent;
struct SDL_JoyDeviceEvent;
struct SDL_JoyAxisEvent;
struct SDL_JoyBallEvent;
struct SDL_JoyButtonEvent;
struct SDL_JoyHatEvent;

namespace ls {
namespace game {

/**----------------------------------------------------------------------------
    A game_state_t is a type used in order to help manage interactions between
    ls::game::system objects and different game state objects.
-----------------------------------------------------------------------------*/
enum game_state_t : unsigned {
    GAME_INVALID = (unsigned)-1,
    
    GAME_PAUSED     = 0,
    GAME_RUNNING    = 1,
    GAME_STOPPED    = 2,
    GAME_INIT       = 3
};

/**----------------------------------------------------------------------------
    @brief Game State Base Class
    
    An Abstract base class that can be overridden to organize games onto stacks
    within an lsSubsystem. gameState objects are pushed onto a "stack" managed
    by the lsSubsystem. Game states are passed hardware events by subsystems
    when they are running.
-----------------------------------------------------------------------------*/
class gameState {
    friend class system;
    
    private:
        /**
         * A "const" pointer to the parent lsSubsystem which manages *this.
         * this pointer is assigned automatically when a system is pushed onto
         * the subsystem's stack. Do not try to modify this.
         */
        mutable system* pSystem; // set by an lsSubsystem upon initialization
        
        /**
         * The current state that is used by *this. This variable is also
         * assigned by the parent subsystem and should not be modified.
         */
        game_state_t currentState = GAME_STOPPED;
        
        /**
         * Used by the parent subsystem to help with gameState management.
         * 
         * @param sys
         * A reference to the parent subsystem.
         */
        void setParentSystem(system& sys);
        
    protected:
        // All hardware and "on###()" methods are called by their parent
        // subsystem.
        
        /*---------------------------------------------------------------------
            Keyboard Events
        ---------------------------------------------------------------------*/
        /**
         * Event which allows a game state to input events when a keyboard key
         * is disengaged.
         * 
         * @param evt
         * A reference to an SDL_KeyboardEvent
         */
        virtual void onKeyboardUpEvent(const SDL_KeyboardEvent& evt);
        
        /**
         * Event which allows a game state to input events when a keyboard key
         * is pressed.
         * 
         * @param evt
         * A reference to an SDL_KeyboardEvent
         */
        virtual void onKeyboardDownEvent(const SDL_KeyboardEvent& evt);
        
        /**
         * Event which allows a game state to input events when a keyboard key
         * is used for text entry.
         * 
         * @param evt
         * A reference to an SDL_TextInputEvent
         */
        virtual void onKeyboardTextEvent(const SDL_TextInputEvent& evt);
        
        /*---------------------------------------------------------------------
            Window Events
        ---------------------------------------------------------------------*/
        /**
         * Event which allows a game state to input events when the current
         * SDL display has been modified.
         * 
         * @param evt
         * A reference to an SDL_WindowEvent
         */
        virtual void onWindowEvent(const SDL_WindowEvent& evt);
        
        /*---------------------------------------------------------------------
            Mouse Events
        ---------------------------------------------------------------------*/
        /**
         * Event which allows a game state to input events when the mouse has
         * moved.
         * 
         * @param evt
         * A reference to an SDL_MouseMotionEvent
         */
        virtual void onMouseMoveEvent(const SDL_MouseMotionEvent& evt);
        
        /**
         * Event which allows a game state to input events when a button on the
         * user's mouse has been depressed.
         * 
         * @param evt
         * A reference to an SDL_MouseButtonEvent
         */
        virtual void onMouseButtonUpEvent(const SDL_MouseButtonEvent& evt);
        
        /**
         * Event which allows a game state to input events when a button on the
         * user's mouse has been engaged.
         * 
         * @param evt
         * A reference to an SDL_MouseButtonEvent
         */
        virtual void onMouseButtonDownEvent(const SDL_MouseButtonEvent& evt);
        
        /**
         * Event which allows a game state to input events when the scroll wheel
         * on the user's mouse has moved.
         * 
         * @param evt
         * A reference to an SDL_MouseWheelEvent
         */
        virtual void onMouseWheelEvent(const SDL_MouseWheelEvent& evt);
        
        /*---------------------------------------------------------------------
            Controller Events
        ---------------------------------------------------------------------*/
        /**
         * Event which allows a game state to input events when a controller
         * has been plugged in.
         * 
         * @param evt
         * A reference to an SDL_ControllerDeviceEvent
         */
        virtual void onControllerAddedEvent(const SDL_ControllerDeviceEvent& evt);
        
        /**
         * Event which allows a game state to input events when a controller
         * has disconnected.
         * 
         * @param evt
         * A reference to an SDL_ControllerDeviceEvent
         */
        virtual void onControllerRemovedEvent(const SDL_ControllerDeviceEvent& evt);
        
        /**
         * Event which allows a game state to input events when a controller
         * has been remapped.
         * 
         * @param evt
         * A reference to an SDL_ControllerDeviceEvent
         */
        virtual void onControllerRemappedEvent(const SDL_ControllerDeviceEvent& evt);
        
        /**
         * Event which allows a game state to input events when a controller
         * has modified its axes.
         * 
         * @param evt
         * A reference to an SDL_ControllerAxisEvent
         */
        virtual void onControllerAxisEvent(const SDL_ControllerAxisEvent& evt);
        
        /**
         * Event which allows a game state to input events when a controller
         * had a button released.
         * 
         * @param evt
         * A reference to an SDL_ControllerButtonEvent
         */
        virtual void onControllerButtonUpEvent(const SDL_ControllerButtonEvent& evt);
        
        /**
         * Event which allows a game state to input events when a controller
         * had a button pressed.
         * 
         * @param evt
         * A reference to an SDL_ControllerButtonEvent
         */
        virtual void onControllerButtonDownEvent(const SDL_ControllerButtonEvent& evt);
        
        /*---------------------------------------------------------------------
            Joystick Events
        ---------------------------------------------------------------------*/
        /**
         * Event which allows a game state to input events when a joystick
         * has been plugged.
         * 
         * @param evt
         * A reference to an SDL_JoyDeviceEvent
         */
        virtual void onJoyAddEvent(const SDL_JoyDeviceEvent& evt);
        
        /**
         * Event which allows a game state to input events when a joystick
         * has been disconnected.
         * 
         * @param evt
         * A reference to an SDL_JoyDeviceEvent
         */
        virtual void onJoyRemoveEvent(const SDL_JoyDeviceEvent& evt);
        
        /**
         * Event which allows a game state to input events when a joystick
         * has modified its axes.
         * 
         * @param evt
         * A reference to an SDL_JoyAxisEvent
         */
        virtual void onJoyAxisEvent(const SDL_JoyAxisEvent& evt);
        
        /**
         * Event which allows a game state to input events when a joystick
         * has changed it's trackball position.
         * 
         * @param evt
         * A reference to an SDL_JoyBallEvent
         */
        virtual void onJoyBallEvent(const SDL_JoyBallEvent& evt);
        
        /**
         * Event which allows a game state to input events when a joystick
         * had a button pressed.
         * 
         * @param evt
         * A reference to an SDL_JoyButtonEvent
         */
        virtual void onJoyButtonDownEvent(const SDL_JoyButtonEvent& evt);
        
        /**
         * Event which allows a game state to input events when a joystick
         * had a button released.
         * 
         * @param evt
         * A reference to an SDL_JoyButtonEvent
         */
        virtual void onJoyButtonUpEvent(const SDL_JoyButtonEvent& evt);
        
        /**
         * Event which allows a game state to input events when a joystick
         * hat button event has occurred.
         * 
         * @param evt
         * A reference to an SDL_JoyHatEvent
         */
        virtual void onJoyHatEvent(const SDL_JoyHatEvent& evt);
        
        /*---------------------------------------------------------------------
            System Events
        ---------------------------------------------------------------------*/
        /**
         * The onStart method is called by the parent subsystem when *this
         * object has been notified to start. Place all memory allocations here.
         * 
         * @return bool
         * TRUE to indicate that *this has successfully initialized, FALSE if
         * otherwise.
         */
        virtual bool onStart();
        
        /**
         * The onStop method is used by the parent subsystem to indicate that
         * *this game state should terminate. Place all memory cleanup here.
         */
        virtual void onStop();
        
        /**
         * This method is used to tell the current game state that it should
         * update its child components/variables/state.
         * 
         * @param tickTime
         * A floating-point number to indicate how many milliseconds
         * have passed since the last parent system update.
         */
        virtual void onRun(float tickTime);
        
        /**
         * This method is used by the parent subsystem to tell *this game state
         * that it's paused.
         * 
         * @param tickTime
         * A floating-point number to indicate how many milliseconds
         * have passed since the last parent system update.
         */
        virtual void onPause(float tickTime);
        
    public:
        /**
         * @brief Default Constructor
         */
        gameState();
        
        /**
         * Copy Constructor -- DELETED
         * 
         * LightSky makes no attempts to copy game state objects.
         */
        gameState(const gameState&) = delete;
        
        /**
         * @brief Move Constructor
         * 
         * Moves all memory allocations from the input parameter into *this.
         * This method may be called at some point by the parent subsystem.
         * 
         * @param gs
         * An r-value reference to a game state who's members are to be moved
         * into *this.
         */
        gameState(gameState&& gs);
        
        /**
         * @brief Destructor
         * 
         * It is recommended to perform all object cleanup using the "onStop()"
         * method. This should be used as a last-resort.
         */
        virtual ~gameState() = 0;
        
        /**
         * @brief Copy Operator -- DELETED
         * 
         * LightSky makes no attempts to copy game state objects.
         */
        gameState& operator=(const gameState&) = delete;
        
        /**
         * @brief Move Operator
         * 
         * Moves all memory allocations from the input parameter into *this.
         * This method may be called at some point by the parent subsystem.
         * 
         * @param gs
         * An r-value reference to a game state that's about to go out
         * of scope and be terminated.
         * 
         * @return A reference to *this.
         */
        gameState& operator=(gameState&& gs);
        
        /**
         * Get the current state of *this object.
         * This object's state is used by the parent subsystem to determine when
         * it should go out of scope and terminate.
         * 
         * @return game_state_t
         */
        game_state_t getState() const;
        
        /**
         * Set the operational state of *this.
         * 
         * @param s
         * A game_state_t which will be used to set the current runtime state
         * of *this.
         */
        void setState(game_state_t s);
        
        /*
         * Get the parent subsystem that manages *this.
         * 
         * @return A reference to *this object's managing lsSubsystem.
         */
        system& getParentSystem() const;
};

} // end game namespace
} // end ls namespace

#include "lightsky/game/generic/gameState_impl.h"

#endif	/* __LS_GAME_GAME_STATE_H__ */
