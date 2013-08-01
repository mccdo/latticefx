#ifndef __SHAPE_VOLUMES_H__
#define __SHAPE_VOLUMES_H__

#include <latticefx/core/HierarchyUtils.h>

class CubeVolumeBrickData : public lfx::core::VolumeBrickData
{
public:
    CubeVolumeBrickData( bool prune, bool soft, bool resPrune );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
	virtual bool resolutionPrune( const osg::Vec3s& brickNum, const osg::Vec3s& brickNumParent ) const;

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const;

protected:
    osg::Vec3s _brickRes;
    osg::Vec3f _cubeMin, _cubeMax;
    bool _soft;
};


class SphereVolumeBrickData : public lfx::core::VolumeBrickData
{
public:
    SphereVolumeBrickData( bool prune, bool soft, bool resPrune );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
	virtual bool resolutionPrune( const osg::Vec3s& brickNum, const osg::Vec3s& brickNumParent ) const;
    
protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const;

protected:
    osg::Vec3s _brickRes;
    osg::Vec3f _center;
    float _minRad, _maxRad;
};

class ConeVolumeBrickData : public lfx::core::VolumeBrickData
{
public:
    ConeVolumeBrickData( bool prune, bool resPrune );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
	virtual bool resolutionPrune( const osg::Vec3s& brickNum, const osg::Vec3s& brickNumParent ) const;

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const;

protected:
    osg::Vec3s _brickRes;
    float _baseRad;
};

#endif