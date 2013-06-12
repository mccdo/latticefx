#ifndef __SHAPE_VOLUMES_H__
#define __SHAPE_VOLUMES_H__

#include <latticefx/core/HierarchyUtils.h>

class CubeVolumeBrickData : public lfx::core::VolumeBrickData
{
public:
    CubeVolumeBrickData( const bool prune, const bool soft );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;

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
    SphereVolumeBrickData( const bool prune, const bool soft );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
    
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
    ConeVolumeBrickData( const bool prune );

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const;

protected:
    osg::Vec3s _brickRes;
    float _baseRad;
};

#endif