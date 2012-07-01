/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#ifndef __LATTICEFX_DATA_SET_H__
#define __LATTICEFX_DATA_SET_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/Renderer.h>

#include <osg/ref_ptr>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <string>
#include <list>
#include <map>
#include <set>



// Forwards
namespace osg {
    class Group;
}
namespace lfx {
    class PagingCallback;
}


namespace lfx {


/** An ordered set of time values. */
typedef std::set< double > TimeSet;

/** \brief Map of time values to ChannelDataPtr.
When ChannelData are added for a specific time, they are added to a map of this type. */
typedef std::map< double, ChannelDataPtr > TimeDataMap;


/** \class DataSet DataSet.h <latticefx/core/DataSet.h>
\brief Data set storage and interface.
\details Create an instance of this class for each data set you application
uses and renders. Each DataSet can contain 0 or more ChannelData objects, which
are arrays of data such as xyz vertex values or other scalar data. The data set
can also contain 0 or more RTPOperation objects, which are mask, filter, or
channel creation functions. An example is the MyMask RTPOperation.
*/
class LATTICEFX_EXPORT DataSet
{
public:
    ///Constructor
    DataSet();
    ///Copy constructor
    DataSet( const DataSet& rhs );
    ///Destructor
    virtual ~DataSet();


    /** \name Data Section
    \details TBD */
    
    ///\{
    /** \brief Add a data channel to the ChannelDataList for a specific time value \c time. */
    void addChannel( const ChannelDataPtr channel, const double time=0. );

    /** \brief Replaces a ChannelData with the same name as \c channel.
    \details If no ChannelData can be found with the same name as \c channel, this function
    behaves the same as addChannel. */
    void replaceChannel( const ChannelDataPtr channel, const double time=0. );

    /** \brief Get a named channel for a specific time value \c time.
    \returns NULL if the named channel doesn't exist at the specified time. */
    ChannelDataPtr getChannel( const std::string& name, const double time=0. );
    /** \overload ChannelDataPtr getChannel(const std::string&,const double) */
    const ChannelDataPtr getChannel( const std::string& name, const double time=0. ) const;



    /** \brief Returns the min and max time extents of all attached data. */
    osg::Vec2d getTimeRange() const;
    /** \overload */
    void getTimeRange( double& minTime, double& maxTime ) const;
    ///\}


    /** \name Preprocessing & Caching section
    \details TBD */
    ///\{
    /** \brief TBD
    \details TBD */
    void addPreprocess( const PreprocessPtr pre );
    ///\}


    /** \name Run-Time Operations Section
    \details TBD */

    ///\{
    /** \brief Add a runtime processing operation to the end of the operation list.
    \details All runtime processing operations are executed sequentially. Note that
    the app is responsible for maintaining operation list order.*/
    void addOperation( const RTPOperationPtr op );

    /** \brief Insert a runtime processing operation at the specified index.
    \details RTPOperation objects already at location >= \c index will have their
    location indices incremented. */
    void insertOperation( const unsigned int index, const RTPOperationPtr op );

    /** \brief Insert \c op before \c location.
    \details \c location and other downstream RTPOperation objects will follow
    \c op during processing. */
    void insertOperation( const RTPOperationPtr location, const RTPOperationPtr op );

    /** \brief Return the total number of runtime processing operations. */
    unsigned int getNumOperations() const;

    /** \brief Get the operation at the specific index.
    \returns NULL if \c index >= getNumOperations(). */
    RTPOperationPtr getOperation( const unsigned int index );

    /** \overload RTPOperationPtr DataSet::getOperation( const unsigned int index ); */
    const RTPOperationPtr getOperation( const unsigned int index ) const;

    /** \brief Get all operations.
    \returns An empty list if no operations exist for this DataSet. */
    RTPOperationList& getOperations();

    /** \overload RTPOperationList& DataSet::getOprtations(); */
    const RTPOperationList& getOperations() const;
    ///\}

    /** Need to be able to set a list of operations.
    Store multiple lists and switch between them. */



    /** \name Rendering Framework Section
    \details TBD */

    ///\{
    /** \brief Specify the Renderer plugin operation for creating the scene graph.
    \details TBD
    */
    void setRenderer( const RendererPtr renderer );

    /** \brief Get the Renderer plugin operation.
    \details TBD */
    RendererPtr getRenderer();
    /** \overload */
    const RendererPtr getRenderer() const;

    /** \brief Get the OSG scene graph.
    \details implicitly calls updateAll(), then returns the \c _sceneGraph
    member variable. */
    osg::Node* getSceneData();


    /** Interface to tell DataSet to use an experimental method to
    construct a scene graph that doesn't page geometry, pages only
    texture data instead. */
    void setSceneGraphPagesTexturesOnly();
    ///\}


    /** \brief Execute all operations (process all data and create scene graph).
    \details Runs all Preprocessing & Caching, Run-Time Processing, and
    Rendering Framework operations.

    To preprocess data as an offline step, apps can attach ChannelData objects and
    Preprocess objects, then call this function. If no Renderer is attached (for
    example, setRenderer(NULL) ), then DataSet will not create a scene graph. */
    bool updateAll();

    /** \brief Dirty flags used by updateAll() to determine which operations to run. */
    typedef enum {
        NOT_DIRTY        = ( 0 ),
        PREPROCESS_DIRTY = ( 0x1 << 0 ),
        RTP_DIRTY        = ( 0x1 << 1 ),
        RENDERER_DIRTY   = ( 0x1 << 2 ),
        ALL_DIRTY = ( PREPROCESS_DIRTY | RTP_DIRTY | RENDERER_DIRTY )
    } DirtyFlags;
    void setDirty( const DirtyFlags flags=ALL_DIRTY );
    DirtyFlags getDirty() const;

protected:
    bool updatePreprocessing();
    bool updateRunTimeProcessing();
    bool updateRenderer();

    void createFallbackMaskList();

    /** \brief TBD
    \details TBD */
    osg::Node* recurseGetSceneGraph( ChannelDataList& data, ChannelDataPtr mask );

    TimeSet getTimeSet() const;
    ChannelDataList getDataAtTime( const double time );
    ChannelDataList getCompositeChannels( ChannelDataList data, const unsigned int index );
    static void setInputs( OperationBasePtr opPtr, ChannelDataList& currentData );

    ChannelDataOSGArrayPtr createSizedMask(  const ChannelDataList& dataList );


    typedef std::map< double, ChannelDataList > ChannelDataTimeMap;
    ChannelDataTimeMap _data;

    typedef std::set< std::string > StringSet;
    /** Set of unique ChannelData names. */
    StringSet _dataNames;

    PreprocessList _preprocess;
    RTPOperationList _ops;
    RendererPtr _renderer;

    /** A mask per time step, possible heterogeneous lengths per time step. */
    ChannelDataList _maskList;

    osg::ref_ptr< osg::Group > _sceneGraph;
    DirtyFlags _dirtyFlags;
};

typedef boost::shared_ptr< DataSet > DataSetPtr;
typedef std::list< DataSetPtr > DataSetList;


// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif