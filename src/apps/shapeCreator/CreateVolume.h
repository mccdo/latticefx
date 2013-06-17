#ifndef __CREATE_VOLUME_H__
#define __CREATE_VOLUME_H__

#include <boost/date_time/posix_time/posix_time.hpp>
#include <latticefx/core/HierarchyUtils.h>
#include <string>


class CreateVolume
{
public:
	CreateVolume(const char *plogstr, const char *ploginfo);

	static bool isVtk(osg::ArgumentParser &arguments);

	virtual bool create();
	virtual bool create(osg::ArgumentParser &arguments, const std::string &csFile);

	void setPrune(bool p) { _prune = p; }
	void setDepth(int depth) { _depth = depth; }
	void setCsFileOrFolder(const char *str) { _csFileOrFolder = str; }
	void setUseCrunchStore(bool use) { _useCrunchStore = use; }
	void setVolumeObj(lfx::core::VolumeBrickDataPtr volumeObj) { _volumeObj = volumeObj; }
	void setCallbackProgress(lfx::core::ICallbackProgress *pcp);

protected:
	virtual bool processArgs(osg::ArgumentParser &arguments);

	void createDataSet( const std::string& csFile, lfx::core::SaveHierarchy* saver );
	void createDataSet( const std::string& csFile, lfx::core::VolumeBrickDataPtr shapeGen, const std::string &baseName );

protected:
	std::string _logstr;
	std::string _loginfo;
	bool _prune;
	int _depth;

	bool _useCrunchStore;
	std::string _csFileOrFolder;

	lfx::core::VolumeBrickDataPtr _volumeObj;
};

#endif