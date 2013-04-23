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
#ifdef VTK_FOUND

#include "vtkCreator.h"
#include "shapeCreator.h"

#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/ArgumentParser>
#include <osgDB/FileUtils>
#include <osg/io_utils>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

VtkCreator::VtkCreator(const char *plogstr, const char *ploginfo)
{
	logstr = plogstr;
	loginfo = ploginfo;
}

////////////////////////////////////////////////////////////////////////////////
int VtkCreator::create(osg::ArgumentParser &arguments, const std::string &csFile)
{
	std::ostringstream ss;

	// get the vtk file
	int iarg = arguments.find( "-vtk" );
	if (iarg < 0 || arguments.argc() <= (iarg+1))
    {
		LFX_CRITICAL_STATIC( logstr, "You must specify a vtk data file." );
        return 1;
	}
	iarg++;

	std::string vtkFile = osgDB::findDataFile(arguments[iarg]);
	if (!boost::filesystem::exists(vtkFile))
	{
		LFX_CRITICAL_STATIC( logstr, "Vtk data file could not be found." );
        return 1;
	}

	vtk::DataSetPtr ds( new vtk::DataSet() );
    ds->SetFileName(vtkFile);
    ds->LoadData();

    int depth( 4 );
    arguments.read( "-depth", depth );
    if( depth < 1 ) depth = 1;

    // get the threads
	int threads( 32 );
	arguments.read("-threads", threads );
	if (threads <= 0) 
	{
		threads = 32;
		LFX_CRITICAL_STATIC( logstr, "Invalid number of threads using restoring default" );
	}

	ss << "Number of threads: " << threads;
    LFX_CRITICAL_STATIC( loginfo, ss.str().c_str() );
	ss.str( std::string() );
	ss.clear();

	bool hireslod = false;
	if (arguments.find( "-hireslod" ) > 0) { hireslod = true; }



	// get scalars and vectors from commandf line
	int countScl = ds->GetNumberOfScalars();
	int countVec = ds->GetNumberOfVectors();
	std::vector<int> scalars, vectors;

	// find which scalars and which vectors to process
	getVtkProcessOptions(arguments, countScl, &scalars, true);
	getVtkProcessOptions(arguments, countVec, &vectors, false);

	// no items were specified so process all
	if (scalars.size() == 0 && vectors.size() == 0)
	{
		for (int i=0; i<countScl; i++)
		{
			scalars.push_back(i);
		}
		for (int i=0; i<countVec; i++)
		{
			vectors.push_back(i);
		}
	}

	boost::posix_time::ptime start_time( boost::posix_time::microsec_clock::local_time() );

	//vtk::VTKVolumeBrickDataPtr vbd(new vtk::VTKVolumeBrickData(ds, true, 0, true, osg::Vec3s(32,32,32), threads));
	//vtk::VTKVolumeBrickDataPtr vbd(new vtk::VTKVolumeBrickData(ds, true, 0, true, osg::Vec3s(8,8,8), threads));
	VolumeBrickDataPtr vbd(new vtk::VTKVolumeBrickData(ds, true, 0, true, osg::Vec3s(32,32,32), threads));
    vbd->setDepth( depth );

	// get our depths
	SaveHierarchy::LODVector depths;
	getLod( &depths, vbd, ds, threads, hireslod );


	if (arguments.find("-nocache") >= 0)
	{
		LFX_CRITICAL_STATIC( loginfo, "Vtk lookup cache is disabled." );
		setCacheCreate(depths, false);
		setCacheUse(depths, false);
	}
	else
	{
		LFX_CRITICAL_STATIC( loginfo, "Vtk lookup cache is enabled." );
		setCacheCreate(depths, true);
		setCacheUse(depths, true);
	}
	LFX_CRITICAL_STATIC( loginfo, "" );

#if 0
	// debug the cache
	scalars.clear();
	scalars.push_back(0);
	scalars.push_back(0);
	// now lets create all scalar and vector bricks
	for (unsigned int i=0; i<scalars.size(); i++)
	{
		std::stringstream ss;
		ss << "D:/skewmatrix/latticefx/bld/src/apps/shapeCreator/log_s" << i << ".txt";
		vbd->debugLogOpen(ss.str().c_str());

		vtkCreateBricks(vbd, csFile, scalars[i], true);
		vbd->cacheCreate(false);
	}
#endif

	for (unsigned int i=0; i<scalars.size(); i++)
	{
		vtkCreateBricks(depths, csFile, scalars[i], true);
		if (!i) { setCacheCreate(depths, false); }
	}

	for (unsigned int i=0; i<vectors.size(); i++)
	{
		vtkCreateBricks(depths, csFile, vectors[i], false);
		if (!i) { setCacheCreate(depths, false); }
	}

	boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

	ss << "Total time to process dataset = " << createTime << " secs" << std::endl;
    LFX_CRITICAL_STATIC( loginfo, ss.str().c_str() );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::setCacheCreate(SaveHierarchy::LODVector &depths, bool create)
{
	for (unsigned int i=0; i<depths.size(); i++)
	{
		((vtk::VTKVolumeBrickData *)depths[i].get())->cacheCreate(create);
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::setCacheUse(SaveHierarchy::LODVector &depths, bool use)
{
	for (unsigned int i=0; i<depths.size(); i++)
	{
		((vtk::VTKVolumeBrickData *)depths[i].get())->cacheUse(use);
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::getLod(SaveHierarchy::LODVector* pdepths, VolumeBrickDataPtr hilod, vtk::DataSetPtr ds, int threads, bool hireslod)
{
	pdepths->clear();
	if (!hireslod)
	{
		pdepths->push_back(hilod);
		return;
	}

	int depth = hilod->getDepth();
	if (depth == 1) // only a single depth, not likely
	{
		pdepths->push_back(hilod);
		return;
	}

	pdepths->resize(depth);
	(*pdepths)[depth-1] = hilod;

	for (int i=0; i<depth-1; i++)
	{
		VolumeBrickDataPtr vbd( new vtk::VTKVolumeBrickData(ds, true, 0, true, osg::Vec3s(32,32,32), threads) );
		vbd->setDepth( i+1 );
		(*pdepths)[i] = vbd;
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::getVtkProcessOptions(osg::ArgumentParser &arguments, int totalItems, std::vector<int> *pItems, bool scalar)
{
	int iarg = 0;
	std::string cmd = "-s";
	if (!scalar) cmd = "-v";

	pItems->clear();

	// find which scalars or vectors
	
	// do they want all items
	iarg = arguments.find(cmd);
	if (iarg >= 0 && iarg < arguments.argc())
	{
		for (int i=0; i< totalItems; i++)
		{
			pItems->push_back(i);
		}

		return;
	}

	
	// look for individual scalars
	for (int i=0; i<totalItems; i++)
	{
		std::ostringstream ss;
		ss << cmd << i;

		iarg = arguments.find(ss.str());
		if (iarg >= 0 && iarg < arguments.argc())
		{
			//pItems->insert(pItems->begin(), i);
			pItems->push_back(i);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::vtkCreateBricks(SaveHierarchy::LODVector &depths, const std::string csFile, int item, bool scalar)
{
	std::ostringstream sst;
	if (scalar)
	{
		sst << "shapevolume_s" << item;
	}
	else
	{
		sst << "shapevolume_v" << item;
	}

	boost::posix_time::ptime start_time( boost::posix_time::microsec_clock::local_time() );

	for (int i=0; i<depths.size(); i++)
	{
		((vtk::VTKVolumeBrickData *)depths[i].get())->setDataNumber(item);
		((vtk::VTKVolumeBrickData *)depths[i].get())->setIsScalar(scalar);
	}

	createDataSet(csFile, depths, sst.str());

    boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

    std::ostringstream ss;
	ss << "Time to create  " << sst.str() << " = " << createTime << " secs";
    LFX_CRITICAL_STATIC( loginfo, ss.str().c_str() );
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::createDataSet( const std::string& csFile, std::vector<VolumeBrickDataPtr> &depths, const std::string &baseName )
{
	if (!depths.size()) return; // no depths then there is a problem
	if (depths.size() == 1) // only 1 depth then this is not a hires lod, create standard fast lod
	{
		createDataSet(csFile, depths[0], baseName);
		return;
	}

    // hires load with all depths coming from the dataset directly
	SaveHierarchy* saver( new SaveHierarchy( baseName ) );
	saver->addAllLevels( depths );
	::createDataSet( csFile, saver );
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::createDataSet( const std::string& csFile, VolumeBrickDataPtr brickData, const std::string &baseName )
{
    //SaveHierarchy* saver( new SaveHierarchy( shapeGen, "shapevolume" ) );
	SaveHierarchy* saver( new SaveHierarchy( baseName ) );
	saver->addLevel( brickData );
	::createDataSet( csFile, saver );
}

#endif