/* 
 * File:   draw/vertex.h
 * Author: Miles Lacey
 * 
 * @file Vertex types and identifiers.
 * This header describes the vertex types that can be used throughout the LS
 * framework. Additional vertex types are added and updated as necessary.
 *
 * Created on April 30, 2014, 8:51 PM
 */

#ifndef __LS_DRAW_VERTEX_H__
#define	__LS_DRAW_VERTEX_H__

#include <cstddef> // std::offsetof
#include "lightsky/draw/setup.h"

namespace ls {
namespace draw {

/**----------------------------------------------------------------------------
 * Basic vertex building block in LS.
 * This vertex type contains enough information to light a textured mesh.
-----------------------------------------------------------------------------*/
struct vertex {
    math::vec3 pos;
    math::vec2 uv;
    math::vec3 norm;
};

/**----------------------------------------------------------------------------
 * These enumerations can be used to describe vertex layouts to VAO objects.
-----------------------------------------------------------------------------*/
enum class vertex_attrib_t : int {
    VERTEX_ATTRIB_POS = 0,
    VERTEX_ATTRIB_TEX = 1,
    VERTEX_ATTRIB_NORM = 2,
    
    VERTEX_ATTRIB_MAT_ROW = 3,
    VERTEX_ATTRIB_MAT_ROW0 = 3, /* Row 1, mat4_t<>[0][] */
    VERTEX_ATTRIB_MAT_ROW1 = 4, /* Row 2, mat4_t<>[1][] */
    VERTEX_ATTRIB_MAT_ROW2 = 5, /* Row 3, mat4_t<>[2][] */
    VERTEX_ATTRIB_MAT_ROW3 = 6 /* Row 4, mat4_t<>[3][] */
};

/**----------------------------------------------------------------------------
 * These enumerations can be used to describe vertex layouts to VAO objects.
-----------------------------------------------------------------------------*/
enum class vertex_desc_t : int {
    ELEMENT_COUNT_POS = offsetof(vertex, pos),
    ELEMENT_COUNT_TEX = offsetof(vertex, uv),
    ELEMENT_COUNT_NORM = offsetof(vertex, norm),
    
    ELEMENT_COUNT_MAT_ROW = 0,
    ELEMENT_COUNT_MAT_ROW0 = sizeof(ls::math::vec4)*0,
    ELEMENT_COUNT_MAT_ROW1 = sizeof(ls::math::vec4)*1,
    ELEMENT_COUNT_MAT_ROW2 = sizeof(ls::math::vec4)*2,
    ELEMENT_COUNT_MAT_ROW3 = sizeof(ls::math::vec4)*3
};

} // end draw namespace
} // end ls namespace

#endif	/* __LS_DRAW_VERTEX_H__ */