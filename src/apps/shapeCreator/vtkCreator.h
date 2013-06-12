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
#ifndef __VTK_CREATOR_H__
#define __VTK_CREATOR_H__

#include "shapeCreatorDefines.h"

#ifdef VTK_FOUND

#  include <latticefx/core/vtk/DataSet.h>
#  include <latticefx/core/vtk/VTKVolumeBrickData.h>

using namespace lfx::core;

class VtkCreator
{
public:
	VtkCreator(const char *plogstr, const char *ploginfo);
	int create(osg::ArgumentParser &arguments, const std::string &csFile);

    void init(const char *vtkFile);
    int getScalars();
    int getVectors();

protected:
	void getVtkProcessOptions(osg::ArgumentParser &arguments, int totalItems, std::vector<int> *pItems, bool scalar);
	void vtkCreateBricks(SaveHierarchy::LODVector &depths, const std::string csFile, int item, bool scalar);

	void createDataSet( const std::string& csFile, SaveHierarchy::LODVector &depths, const std::string &baseName );
	void createDataSet( const std::string& csFile, VolumeBrickDataPtr brickData, const std::string &baseName );

	void setCacheCreate(SaveHierarchy::LODVector &depths, bool create);
	void setCacheUse(SaveHierarchy::LODVector &depths, bool use);

	void getLod(SaveHierarchy::LODVector* pdepths, VolumeBrickDataPtr hilod, vtk::DataSetPtr ds, int threads, bool hireslod, bool prune);

protected:
	std::string logstr;
	std::string loginfo;

    vtk::DataSetPtr m_pds;
    int m_depth;
    int m_threads;
};

// VTK_FOUND
#endif

// __VTK_CREATOR_H__
#endif
