
#include <latticefx/DataSet.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/Renderer.h>
#include <latticefx/VectorRenderer.h>
#include <latticefx/PlayControl.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <iostream>



lfx::DataSetPtr prepareDataSet()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    const unsigned int w( 73 ), h( 41 ), d( 11 );
    std::cout << "Creating data set. Dimentions: " << w << " x " << h << " x " << d << std::endl;

    unsigned int samplesPerTime( w*h*d );
    vertArray->resize( samplesPerTime );
    unsigned int wIdx, hIdx, dIdx, index( 0 );
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const float x( ((double)wIdx)/(w-1.) * (double)w - (w*.5) );
                const float y( ((double)hIdx)/(h-1.) * (double)h - (h*.5) );
                const float z( ((double)dIdx)/(d-1.) * (double)d - (d*.5) );
                (*vertArray)[ index ].set( x, y, z );
                ++index;
            }
        }
    }

    lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( vertData );

    const double maxTime( 8. );
    const double sampleRate( 60. );
    std::cout << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz." << std::endl;

    int count( 0 );
    double time;
    for( time=0.; time<maxTime; time += 1./sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
        dirArray->resize( w*h*d );
        index = 0;
        for( wIdx=0; wIdx<w; ++wIdx )
        {
            for( hIdx=0; hIdx<h; ++hIdx )
            {
                for( dIdx=0; dIdx<d; ++dIdx )
                {
                    const float x( ((double)wIdx)/(w-1.) * (double)w - (w*.5) );
                    const float y( ((double)hIdx)/(h-1.) * (double)h - (h*.5) );
                    (*dirArray)[ index ].set( sin( x+y+time ), sin( (x+y+time)*1.2 ), .5 );
                    ++index;
                    ++count;
                }
            }
        }
        lfx::ChannelDataOSGArrayPtr dirData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) ) );
        dsp->addChannel( dirData, time );
    }
    std::cout << "Total samples: " << count << std::endl;

    lfx::RendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->addInput( "positions" );
    renderOp->addInput( "directions" );
    dsp->setRenderer( renderOp );

    return( dsp );
}



int main( int argc, char** argv )
{
    // Create an example data set.
    lfx::DataSetPtr dsp( prepareDataSet() );

    lfx::PlayControlPtr playControl( new lfx::PlayControl( dsp->getSceneData() ) );
    playControl->setTimeRange( dsp->getTimeRange() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.setSceneData( dsp->getSceneData() );

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
