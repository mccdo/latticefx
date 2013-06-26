#ifndef __CREATE_VOLUME_H__
#define __CREATE_VOLUME_H__

#ifndef Q_MOC_RUN
#include <boost/date_time/posix_time/posix_time.hpp>
#include <latticefx/core/HierarchyUtils.h>
#endif

#include <string>


class CreateVolume
{
public:
	CreateVolume( const char *plogstr, const char *ploginfo );
    virtual ~CreateVolume(){;}

	static bool isVtk( osg::ArgumentParser &arguments );

	virtual bool create();
	virtual bool create( osg::ArgumentParser &arguments, const std::string &csFile );

	void setPrune( bool p ) { _prune = p; }
	void setDepth( int depth ) { _depth = depth; }
	void setCsFileOrFolder( const char *str ) { _csFileOrFolder = str; }
	void setBaseName( const char *str ) { _basename = str; }
	void setUseCrunchStore( bool use ) { _useCrunchStore = use; }
	void setVolumeObj( lfx::core::VolumeBrickDataPtr volumeObj ) { _volumeObj = volumeObj; }
	void setCallbackProgress( lfx::core::ICallbackProgress *pcp );

protected:
    
	virtual bool processArgs( osg::ArgumentParser &arguments );

	bool createDataSet( const std::string& csFile, lfx::core::SaveHierarchy* saver );
	bool createDataSet( const std::string& csFile, lfx::core::VolumeBrickDataPtr shapeGen, const std::string &baseName );

protected:
	std::string _logstr;
	std::string _loginfo;
	bool _prune;
	int _depth;

	bool _useCrunchStore;
	std::string _csFileOrFolder;
	std::string _basename;

	lfx::core::VolumeBrickDataPtr _volumeObj;
	lfx::core::ICallbackProgress *_pcbProgress;
};

#endif