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

#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PlayControl.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgwTools/Shapes.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <iostream>
#include <fstream>
#include <sstream>


using namespace lfx::core;


const std::string logstr( "lfx.demo" );


class ChannelDataVisualFieldSample : public ChannelData
{
public:
    ChannelDataVisualFieldSample( const std::string& name=std::string( "" ) )
      : ChannelData( name ),
        _timeValid( false )
    {}
    ~ChannelDataVisualFieldSample()
    {}

    double getTime() const
    {
        if( !_timeValid )
        {
            std::istringstream istr( _baseString );
            int year, month, day;
            char hyphen;
            istr >> year >> hyphen >> month >> hyphen >> day;

            // This formula gives us 1 second per month.
            _time = ( (year - 2011) * 12. ) + ( month - 1. ) + ( day / 31. );

            _timeValid = true;
        }
        return( _time );
    }

    std::string _baseString;
    osg::Vec2 _leftScale, _rightScale;
    osg::Vec2 _leftTrans, _rightTrans;

protected:
    mutable bool _timeValid;
    mutable double _time;
};
typedef boost::shared_ptr< ChannelDataVisualFieldSample > ChannelDataVisualFieldSamplePtr;


class VisualFieldRenderer : public Renderer
{
public:
    VisualFieldRenderer()
      : Renderer( "vf" )
    {}
    ~VisualFieldRenderer()
    {}

    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn )
    {
        ChannelDataPtr cdp( getInput( "data" ) );
        ChannelDataVisualFieldSamplePtr cdvf( boost::static_pointer_cast< ChannelDataVisualFieldSample >( cdp ) );

        osg::ref_ptr< osg::Geode > root( new osg::Geode );

        root->addDrawable( createEye( cdvf->_baseString,
            cdvf->_leftScale, cdvf->_leftTrans,
            osg::Vec3( -1., -.5, 0. ), "-left.jpg" ) );
        root->addDrawable( createEye( cdvf->_baseString,
            cdvf->_rightScale, cdvf->_rightTrans,
            osg::Vec3( 0., -.5, 0. ), "-right.jpg" ) );

        return( root.release() );
    }

    virtual osg::StateSet* getRootState()
    {
        osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );
        stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        return( stateSet.release() );
    }

protected:
    osg::Drawable* createEye( const std::string& baseName,
        const osg::Vec2& scale, const osg::Vec2& trans,
        const osg::Vec3& corner, const std::string& suffix )
    {
        osg::ref_ptr< osg::Geometry > geom( osgwTools::makePlane( corner, osg::Vec3( 1., 0., 0. ), osg::Vec3( 0., 1., 0. ) ) );
        {
            osg::Vec2Array* v( new osg::Vec2Array );
            v->resize( 4 );
            osg::Vec2& v0( (*v)[ 0 ] );
            osg::Vec2& v1( (*v)[ 1 ] );
            osg::Vec2& v2( (*v)[ 2 ] );
            osg::Vec2& v3( (*v)[ 3 ] );

            // Set scales
            v1[ 0 ] = scale[ 0 ];
            v2[ 1 ] = scale[ 1 ];
            v3.set( scale[ 0 ], scale[ 1 ] );
            // And now translate
            v0 += trans;
            v1 += trans;
            v2 += trans;
            v3 += trans;

            geom->setTexCoordArray( 0, v );
        }

        osg::StateSet* stateSet( geom->getOrCreateStateSet() );

        const std::string imageName( "vf-" + baseName + suffix );
        osg::Image* image( osgDB::readImageFile( imageName ) );

        osg::Texture2D* tex( new osg::Texture2D( image ) );
        tex->setResizeNonPowerOfTwoHint( false );
        tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );

        stateSet->setTextureAttributeAndModes( 0, tex );

        return( geom.release() );
    }
};
typedef boost::shared_ptr< VisualFieldRenderer > VisualFieldRendererPtr;




DataSetPtr prepareDataSet()
{
    DataSetPtr dsp( new DataSet() );

    std::map< std::string, ChannelDataVisualFieldSamplePtr > cdmap;

    std::ifstream ifs( osgDB::findDataFile( "vf-registration.txt" ) );
    if( ifs.bad() )
    {
        LFX_FATAL_STATIC( logstr, "Can't open \"vf-registration.txt\"." );
        return( DataSetPtr( (DataSet*)NULL ) );
    }
    while( !ifs.eof() )
    {

        std::string name;
        ifs >> name;
        if( name.empty() )
            continue;

        const bool isLeft( name.find( "left" ) != name.npos );

        // Remove leading "vf-"
        std::string::size_type loc( name.find_first_of( '-' ) );
        name = name.substr( loc+1 );
        // Remove trailing "-left" ir "-right"
        loc = name.find_last_of( '-' );
        name = name.substr( 0, loc );

        bool addData( false );
        ChannelDataVisualFieldSamplePtr cdvf;
        if( cdmap.find( name ) == cdmap.end() )
        {
            cdvf = ChannelDataVisualFieldSamplePtr( new ChannelDataVisualFieldSample( "data" ) );
            cdmap[ name ] = cdvf;
            cdvf->_baseString = name;
            addData = true;
        }
        else
            cdvf = cdmap[ name ];

        std::string scratch;
        if( isLeft )
        {
            ifs >> scratch >> cdvf->_leftScale;
            ifs >> scratch >> cdvf->_leftTrans;
        }
        else
        {
            ifs >> scratch >> cdvf->_rightScale;
            ifs >> scratch >> cdvf->_rightTrans;
        }

        if( addData )
            dsp->addChannel( cdvf, cdvf->getTime() );
    }


    VisualFieldRendererPtr renderOp( new VisualFieldRenderer() );
    renderOp->addInput( "data" );

    dsp->setRenderer( renderOp );

    return( dsp );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    LFX_INFO_STATIC( logstr, "Options:" );

    osg::ArgumentParser arguments( &argc, argv );

    // Create an example data set.
    osg::Group* root( new osg::Group );
    DataSetPtr dsp( prepareDataSet() );
    root->addChild( dsp->getSceneData() );

    // Play the time series animation
    PlayControlPtr playControl( new PlayControl( dsp->getSceneData() ) );
    playControl->setTimeRange( dsp->getTimeRange() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.setSceneData( root );

    double prevClockTime( 0. );
    while( !( viewer.done() ) )
    {
        const double clockTime( viewer.getFrameStamp()->getReferenceTime() );
        const double elapsed( clockTime - prevClockTime );
        prevClockTime = clockTime;
        playControl->elapsedClockTick( elapsed );

        viewer.frame();
    }
    return( 0 );
}
