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
//C++ header - fIVE|Analyse - Copyright (C) 2002-2003 Michael Gronager, UNI-C
//Distributed under the terms of the GNU Library General Public License (LGPL)
//as published by the Free Software Foundation.

#ifndef VTKACTORTOOSG_H
#define VTKACTORTOOSG_H
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ref_ptr>

#include <vtkActor.h>
#include <vtkCellArray.h>

namespace lfx
{
namespace core
{
namespace vtk
{
// vtkActorToOSG - translates vtkActor to osg::Geode. If geode is NULL, new one
//   will be created. Optional verbose parameter prints debugging and
//   performance information.
osg::Geode* vtkActorToOSG( vtkActor* actor, osg::Geode* geode = NULL, int verbose = 0 );

osg::Geometry* processPrimitive( vtkActor* a, vtkCellArray* prims, int pType, int v );
}
}
}
#endif
