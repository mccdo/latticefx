/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#ifndef __LFX_CORE_IVTK_RENDERER_H__
#define __LFX_CORE_IVTK_RENDERER_H__ 1

#include <latticefx/core/vtk/Export.h>

#include <string>

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class IVTKRenderer IVTKRenderer.h <latticefx/core/vtk/IVTKRenderer.h>
 \brief This class provides an interface for vtk renderer common properties and settings
 \details This class provides an interface for vtk renderer common properties and settings*/

class LATTICEFX_CORE_VTK_EXPORT IVTKRenderer
{
public:
    ///Default constructor
	IVTKRenderer() {}

    ///Destructor
	virtual ~IVTKRenderer() {}

    ///Set the active vector name to tell the render what to put in the textures
    ///\param activeVector The active vector name to use. 
	///\note for streamlines vectors are used to render a stream of points as lines.
    void SetActiveVector( const std::string& activeVector );
	std::string GetActiveVector() const;

    ///Set the active scalar name to tell the render what to put in the textures
    ///\param activeScalar The active scalar name to use
    void SetActiveScalar( const std::string& activeScalar );
	std::string GetActiveScalar() const;

    ///Set the color by scalar
    ///\note Used in pipelines where the active scalar is used to make a surface
    ///or some other feature and a second scalar is used for color.
	///but for streamlines scalars are only used for color and vectors are used for position
    void SetColorByScalar( std::string const scalarName );
	std::string GetColorByScalar() const;

protected:

    ///The active vector to set which vector to use for rendering
    std::string m_activeVector;
    ///The active scalar to set which scalar to use for rendering
    std::string m_activeScalar;
    ///The color by scalar
    std::string m_colorByScalar;
	///The color by scalar
    std::string m_curScalar;
};

}
}
}

#endif