/* 
 * File:   draw/setup.h
 * Author: Miles Lacey
 *
 * Created on November 15, 2013, 8:37 PM
 */

#ifndef __LS_DRAW_SETUP_H__
#define	__LS_DRAW_SETUP_H__

#include "lightsky/setup/setup.h"
#include "lightsky/utils/assertions.h"
#include "lightsky/utils/hash.h"
#include "lightsky/utils/log.h"
#include "lightsky/math/math.h"

/*-----------------------------------------------------------------------------
    Default Display Resolution
-----------------------------------------------------------------------------*/
/*-------------------------------------
    Default Display Width
-------------------------------------*/
#ifndef LS_DRAW_DEFAULT_DISPLAY_WIDTH
    #define LS_DRAW_DEFAULT_DISPLAY_WIDTH 800
#endif

/*-------------------------------------
    Default Display Height
-------------------------------------*/
#ifndef LS_DRAW_DEFAULT_DISPLAY_HEIGHT
    #define LS_DRAW_DEFAULT_DISPLAY_HEIGHT 600
#endif

/*-------------------------------------
    Debugging Various Messages.
-------------------------------------*/
#ifdef LS_DEBUG
    namespace ls {
    namespace draw {
        void printGlError(int line, const char* file);
    } // end draw namespace
    } // end ls namespace
#else
    namespace ls {
    namespace draw {
        inline void printGlError(int, const char*) {}
    } // end draw namespace
    } // end ls namespace
#endif

#ifndef LOG_GL_ERR
    #define LOG_GL_ERR() ls::draw::printGlError(__LINE__, __FILE__)
#endif

#endif	/* __LS_DRAW_SETUP_H__ */