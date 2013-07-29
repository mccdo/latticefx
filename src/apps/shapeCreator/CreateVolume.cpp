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
CreateVolume::CreateVolume( const char *plogstr, const char *ploginfo ) :
	_logstr( plogstr ),
	_loginfo( ploginfo ),
	_prune( false ),
	_resPrune( false ),
	_depth( 4 ),
	_useCrunchStore( true ),
	_csFileOrFolder( "volume" ),
	_basename( "shapevolume" ),
	_pcbProgress( NULL )
{
}

////////////////////////////////////////////////////////////////////////////////
void CreateVolume::setCallbackProgress( lfx::core::ICallbackProgress *pcp )
{
	_pcbProgress = pcp;

	if (_volumeObj != NULL) _volumeObj->setCallbackProgress( pcp );
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::create()
{
	if (_volumeObj == NULL) return false;

	_volumeObj->setDepth(_depth);

	// compute progress count
	int brickCount = _volumeObj->getBrickCount();
	int totalCount = brickCount*8; // see Downsampler::getLow
	if (_pcbProgress)
	{
		_pcbProgress->clearProg();
		_pcbProgress->addToProgTot(totalCount);
	}

	boost::posix_time::ptime start_time( boost::posix_time::microsec_clock::local_time() );
	if (!createDataSet( _csFileOrFolder, _volumeObj, _basename )) return false;
    boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

    std::ostringstream ss;
    ss << "Time to create data set = " << createTime << " secs" << std::endl;
    LFX_CRITICAL_STATIC( _logstr, ss.str().c_str() );
	if (_pcbProgress) _pcbProgress->sendMsg( ss.str().c_str() );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::create( osg::ArgumentParser &arguments, const std::string &csFile )
{
	if (!processArgs(arguments)) return false;

	_csFileOrFolder = csFile;
	return create();
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::processArgs( osg::ArgumentParser &arguments )
{
	_prune = false;
	if (arguments.find( "-prune" ) > 0) { _prune = true; }

	_resPrune = false;
	if ( arguments.find( "-rp" ) > 0 ) { _resPrune = true; }

	_depth = 4;
    arguments.read( "-depth", _depth );
    if( _depth < 1 ) _depth = 1;

	bool softCube( arguments.find( "-scube" ) > 0 );
    bool softSphere( arguments.find( "-ssphere" ) > 0 );

    if( ( arguments.find( "-sphere" ) > 0 ) || softSphere )
    {
        _volumeObj.reset(new SphereVolumeBrickData( _prune, softSphere, _resPrune ));
    }
    else if( arguments.find( "-cone" ) > 0 )
    {
		_volumeObj.reset(new ConeVolumeBrickData( _prune, _resPrune ));
    }
#ifdef VTK_FOUND
    else if( arguments.find( "-vtk" ) > 0 )
    {
    }
#endif
    else
    {
        _volumeObj.reset(new CubeVolumeBrickData( _prune, softCube, _resPrune ));
    }

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::isVtk( osg::ArgumentParser &arguments )
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
bool CreateVolume::createDataSet( const std::string& csFile, SaveHierarchy* saver )
{
	if (_pcbProgress && _pcbProgress->checkCancel()) return false;

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
			std::ostringstream ss;
			ss << "Unable to set DataManager. Make sure the db file exists: " << csFile;
            LFX_FATAL_STATIC( _logstr, std::string( exc.what() ) );
            LFX_FATAL_STATIC( _logstr, ss.str().c_str() );
			if (_pcbProgress) _pcbProgress->sendMsg(ss.str().c_str());
            return false;
        }

		// start a transaction
		crunchstore::SQLiteTransactionKey key = sqstore->BeginTransaction();
		cs->setTransactionKey(&key);

        saver->save( ( DBBasePtr )cs );

		// end the transaction
		sqstore->EndTransaction(key);
    }
#endif
    if( csFile.empty() || !_useCrunchStore )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk(csFile) ) );
        saver->save( ( DBBasePtr )disk );
    }

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CreateVolume::createDataSet( const std::string& csFile, VolumeBrickDataPtr shapeGen, const std::string &baseName )
{
    //SaveHierarchy* saver( new SaveHierarchy( shapeGen, "shapevolume" ) );
	SaveHierarchy saver( baseName );
	saver.addLevel( shapeGen ); 
	return createDataSet( csFile, &saver );
}