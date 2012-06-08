/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
#pragma once
//This class wraps the OSG Vertex Shader based rendering
//the Testure2D data is used as away to transfer data into the shader
//It acts as a raw array instead of 2D data

#include <osg/Geometry>
#include <osg/Geode>

#include <vtkPointData.h>
#include <vtkPoints.h>

#include <string>

class vtkPolyData;

class OSGVectorStage
{
public:
    OSGVectorStage(void);
    ~OSGVectorStage(void);

    //create a osgNode
    osg::Geode* createInstanced( vtkPolyData* glyph, std::string vectorName="GlyphVector", std::string scalarName="", float scale1=1.0 );

private:

    //an m x n texture is used to transfer data
    //int tm; 
    //int tn;

    //two utility functions used to determine tm and tn, since texture dimension needs to be 2^n
    //int mylog2(unsigned x);
    //int mypow2(unsigned x);

    //create the glyph arrow in OSG
    void createArrow( osg::Geometry& geom, int nInstances=1, float scale2=1.0 );

    //create the position array based on the passed in VTK points
    //float* createPositionArray( int m, int n , vtkPoints* points);
    
    //create the attitude (rotation) array based on the passed VTK dataArray (dataArray should have 3 components per tuple)
    //float* createAttitudeArray( int m, int n, vtkDataArray* dataArray);

    //create the scalar data array based on the passed VTK dataArray (dataArray should have 1 components per tuple)
    //float* createScalarArray( int m, int n, vtkDataArray* dataArray);

};
