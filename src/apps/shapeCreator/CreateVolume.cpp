#include "CreateVolume.h"
#include "ShapeVolumes.h"

#include <osg/ArgumentParser>
#include <osgDB/FileUtils>
#include <osg/io_utils>

#include <latticefx/core/DBDisk.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/utils/CompilerGuards.h>

//DIAG_OFF(unused-parameter)
//#include "boost/date_time/posix_time/posix_time.hpp"
//#include "boost/date_time/posix_time/posix_time_types.hpp"
//DIAG_ON(unused-parameter)

#ifdef LFX_USE_CRUNCHSTORE
#  include <latticefx/core/DBCrunchStore.h>
#  include <crunchstore/DataManager.h>
#  include <crunchstore/NullCache.h>
#  include <crunchstore/NullBuffer.h>
#  include <crunchstore/SQLiteStore.h>
#endif

using namespace lfx::core;

////////////////////////////////////////////////////////////////////////////////
CreateVolume::CreateVolume(const char *plogstr, const char *ploginfo)
{
	_logstr = plogstr;
	_loginfo = ploginfo;

	_prune = false;
	_depth = 4;

	_useCrunchStore = true;
	_csFileOrFolder = "volume";
	_basename = "shapevolume"; 

	_pcbProgress = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void CreateVolume::setCallbackProgress(lfx::core::ICallbackProgress *pcp)
{
	_pcbProgress = pcp;

	if (_volumeObj != NULL) _volumeObj->setCallbackProgress(pcp);
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::create()
{
	if (_volumeObj == NULL) return false;

	_volumeObj->setDepth(_depth);

	boost::posix_time::ptime start_time( boost::posix_time::microsec_clock::local_time() );
	createDataSet( _csFileOrFolder, _volumeObj, _basename );
    boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

    std::ostringstream ss;
    ss << "Time to create data set = " << createTime << " secs" << std::endl;
    LFX_CRITICAL_STATIC( _logstr, ss.str().c_str() );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::create(osg::ArgumentParser &arguments, const std::string &csFile)
{
	if (!processArgs(arguments)) return false;

	_csFileOrFolder = csFile;
	return create();
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::processArgs(osg::ArgumentParser &arguments)
{
	_prune = false;
	if (arguments.find( "-prune" ) > 0) { _prune = true; }

	_depth = 4;
    arguments.read( "-depth", _depth );
    if( _depth < 1 ) _depth = 1;

	bool softCube( arguments.find( "-scube" ) > 0 );
    bool softSphere( arguments.find( "-ssphere" ) > 0 );

    if( ( arguments.find( "-sphere" ) > 0 ) || softSphere )
    {
        _volumeObj.reset(new SphereVolumeBrickData( _prune, softSphere ));
    }
    else if( arguments.find( "-cone" ) > 0 )
    {
		_volumeObj.reset(new ConeVolumeBrickData( _prune ));
    }
#ifdef VTK_FOUND
    else if( arguments.find( "-vtk" ) > 0 )
    {
    }
#endif
    else
    {
        _volumeObj.reset(new CubeVolumeBrickData( _prune, softCube ));
    }

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::isVtk(osg::ArgumentParser &arguments)
{
#ifdef VTK_FOUND
    if( arguments.find( "-vtk" ) > 0 )
    {
		return true;
    }
#endif

	return false;
}

////////////////////////////////////////////////////////////////////////////////
void CreateVolume::createDataSet( const std::string& csFile, SaveHierarchy* saver )
{
	 // Configure database to use
#ifdef LFX_USE_CRUNCHSTORE
    if( !( csFile.empty() ) && _useCrunchStore)
    {
        DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );

        crunchstore::DataManagerPtr manager( crunchstore::DataManagerPtr( new crunchstore::DataManager() ) );
        crunchstore::DataAbstractionLayerPtr cache( new crunchstore::NullCache );
        crunchstore::DataAbstractionLayerPtr buffer( new crunchstore::NullBuffer );
        manager->SetCache( cache );
        manager->SetBuffer( buffer );
        crunchstore::SQLiteStorePtr sqstore( new crunchstore::SQLiteStore );
        sqstore->SetStorePath( csFile );
        manager->AttachStore( sqstore, crunchstore::Store::BACKINGSTORE_ROLE );
        try
        {
            cs->setDataManager( manager );
        }
        catch( std::exception exc )
        {
            LFX_FATAL_STATIC( _logstr, std::string( exc.what() ) );
            LFX_FATAL_STATIC( _logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        saver->save( ( DBBasePtr )cs );
    }
#endif
    if( csFile.empty() || !_useCrunchStore )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk(csFile) ) );
        saver->save( ( DBBasePtr )disk );
    }
}

////////////////////////////////////////////////////////////////////////////////
void CreateVolume::createDataSet( const std::string& csFile, VolumeBrickDataPtr shapeGen, const std::string &baseName )
{
    //SaveHierarchy* saver( new SaveHierarchy( shapeGen, "shapevolume" ) );
	SaveHierarchy saver( baseName, _pcbProgress );
	saver.addLevel( shapeGen );
	createDataSet( csFile, &saver );
}