#include <latticefx/DataSet.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/RTPOperation.h>
#include <latticefx/Renderer.h>
#include <latticefx/TextureUtils.h>
#include <latticefx/MaskUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <osgwTools/Shapes.h>



class InstancedVectors : public lfx::Renderer
{
public:
    InstancedVectors() : lfx::Renderer()
    {}
    virtual ~InstancedVectors()
    {}

    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn )
    {
        osg::ref_ptr< osg::Geode > geode( new osg::Geode );

        osg::Geometry* geom( osgwTools::makeArrow() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        geom->setInitialBound( osg::BoundingBox( -2.5, -1., -1., 2.5, 1., 2. ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        const osg::Array* sourceArray( _inputs[ 0 ]->asOSGArray() );
        const unsigned int numElements( sourceArray->getNumElements() );
        unsigned int idx;
        for( idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( _inputs[ 0 ] ) );
        stateSet->setTextureAttributeAndModes( 0, posTex, osg::StateAttribute::OFF );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( 0 );
        stateSet->addUniform( posUni );

        osg::Texture3D* dirTex( lfx::createTexture3DForInstancedRenderer( _inputs[ 1 ] ) );
        stateSet->setTextureAttributeAndModes( 1, dirTex, osg::StateAttribute::OFF );
        osg::Uniform* dirUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texDir" ) ); dirUni->set( 1 );
        stateSet->addUniform( dirUni );

        const osg::Vec3f dimensions( lfx::computeTexture3DDimensions( numElements ) );
        osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
        stateSet->addUniform( texDim );

        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
        vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.vs" ) );
        program->addShader( vertexShader );
        osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
        fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.fs" ) );
        program->addShader( fragmentShader );

        return( geode.release() );
    }

protected:
};

lfx::DataSetPtr prepareDataSet()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
    const unsigned int w( 4 ), h( 1 ), d( 1 );
    vertArray->resize( w*h*d );
    dirArray->resize( w*h*d );
    unsigned int wIdx, hIdx, dIdx, index( 0 );
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const float x( ((double)wIdx)/(w-1.) * 4. - 2. );
                const float y( 0. );
                const float z( 0. );
                (*vertArray)[ index ].set( x, y, z );
                (*dirArray)[ index ].set( sin(x), .5, .5 );
                ++index;
            }
        }
    }
    lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    lfx::ChannelDataOSGArrayPtr dirData( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) );

    // Create a data set and add the vertex and direction data.
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( vertData );
    dsp->addChannel( dirData );

    lfx::RendererPtr renderOp( new InstancedVectors() );
    renderOp->addInput( vertData );
    renderOp->addInput( dirData );
    dsp->setRenderer( renderOp );

    return( dsp );
}



int main( int argc, char** argv )
{
    // Create an example data set.
    lfx::DataSetPtr dsp( prepareDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( dsp->getSceneData() );
    return( viewer.run() );
}
