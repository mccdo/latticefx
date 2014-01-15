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
#ifndef __LATTICEFX_CORE_VTK_ACTORRENDERER_H__
#define __LATTICEFX_CORE_VTK_ACTORRENDERER_H__ 1

#include <latticefx/core/vtk/Export.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/vtk/IVTKRenderer.h>

#include <string>

namespace osg
{
class Node;
}

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class VTKActorRenderer VTKActorRenderer.h <latticefx/core/vtk/VTKActorRenderer.h>
 \brief This class converts a vtkPolyData to OSG arrays for rendering.
 \details This class goes through a conversion process with vtkPolyData to enable
 the instance rendering tools to render a VTK vector field using GPU based instance
 rendering. */

class LATTICEFX_CORE_VTK_EXPORT VTKActorRenderer : public lfx::core::Renderer, public IVTKRenderer
{
public:
    ///Default constructor
    ///We are really a fancy lfx::core::VectorRenderer specific to VTK data
    VTKActorRenderer( const std::string& logName = std::string( "" ) )
        :
        lfx::core::Renderer( logName )
    {
        ;
    }

    ///Destructor
    virtual ~VTKActorRenderer()
    {
        ;
    }

	///Get a string name for this class
	virtual std::string getClassName() const { return std::string( "VTKActorRenderer" ); }

    ///We are overriding the lfx::core::VectorRenderer method and then calling it
    ///once we have given it all of the data it needs.
    virtual osg::Node* getSceneGraph( const lfx::core::ChannelDataPtr maskIn );

	virtual void dumpState( std::ostream &os );

protected:

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );
};

typedef boost::shared_ptr< VTKActorRenderer > VTKActorRendererPtr;

}
}
}

#endif
