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
//#include "shapeCreatorDefines.h"

#ifdef VTK_FOUND

#include "VtkCreator.h"

#ifndef Q_MOC_RUN
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/ArgumentParser>
#include <osgDB/FileUtils>
#include <osg/io_utils>

//#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////
VtkCreator::VtkCreator( const char *plogstr, const char *ploginfo )
	: CreateVolume(plogstr, ploginfo)
{
	_threads = 32;
	_hireslod = false;
	_nocache = false;
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::init( const char *vtkFile )
{
    _pds.reset(new vtk::DataSet() );
    _pds->SetFileName(vtkFile);
    _pds->LoadData();
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::haveFile()
{
    if (_pds == NULL) return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
int VtkCreator::getNumScalars()
{
    if (_pds == NULL) { return 0; }

	return _pds->GetNumberOfScalars();

}

////////////////////////////////////////////////////////////////////////////////
int VtkCreator::getNumVectors()
{
    if (_pds == NULL) { return 0; }

	return _pds->GetNumberOfVectors();
}

////////////////////////////////////////////////////////////////////////////////
std::string VtkCreator::getScalarName( int num )
{
    if (num <0 || num >= getNumScalars()) return std::string( "" );

    return _pds->GetScalarName(num);
}

////////////////////////////////////////////////////////////////////////////////
std::string VtkCreator::getVectorName(int num)
{
    if (num <0 || num >= getNumVectors()) return std::string( "" );

    return _pds->GetVectorName(num);
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::create()
{
	std::ostringstream ss;

	boost::posix_time::ptime start_time( boost::posix_time::microsec_clock::local_time() );

	//vtk::VTKVolumeBrickDataPtr vbd(new vtk::VTKVolumeBrickData(ds, true, 0, true, osg::Vec3s(32,32,32), threads));
	//vtk::VTKVolumeBrickDataPtr vbd(new vtk::VTKVolumeBrickData(ds, true, 0, true, osg::Vec3s(8,8,8), threads));
	VolumeBrickDataPtr vbd( new vtk::VTKVolumeBrickData( _pds, _prune, 0, true, osg::Vec3s(32,32,32), _threads ) );
    vbd->setDepth( _depth );

	// get our depths
	SaveHierarchy::LODVector depths;
	getLod( &depths, vbd, _pds, _threads, _hireslod, _prune );


	if (_nocache)
	{
		LFX_CRITICAL_STATIC( _loginfo, "Vtk lookup cache is disabled." );
		if (_pcbProgress) _pcbProgress->sendMsg( "Vtk lookup cache is disabled." );
		setCacheCreate( depths, false );
		setCacheUse( depths, false );
	}
	else
	{
		LFX_CRITICAL_STATIC( _loginfo, "Vtk lookup cache is enabled." );
		if (_pcbProgress) _pcbProgress->sendMsg( "Vtk lookup cache is enabled." );
		setCacheCreate( depths, true );
		setCacheUse( depths, true );
	}
	LFX_CRITICAL_STATIC( _loginfo, "" );

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

	// compute progress count
	int brickCount = vbd->getBrickCount();
	int itemCount = brickCount*8; // see VolumeBrickData::getSeamlessBrick (8 create image calls), and SaveHierarchy::recurseSaveBricks(1 call for each brick)
	int totalCount = _scalars.size() * itemCount + _vectors.size() * itemCount + 10;
	if ( _pcbProgress )
	{
		_pcbProgress->clearProg();
		_pcbProgress->addToProgTot( totalCount );
	}


	for ( unsigned int i=0; i<_scalars.size(); i++ )
	{
		if ( _pcbProgress )
		{
			if (_pcbProgress->checkCancel()) return 0;
			std::string name = getScalarName( _scalars[i] );

			std::ostringstream oss;
			oss << "Creating bricks for scalar " << name << "...";
			_pcbProgress->sendMsg(oss.str().c_str());
		}

		if ( !vtkCreateBricks( depths, _csFileOrFolder, _scalars[i], true ) ) return false;
		if (!i) { setCacheCreate( depths, false ); }
	}

	for ( unsigned int i=0; i<_vectors.size(); i++ )
	{
		if ( _pcbProgress )
		{
			if (_pcbProgress->checkCancel()) return 0;
			std::string name = getVectorName( _vectors[i] );

			std::ostringstream oss;
			oss << "Creating bricks for vector " << name << "...";
			_pcbProgress->sendMsg( oss.str().c_str() );
		}
		

		if ( !vtkCreateBricks( depths, _csFileOrFolder, _vectors[i], false ) ) return false;
		if ( !i ) { setCacheCreate( depths, false ); }
	}

	boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

	ss << "Total time to process dataset = " << createTime << " secs" << std::endl;
    LFX_CRITICAL_STATIC( _loginfo, ss.str().c_str() );
	if ( _pcbProgress ) _pcbProgress->sendMsg( ss.str().c_str() );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::create( osg::ArgumentParser &arguments, const std::string &csFile )
{
	return CreateVolume::create( arguments, csFile );
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::processArgs( osg::ArgumentParser &arguments )
{
	std::ostringstream ss;

	if ( !CreateVolume::processArgs( arguments ) ) return false;

	// get the vtk file
	int iarg = arguments.find( "-vtk" );
	if (iarg < 0 || arguments.argc() <= (iarg+1))
    {
		LFX_CRITICAL_STATIC( _logstr, "You must specify a vtk data file." );
        return false;
	}
	iarg++;

	std::string vtkFile = osgDB::findDataFile( arguments[iarg] );
	if ( !boost::filesystem::exists( vtkFile ) )
	{
		LFX_CRITICAL_STATIC( _logstr, "Vtk data file could not be found." );
        return false;
	}

	_pds.reset( new vtk::DataSet() );
    _pds->SetFileName( vtkFile );
    _pds->LoadData();


    // get the threads
	_threads = 32;
	arguments.read( "-threads", _threads );
	if ( _threads <= 0 ) 
	{
		_threads = 32;
		LFX_CRITICAL_STATIC( _logstr, "Invalid number of threads using restoring default" );
	}

	ss << "Number of threads: " << _threads;
    LFX_CRITICAL_STATIC( _loginfo, ss.str().c_str() );
	ss.str( std::string() );
	ss.clear();

	_hireslod = false;
	if ( arguments.find( "-hireslod" ) > 0 ) { _hireslod = true; }


	// get scalars and vectors from commandf line
	int countScl = getNumScalars();
	int countVec = getNumVectors();

	// find which scalars and which vectors to process
	getVtkProcessOptions( arguments, countScl, &_scalars, true );
	getVtkProcessOptions( arguments, countVec, &_vectors, false );

	// no items were specified so process all
	if ( _scalars.size() == 0 && _vectors.size() == 0 )
	{
		for ( int i=0; i<countScl; i++ )
		{
			_scalars.push_back( i );
		}
		for ( int i=0; i<countVec; i++ )
		{
			_vectors.push_back( i );
		}
	}

	_nocache = false;
	if ( arguments.find("-nocache") >= 0 )
	{
		_nocache = true;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::setCacheCreate( SaveHierarchy::LODVector &depths, bool create )
{
	for (unsigned int i=0; i<depths.size(); i++)
	{
		((vtk::VTKVolumeBrickData *)depths[i].get())->cacheCreate( create );
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::setCacheUse( SaveHierarchy::LODVector &depths, bool use )
{
	for ( unsigned int i=0; i<depths.size(); i++ )
	{
		((vtk::VTKVolumeBrickData *)depths[i].get())->cacheUse( use );
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::getLod( SaveHierarchy::LODVector* pdepths, VolumeBrickDataPtr hilod, vtk::DataSetPtr ds, int threads, bool hireslod, bool prune )
{
	hilod->setCallbackProgress( _pcbProgress );

	pdepths->clear();
	if ( !hireslod )
	{
		pdepths->push_back( hilod );
		return;
	}

	int depth = hilod->getDepth();
	if ( depth == 1 ) // only a single depth, not likely
	{
		pdepths->push_back( hilod );
		return;
	}

	pdepths->resize( depth );
	(*pdepths)[depth-1] = hilod;

	for ( int i=0; i<depth-1; i++ )
	{
		VolumeBrickDataPtr vbd( new vtk::VTKVolumeBrickData(ds, prune, 0, true, osg::Vec3s(32,32,32), threads) );
		vbd->setDepth( i+1 );
		vbd->setCallbackProgress( _pcbProgress );
		(*pdepths)[i] = vbd;
	}
}

////////////////////////////////////////////////////////////////////////////////
void VtkCreator::getVtkProcessOptions( osg::ArgumentParser &arguments, int totalItems, std::vector<int> *pItems, bool scalar )
{
	int iarg = 0;
	std::string cmd = "-s";
	if (!scalar) cmd = "-v";

	pItems->clear();

	// find which scalars or vectors
	
	// do they want all items
	iarg = arguments.find(cmd);
	if ( iarg >= 0 && iarg < arguments.argc() )
	{
		for ( int i=0; i< totalItems; i++ )
		{
			pItems->push_back( i );
		}

		return;
	}

	
	// look for individual scalars
	for ( int i=0; i<totalItems; i++ )
	{
		std::ostringstream ss;
		ss << cmd << i;

		iarg = arguments.find(ss.str());
		if ( iarg >= 0 && iarg < arguments.argc() )
		{
			pItems->push_back( i );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::vtkCreateBricks( SaveHierarchy::LODVector &depths, const std::string csFile, int item, bool scalar )
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

	for ( int i=0; i<depths.size(); i++ )
	{
		((vtk::VTKVolumeBrickData *)depths[i].get())->setDataNumber( item );
		((vtk::VTKVolumeBrickData *)depths[i].get())->setIsScalar( scalar );
	}

	if ( !createDataSet( csFile, depths, sst.str() ) ) return false;

    boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

    std::ostringstream ss;
	ss << "Time to create  " << sst.str() << " = " << createTime << " secs";
    LFX_CRITICAL_STATIC( _loginfo, ss.str().c_str() );
	if ( _pcbProgress ) _pcbProgress->sendMsg( ss.str().c_str() );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::createDataSet( const std::string& csFile, std::vector<VolumeBrickDataPtr> &depths, const std::string &baseName )
{
	if ( !depths.size() ) return false; // no depths then there is a problem
	if ( depths.size() == 1 ) // only 1 depth then this is not a hires lod, create standard fast lod
	{
		return createDataSet( csFile, depths[0], baseName );
	}

    // hires load with all depths coming from the dataset directly
	SaveHierarchy saver( baseName );
	saver.addAllLevels( depths );
	return CreateVolume::createDataSet( csFile, &saver );
}

////////////////////////////////////////////////////////////////////////////////
bool VtkCreator::createDataSet( const std::string& csFile, VolumeBrickDataPtr brickData, const std::string &baseName )
{
	SaveHierarchy* saver( new SaveHierarchy( baseName ) );
	saver->addLevel( brickData );
	return CreateVolume::createDataSet( csFile, saver );
}

#endif
