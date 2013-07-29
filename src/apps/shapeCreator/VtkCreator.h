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

#include "CreateVolume.h"


#ifdef VTK_FOUND

#ifndef Q_MOC_RUN
#  include <latticefx/core/vtk/DataSet.h>
#  include <latticefx/core/vtk/VTKVolumeBrickData.h>
#endif

using namespace lfx::core;

class VtkCreator : public CreateVolume
{
public:
	VtkCreator( const char *plogstr, const char *ploginfo );

	virtual bool create();
	virtual bool create( osg::ArgumentParser &arguments, const std::string &csFile );

    void init( const char *vtkFile );
    bool haveFile();

    int getNumScalars();
    int getNumVectors();
    std::string getScalarName( int num );
    std::string getVectorName( int num );

	void setThreads( int count ) { _threads = count; } 
	void setHiresLod( bool on ) { _hireslod = on; }
	void setCache( bool on ) { _nocache = !on; }
	void setScalarsToProcess( const std::vector<int> &indexs ) { _scalars = indexs; }
	void setVectorsToProcess( const std::vector<int> &indexs ) { _vectors = indexs; }


protected:

	virtual bool processArgs( osg::ArgumentParser &arguments );

	void getVtkProcessOptions( osg::ArgumentParser &arguments, int totalItems, std::vector<int> *pItems, bool scalar );
	bool vtkCreateBricks( SaveHierarchy::LODVector &depths, const std::string csFile, int item, bool scalar );

	bool createDataSet( const std::string& csFile, SaveHierarchy::LODVector &depths, const std::string &baseName );
	bool createDataSet( const std::string& csFile, VolumeBrickDataPtr brickData, const std::string &baseName );

	void setCacheCreate( SaveHierarchy::LODVector &depths, bool create );
	void setCacheUse( SaveHierarchy::LODVector &depths, bool use );

	void getLod( SaveHierarchy::LODVector* pdepths, VolumeBrickDataPtr hilod, vtk::DataSetPtr ds, int threads, bool hireslod, bool prune, bool resPrune );

protected:

    vtk::DataSetPtr _pds;
    int _threads;
	bool _hireslod;
	bool _nocache;
	std::vector<int> _scalars;
	std::vector<int> _vectors;
};

// VTK_FOUND
#endif

// __VTK_CREATOR_H__
#endif
