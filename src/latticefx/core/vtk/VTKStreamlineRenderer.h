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

#ifndef __LFX_CORE_VTK_STREAMLINE_RENDERER_H__
#define __LFX_CORE_VTK_STREAMLINE_RENDERER_H__ 1

#include <latticefx/core/StreamlineRenderer.h>
#include <latticefx/core/vtk/IVTKRenderer.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/vtk/Export.h>

#include <vtkType.h>

class vtkPolyData;

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class VTKStreamlineRenderer VTKStreamlineRenderer.h <latticefx/core/vtk/VTKStreamlineRenderer.h>
 \brief This class converts a vtkPolyData to OSG arrays for rendering.
 \details This class goes through a conversion process with vtkPolyData to enable
 the instance rendering tools to render VTK streamlines using GPU based instance
 rendering. */

class LATTICEFX_CORE_VTK_EXPORT VTKStreamlineRenderer : public lfx::core::StreamlineRenderer, public IVTKRenderer
{
public:
    ///Default constructor
    VTKStreamlineRenderer( const std::string& logName = std::string( "" ) )
        :
        lfx::core::StreamlineRenderer( logName ),
        m_pd( 0 ),
		m_refresh( false )
    {
        ;
    }

    ///Destructor
    virtual ~VTKStreamlineRenderer()
    {
        ;
    }

	///Get a string name for this class
	virtual std::string getClassName() const { return std::string( "VTKStreamlineRenderer" ); }

    ///We are overriding the lfx::core::StreamlineRenderer method and then calling it
    ///once we have given it all of the data it needs.
    virtual osg::Node* getSceneGraph( const lfx::core::ChannelDataPtr maskIn );

	virtual void dumpState( std::ostream &os );

	void FullRefresh() { m_refresh = true; }

protected:

	struct Point
    {
        double p[ 3 ];
        vtkIdType vertId; // should be vtkidType instead
    };

	void ExtractVTKPrimitives( vtkPolyData *pd );
	void ProcessStreamLines( vtkPolyData* polydata,  std::vector< std::deque< Point > > &streamlineList );
	bool IsStreamlineBackwards( vtkIdType cellId, vtkPolyData* polydata );
	void CreateStreamLines( vtkPolyData *polyData, const std::vector< std::deque< Point > > &streamlineList);
	void SetupColorArrays( vtkPolyData* pd, const std::vector< std::deque< Point > > &streamlineList );

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

    ///The raw VTK data to render
    vtkPolyData* m_pd;
    ///Scalar channel data
    std::map< std::string, lfx::core::ChannelDataPtr > m_scalarChannels;
    ///The raw dataset object
    lfx::core::vtk::ChannelDatavtkDataObjectPtr m_dataObject;

	bool m_refresh;
};

typedef boost::shared_ptr< VTKStreamlineRenderer > VTKStreamlineRendererPtr;

}
}
}

#endif