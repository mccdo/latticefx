/*************** <auto-copyright.rb BEGIN do not edit this line> *************
 *
 * latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; 
 * version 2.1 of the License.
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
 * -----------------------------------------------------------------
 * Date modified: $Date$
 * Version:       $Rev$
 * Author:        $Author$
 * Id:            $Id$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> **************/
#include <latticefx/core/vtk/VTKPrimitiveSetGenerator.h>

#include <vtkCellArray.h>

#include <osg/Geometry>

namespace lfx {

namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
VTKPrimitiveSetGenerator::VTKPrimitiveSetGenerator( const VTKPrimitiveSetGenerator& rhs )
    : 
    lfx::core::PrimitiveSetGenerator( rhs )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
void VTKPrimitiveSetGenerator::operator()( const SurfaceRenderer* /* surfaceRenderer */, osg::Geometry* geom )
{
    //int numStrips = polydata->GetNumberOfStrips();
    vtkIdType cStripNp;
    vtkIdType* pts;
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
    }
    //osg::DrawArrays* da( new osg::DrawArrays( GL_TRIANGLES, 0,
    //                                         geom->getVertexArray()->getNumElements() ) );
    //geom->addPrimitiveSet( da );
}
////////////////////////////////////////////////////////////////////////////////
}
}
}
