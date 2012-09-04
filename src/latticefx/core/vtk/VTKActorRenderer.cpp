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
// --- latticeFX Includes --- //
#include <latticefx/core/vtk/VTKActorRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>
#include <latticefx/core/vtk/vtkActorToOSG.h>

#include <latticefx/core/ChannelDataOSGArray.h>

// --- VTK Includes --- //
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

// --- OSG Includes --- //
#include <osg/Geode>
#include <osg/Node>
#include <osg/LightModel>
#include <osg/CopyOp>
#include <osgUtil/Optimizer>

namespace lfx {
    
namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
void VTKActorRenderer::SetActiveVector( const std::string& activeVector )
{
    m_activeVector = activeVector;
}
////////////////////////////////////////////////////////////////////////////////
void VTKActorRenderer::SetActiveScalar( const std::string& activeScalar )
{
    m_activeScalar = activeScalar;
}
////////////////////////////////////////////////////////////////////////////////
osg::Node* VTKActorRenderer::getSceneGraph( const lfx::core::ChannelDataPtr maskIn )
{
    vtkPolyDataMapper* tempVtkPD = 
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyDataMapper >( getInput( "vtkPolyDataMapper" ) )->GetPolyDataMapper();
    
    //Setup the vtkActor and Mapper for the vtkActorToOSG utility
    double definedRange[ 2 ] = { 0.1, 1.0 };
    vtkLookupTable* lut = vtkLookupTable::New();
    //Grey scale
    /*lut->SetNumberOfColors( 402 );
    lut->SetHueRange( 0.0f , 0.0f );
    lut->SetSaturationRange( 0.0f , 0.0f );
    lut->SetValueRange( 0.2f , 1.0f );
    lut->SetTableRange( definedRange );
    lut->Build();*/

    //http://www.ncsu.edu/scivis/lessons/colormodels/color_models2.html#hue.
    lut->SetNumberOfColors( 256 );            //default is 256
    lut->SetHueRange( 2.0f / 3.0f, 0.0f );    //a blue-to-red scale
    lut->SetSaturationRange( 1.0f , 1.0f );
    lut->SetValueRange( 1.0f , 1.0f );
    lut->SetTableRange( definedRange );
    lut->ForceBuild();
    
    vtkPolyDataMapper* mapper = tempVtkPD;
    mapper->SelectColorArray( m_activeScalar.c_str() );
    mapper->SetLookupTable( lut );
    //mapper->Update();

    vtkActor* actor = vtkActor::New();
    actor->SetMapper( mapper );
    actor->GetProperty()->SetSpecularPower( 20.0f );
    
    lut->Delete();
    //Complete
    
    osg::ref_ptr< osg::Geode > tempGeode = vtkActorToOSG( actor, 0, 0 );
    actor->Delete();

    osg::ref_ptr< osg::LightModel > lightModel = new osg::LightModel();
    lightModel->setTwoSided( true );
    tempGeode->getOrCreateStateSet()->setAttributeAndModes(
        lightModel.get(), osg::StateAttribute::ON );
    osgUtil::Optimizer geodeOpti;
    geodeOpti.optimize( tempGeode.get(),
                       //osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS |
                       //osgUtil::Optimizer::REMOVE_REDUNDANT_NODES |
                       //osgUtil::Optimizer::REMOVE_LOADED_PROXY_NODES |
                       //osgUtil::Optimizer::COMBINE_ADJACENT_LODS |
                       //osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
                       osgUtil::Optimizer::MERGE_GEOMETRY |
                       osgUtil::Optimizer::CHECK_GEOMETRY |
                       //osgUtil::Optimizer::SPATIALIZE_GROUPS |
                       osgUtil::Optimizer::TRISTRIP_GEOMETRY |
                       //osgUtil::Optimizer::OPTIMIZE_TEXTURE_SETTINGS |
                       osgUtil::Optimizer::MERGE_GEODES );
                       //osgUtil::Optimizer::STATIC_OBJECT_DETECTION );
    
    
    return( tempGeode.release() );
}
////////////////////////////////////////////////////////////////////////////////    
}
}
}

