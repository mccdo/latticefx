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
#include <latticefx/DataSet.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/RTPOperation.h>
#include <latticefx/Renderer.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osgViewer/Viewer>


/** \brief Example of a run time operation to mask off part of the data.
\details The data is a plot of -( x*x + y*y ). This mask operation
removes all data points of the graph that satisfy y + z > 0.
*/
class MyMask : public lfx::RTPOperation
{
public:
    MyMask()
      : lfx::RTPOperation( lfx::RTPOperation::Mask )
    {}
    virtual ~MyMask()
    {}

    virtual lfx::ChannelDataPtr mask( const lfx::ChannelDataPtr maskIn )
    {
        lfx::ChannelDataPtr input = getInput( "vertices" );
        if( ( input == NULL ) )
        {
            osg::notify( osg::WARN ) << "MyMask::mask(): Invalid input." << std::endl;
            return( lfx::ChannelDataPtr( ( lfx::ChannelData* )( NULL ) ) );
        }

        // Get the threshold test value, configurable from the calling code using
        // setValue( "threshold", lfx::OperationValue( floatVal ) );
        float threshold( 0.f );
        if( hasValue( "threshold" ) )
            threshold = getValue( "threshold" )->getFloat();

        osg::Vec3Array* xyz = static_cast< osg::Vec3Array* >( input->asOSGArray() );
        unsigned int size( xyz->getNumElements() );

        osg::ref_ptr< osg::ByteArray > maskData( new osg::ByteArray );
        maskData->resize( size );

        osg::ByteArray* maskInData = static_cast< osg::ByteArray* >( maskIn->asOSGArray() );
        signed char* maskInPtr = &( (*maskInData)[ 0 ] );
        unsigned int idx;
        for( idx=0; idx<size; ++idx, ++maskInPtr )
        {
            if( *maskInPtr == 0 )
                continue;

            signed char& maskValue( (*maskData)[ idx ] );
            const osg::Vec3& v( (*xyz)[ idx ] );
            maskValue = ( v.z() + v.y() > threshold ) ? 0 : 1;
        }

        lfx::ChannelDataOSGArrayPtr cdp( new lfx::ChannelDataOSGArray( maskData.get() ) );
        return( cdp );
    }

protected:
};

class MyRenderer : public lfx::Renderer
{
public:
    MyRenderer() : lfx::Renderer()
    {}
    virtual ~MyRenderer()
    {}

    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn )
    {
        osg::ref_ptr< osg::Geode > geode( new osg::Geode );
        geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        osg::Geometry* geom = new osg::Geometry;
        lfx::ChannelDataPtr input( getInput( "vertices" )->getMaskedChannel( maskIn ) );
        geom->setVertexArray( input->asOSGArray() );

        unsigned int idx, size = geom->getVertexArray()->getNumElements();
        osg::DrawElementsUInt* deui( new osg::DrawElementsUInt( GL_POINTS, size ) );
        for( idx=0; idx<size; idx++ )
            (*deui)[ idx ] = idx;
        geom->addPrimitiveSet( deui );
        geode->addDrawable( geom );

        return( geode.release() );
    }

protected:
};

lfx::DataSetPtr prepareDataSet()
{
    // Create a vertex array for the graph - (x*x + y*y), with the plot
    // space ranging from -1 to 1 in both x and y.
    osg::ref_ptr< osg::Vec3Array > xyzData( new osg::Vec3Array );
    const unsigned int w( 400 ), h( 400 );
    xyzData->resize( w*h );
    unsigned int wIdx, hIdx;
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            const float x( ((float)wIdx)/w * 2. - 1. );
            const float y( ((float)hIdx)/h * 2. - 1. );
            (*xyzData)[ (wIdx*w) + hIdx ].set(
                x, y, -( x*x + y*y ) );
        }
    }
    lfx::ChannelDataOSGArrayPtr cdp( new lfx::ChannelDataOSGArray( xyzData.get(), "vertices" ) );

    // Create a data set and add the vertex data.
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( cdp );

    // Create a mask operation and add it to the data set.
    lfx::RTPOperationPtr maskOp( new MyMask() );
    maskOp->setValue( "threshold", lfx::OperationValue( -0.1f ) );
    maskOp->addInput( cdp->getName() );
    //maskOp->setEnable( false ); // Optionally disable the mask operation.
    dsp->addOperation( maskOp );

    lfx::RendererPtr renderOp( new MyRenderer() );
    renderOp->addInput( cdp->getName() );
    dsp->setRenderer( renderOp );

    return( dsp );
}



int main( int argc, char** argv )
{
    // Create an example data set.
    lfx::DataSetPtr dsp( prepareDataSet() );

    osgViewer::Viewer viewer;
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( dsp->getSceneData() );
    return( viewer.run() );
}
