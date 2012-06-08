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
#include <latticefx/TextureUtils.h>
#include <latticefx/BoundUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <osgwTools/Shapes.h>

#include <vtk_utils/readWriteVtkThings.h>
#include <vtk_utils/DataSet.h>


class InstancedVectors : public lfx::Renderer
{
public:
    InstancedVectors() : lfx::Renderer()
    {}
    virtual ~InstancedVectors()
    {}

    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn )
    {
        const osg::Array* sourceArray( _inputs[ 0 ]->asOSGArray() );
        const osg::Vec3Array* positions( dynamic_cast< const osg::Vec3Array* >( sourceArray ) );

        osg::ref_ptr< osg::Geode > geode( new osg::Geode );

        osg::Geometry* geom( osgwTools::makeArrow() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        geom->setInitialBound( lfx::getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        const unsigned int numElements( sourceArray->getNumElements() );
        for( unsigned int idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( getInput( "positions" ) ) );
        stateSet->setTextureAttributeAndModes( 0, posTex, osg::StateAttribute::OFF );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( 0 );
        stateSet->addUniform( posUni );

        osg::Texture3D* dirTex( lfx::createTexture3DForInstancedRenderer( getInput( "directions" ) ) );
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
    renderOp->addInput( vertData->getName() );
    renderOp->addInput( dirData->getName() );
    dsp->setRenderer( renderOp );

    return( dsp );
}

void loadDataSet()
{
    lfx::vtk_utils::DataSet* tempDataSet = new lfx::vtk_utils::DataSet();
    tempDataSet->SetFileName( "filename" );
    tempDataSet->SetUUID( "VTK_DATA_FILE", "test" );
    //ves::open::xml::DataValuePairPtr stringDVP =
    //    tempInfoPacket->GetProperty( "VTK_ACTIVE_DATA_ARRAYS" );
    /*std::vector< std::string > vecStringArray;
    if( stringDVP )
    {
        ves::open::xml::OneDStringArrayPtr stringArray =
        boost::dynamic_pointer_cast <
        ves::open::xml::OneDStringArray > (
                                           stringDVP->GetDataXMLObject() );
        vecStringArray = stringArray->GetArray();
        tempDataSet->SetActiveDataArrays( vecStringArray );
    }*/
            const std::string tempDataSetFilename =
        tempDataSet->GetFileName();
        std::cout << "|\tLoading data for file "
        << tempDataSetFilename
        << std::endl;
        //tempDataSet->SetArrow(
        //                        ves::xplorer::ModelHandler::instance()->GetArrow() );
        //Check and see if the data is part of a transient series
        /*if( tempInfoPacket->GetProperty( "VTK_TRANSIENT_SERIES" ) )
        {
            std::string precomputedSurfaceDir =
            tempInfoPacket->GetProperty( "VTK_TRANSIENT_SERIES" )->
            GetDataString();
            lastDataAdded->LoadTransientData( precomputedSurfaceDir );
        }
        else*/
        {
            tempDataSet->LoadData();
        }
        //If the data load failed
        /*if( !tempDataSet->GetDataSet() )
        {
            std::cout << "|\tData failed to load." << std::endl;
            //_activeModel->DeleteDataSet( tempDataSetFilename );
        }
        else
        {
            std::cout << "|\tData is loaded for file "
            << tempDataSetFilename
            << std::endl;
            if( lastDataAdded->GetParent() == lastDataAdded )
            {
                _activeModel->GetDCS()->
                AddChild( lastDataAdded->GetDCS() );
                _activeModel->SetActiveDataSet( 0 );
            }
            m_datafileLoaded( tempDataSetFilename );
        }*/
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
