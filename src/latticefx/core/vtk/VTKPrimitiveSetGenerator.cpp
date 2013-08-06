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
#include <latticefx/core/vtk/VTKPrimitiveSetGenerator.h>

#include <osg/Geometry>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
VTKPrimitiveSetGenerator::VTKPrimitiveSetGenerator( const VTKPrimitiveSetGenerator& rhs )
    :
    lfx::core::PrimitiveSetGenerator( rhs )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void VTKPrimitiveSetGenerator::operator()( const SurfaceRenderer*, osg::Geometry* geom )
{
    if( !m_primitives.empty() )
    {
        for( std::vector< osg::ref_ptr< osg::DrawElementsUInt > >::const_iterator
            iter = m_primitives.begin(); iter != m_primitives.end(); ++iter )
        {
            geom->addPrimitiveSet( iter->get() );
        }

        return;
    }

    vtkIdType cStripNp = 0;
    vtkIdType* pts = 0;
    int stripNum = 0;
    int startVertexIdx = 0;

    for( m_triStrips->InitTraversal();  m_triStrips->GetNextCell( cStripNp, pts ); ++stripNum )
    {
        if( cStripNp <= 0 )
        {
            continue;
        }

        osg::ref_ptr< osg::DrawElementsUInt > deui = new osg::DrawElementsUInt( GL_TRIANGLE_STRIP, 0 );

        for( int i = 0; i < cStripNp; ++i )
        {
            deui->push_back( ( unsigned int )( startVertexIdx + i ) );
        }

        startVertexIdx += cStripNp;
        geom->addPrimitiveSet( deui.get() );
        m_primitives.push_back( deui.get() );
    }
}

////////////////////////////////////////////////////////////////////////////////
void VTKPrimitiveSetGenerator::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	PrimitiveSetGenerator::serializeData( json );

	json->insertObj( VTKPrimitiveSetGenerator::getClassName(), true);
	// store any class specific data here
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKPrimitiveSetGenerator::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !PrimitiveSetGenerator::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKPrimitiveSetGenerator::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKPrimitiveSetGenerator data";
		return false;
	}

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
}
}
}
