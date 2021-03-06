/*
 * File:   context.h
 * Author: Miles Lacey
 *
 * Created on August 2, 2014, 8:44 PM
 */

#ifndef CONTEXT_H
#define CONTEXT_H

class Display;



/**
 * The render context is a wrapper class for an SDL_GLContext structure. This
 * class helps to provide RAII to the OpenGL context created from a display.
 * If is also useful in providing information about the currently loaded OpenGL
 * resource handle.
 */
class Context final {
  private:
    /**
     * pContext is a void pointer to an SDL_GLContext structure.
     */
    void* pContext = nullptr;

  public:
    /**
     * @brief Constructor
     */
    Context();

    /**
     * Copy Constructor -- DELETED
     */
    Context(const Context&) = delete;

    /**
     * @brief Move Constructor
     *
     * Moves all render context data from the input parameter into *this. No
     * copies are performed.
     *
     * @param ctx
     * An R-Value reference to a render context.
     */
    Context(Context&& ctx);

    /**
     * @brief Destructor
     *
     * Frees all hardware handles and memory resources used by *this. This
     * effectively does the same thing as "terminate()."
     * Make sure the display object that this was created with has not yet
     * been destroyed.
     */
    ~Context();

    /**
     * Copy Operator -- DELETED
     */
    Context& operator=(const Context&ctx) = delete;

    /**
     * @brief Move Operator
     *
     * Moves all render context data from the input parameter into *this. No
     * copies are performed.
     *
     * @param context&&
     * An R-Value reference to a render context.
     */
    Context& operator=(Context&& ctx);

    /**
     * @brief Initializer method for *this.
     *
     * @param disp
     * A reference to a constant display object. This display object must
     * have already been initialized and created with OpenGL 3.3 core
     * attributes.
     *
     * @return bool
     * TRUE if a context was able to be created from the input display
     * object, FALSE if not.
     */
    bool init(const Display& disp, bool useVsync = true);

    /**
     * @brief Destructor
     *
     * Frees all hardware handles and memory resources used by *this.
     * Make sure the display object that this was created with has not yet
     * been destroyed.
     */
    void terminate();

    /**
     * Bind this render context to the active display. This method should
     * only be called on one display and one display only. Binding this
     * render context to more than one display may cause OpenGL to crash.
     *
     * @param disp
     * A constant reference to the dsplay object that *this context has
     * been initialized with.
     */
    void make_current(const Display& disp) const;

    /**
     * Get a pointer to the SDL_GLContext that is used by the active
     * display. This context must have been made current in order to be
     * used.
     *
     * @return A void pointer that can be safely casted to a SDL_GLContext.
     */
    void* get_context() const;

    /**
     * @brief Enable/Disable VSync
     *
     * This context must have been made current in order to be used.
     *
     * @param TRUE if vsync is desired, FALSE to disable it.
     */
    void set_vsync(bool vsync);

    /**
     * Determine if VSync is enabled or disabled within the current window.
     * This context must have been made current in order to be used.
     *
     * @return TRUE if VSync is enabled, FALSE if not.
     */
    bool get_vsync() const;

    /**
     * Swap the current display's front and back buffers. This method
     * should only be called on one display and one display only. Binding
     * this render context to more than one display may cause OpenGL to
     * crash.
     *
     * This context must have been made current in order to be used.
     *
     * @param disp
     * A constant reference to the dsplay object that *this context has
     * been initialized with.
     */
    void flip(const Display& disp) const;
};

/*-------------------------------------
    Get a pointer to the SDL_GLContext that is used by the active
    renderer.
-------------------------------------*/
inline void* Context::get_context() const {
    return pContext;
}

#endif  /* CONTEXT_H */
