
#include <latticefx/DataSet.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/Renderer.h>
#include <latticefx/VectorRenderer.h>
#include <latticefx/PlayControl.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <iostream>


unsigned int computeDynamicPositions( osg::Vec3Array* a,
        const unsigned int w, const unsigned int h, const unsigned int d, const double t )
{
    a->resize( w*h*d );
    unsigned int index( 0 );
    unsigned int wIdx, hIdx, dIdx;
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const double x( ((double)wIdx)/(w-1.) * (double)w - (w*.5) );
                const double y( ((double)hIdx)/(h-1.) * (double)h - (h*.5) );
                const double z( ((double)dIdx)/(d-1.) * (double)d - (d*.5) );
                (*a)[ index ].set( x + sin( (x+y+t)*.8 ), y + sin( (x+y+t) ), z + sin( (x+y+t)*1.2 ) );
                ++index;
            }
        }
    }
    return( index );
}

lfx::DataSetPtr prepareSimplePoints()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    const unsigned int w( 73 ), h( 41 ), d( 11 );
    std::cout << "Creating data set. Dimensions: " << w << " x " << h << " x " << d << std::endl;

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    const double maxTime( 8. );
    const double sampleRate( 60. );
    std::cout << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz." << std::endl;

    unsigned int totalSamples( 0 );
    double time;
    for( time=0.; time<maxTime; time += 1./sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array );
        unsigned int count( computeDynamicPositions( posArray.get(), w, h, d, time ) );
        totalSamples += count;
        lfx::ChannelDataOSGArrayPtr posData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( posArray.get(), "positions" ) ) );
        dsp->addChannel( posData, time );
    }
    std::cout << "Total samples: " << totalSamples << std::endl;

    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::SIMPLE_POINTS );
    renderOp->addInput( "positions" );
    dsp->setRenderer( renderOp );

    return( dsp );
}
lfx::DataSetPtr preparePointSprites()
{
    lfx::DataSetPtr dsp( (lfx::DataSet*) NULL );
    return( dsp );
}
lfx::DataSetPtr prepareSpheres()
{
    const unsigned int w( 15 ), h( 12 ), d( 9 );
    std::cout << "Creating data set. Dimensions: " << w << " x " << h << " x " << d << std::endl;
    const unsigned int samplesPerTime( w*h*d );

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    unsigned int totalSamples( 0 );

    const double maxTime( 8. );
    const double sampleRate( 30. );
    std::cout << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz." << std::endl;
    double time;
    for( time=0.; time<maxTime; time += 1./sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array );
        unsigned int count( computeDynamicPositions( posArray.get(), w, h, d, time ) );
        totalSamples += count;
        lfx::ChannelDataOSGArrayPtr posData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( posArray.get(), "positions" ) ) );
        dsp->addChannel( posData, time );

        // Array of radius values.
        osg::ref_ptr< osg::FloatArray > radArray( new osg::FloatArray );
        radArray->resize( samplesPerTime );
        unsigned int wIdx, hIdx, dIdx, index( 0 );
        for( wIdx=0; wIdx<w; ++wIdx )
        {
            for( hIdx=0; hIdx<h; ++hIdx )
            {
                for( dIdx=0; dIdx<d; ++dIdx )
                {
                    const double x( ((double)wIdx)/(w-1.) );
                    const double y( ((double)hIdx)/(h-1.) );
                    const double rad( osg::absolute( sin( x+y+time ) ) ); 
                    (*radArray)[ index ] = rad * .33;
                    ++index;
                }
            }
        }
        lfx::ChannelDataOSGArrayPtr radData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( radArray.get(), "radii" ) ) );
        dsp->addChannel( radData, time );
    }
    std::cout << "Total samples: " << totalSamples << std::endl;

    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::SPHERES );
    renderOp->addInput( "positions" );
    renderOp->addInput( "radii" );
    dsp->setRenderer( renderOp );

    return( dsp );
}
lfx::DataSetPtr prepareDirectionVectors()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    const unsigned int w( 73 ), h( 41 ), d( 11 );
    std::cout << "Creating data set. Dimensions: " << w << " x " << h << " x " << d << std::endl;

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

    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::DIRECTION_VECTORS );
    renderOp->addInput( "positions" );
    renderOp->addInput( "directions" );
    dsp->setRenderer( renderOp );

    return( dsp );
}

lfx::DataSetPtr prepareDataSet( const lfx::VectorRenderer::PointStyle& style )
{
    switch( style )
    {
    case lfx::VectorRenderer::POINT_SPRITES:
        std::cout << "point sprites not yet implemented." << std::endl;
    default:
    case lfx::VectorRenderer::SIMPLE_POINTS:
        return( prepareSimplePoints() );
//    case lfx::VectorRenderer::POINT_SPRITES:
//        return( preparePointSprites() );
    case lfx::VectorRenderer::SPHERES:
        return( prepareSpheres() );
    case lfx::VectorRenderer::DIRECTION_VECTORS:
        return( prepareDirectionVectors() );
    }
}


int main( int argc, char** argv )
{
    std::cout << "With no options, render as simple points." << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t-ps\tRender as point sprites." << std::endl;
    std::cout << "\t-s\tRender as spheres." << std::endl;
    std::cout << "\t-d\tRender as direction vectors." << std::endl << std::endl;
    osg::ArgumentParser arguments( &argc, argv );
    lfx::VectorRenderer::PointStyle style( lfx::VectorRenderer::SIMPLE_POINTS );
    if( arguments.find( "-ps" ) > 0 ) style = lfx::VectorRenderer::POINT_SPRITES;
    if( arguments.find( "-s" ) > 0 ) style = lfx::VectorRenderer::SPHERES;
    if( arguments.find( "-d" ) > 0 ) style = lfx::VectorRenderer::DIRECTION_VECTORS;

    // Create an example data set.
    lfx::DataSetPtr dsp( prepareDataSet( style ) );

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
