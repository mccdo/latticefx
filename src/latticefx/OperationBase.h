
#ifndef __LATTICEFX_OPERATION_BASE_H__
#define __LATTICEFX_OPERATION_BASE_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>
#include <map>



namespace lfx {


// Forward declaration.
class DataSet;


/** \class OperationValue OperationBase.h <latticefx/OperationBase.h>
\brief
\details
*/
struct LATTICEFX_EXPORT OperationValue
{
public:
    typedef enum {
        NO_VALUE,
        INT_VALUE,
        FLOAT_VALUE,
        VEC3_VALUE,
        STRING_VALUE,
        BOOL_VALUE
    } ValueType;

    OperationValue();
    virtual ~OperationValue();

    explicit OperationValue( const int value );
    explicit OperationValue( const float value );
    explicit OperationValue( const osg::Vec3& value );
    explicit OperationValue( const std::string& value );
    explicit OperationValue( const bool value );

    void set( const int value );
    void set( const float value );
    void set( const osg::Vec3& value );
    void set( const std::string& value );
    void set( const bool value );

    ValueType getType() const;

    int getInt() const;
    float getFloat() const;
    osg::Vec3 getVec3() const;
    const std::string& getString() const;
    bool getBool() const;

protected:
    ValueType _valueType;
    union {
        int _int;
        float _float;
        float _vec3[ 3 ];
        bool _bool;
    } _value;
    std::string _valueString;
};

typedef std::map< std::string, OperationValue > NameValueMap;



/** \class OperationBase OperationBase.h <latticefx/OperationBase.h>
\brief Base class for Preprocessing & Caching, Run-Time Processing, and Rendering Framework operations.
\details TBD
*/
class LATTICEFX_EXPORT OperationBase
{
public:
    typedef enum {
        UnspecifiedType,
        PreprocessCacheType,
        RunTimeProcessingType,
        RendererType
    } OperationType;
    /** \brief */
    OperationType getType() const { return( _opType ); }

    OperationBase( const OperationType opType=UnspecifiedType );
    virtual ~OperationBase();

    virtual lfx::OperationBase* create() { return( NULL ); }

    typedef std::vector< std::string > StringList;
    /** \brief Add an input by name.
    \details \c name must match the name of a ChannelData added to the DataSet. */
    virtual void addInput( const std::string& name );
    /** \brief Retrieve the ChannelData corresponding to the named input.
    \details Before invoking the OperationBase, DataSet uses the input names to
    assign actual ChannelData inputs. Use this method to retrieve such an input. */
    virtual ChannelDataPtr getInput( const std::string& name );
    /** \brief Add all inputs by name.
    \details All names in \c inputList must match the names of ChannelData objects added to the DataSet. */
    virtual void setInputs( StringList& inputList );
    /** \brief Get all input names. */
    virtual StringList getInputNames();
    /** \overload StringList OperationBase::getInputNames() */
    virtual const StringList& getInputNames() const;


    /** \brief */
    virtual void setEnable( const bool enable=true );
    /** \brief */
    virtual bool getEnable() const;


    /** \brief */
    void setValue( const std::string& name, const OperationValue& value );
    /** \brief */
    bool hasValue( const std::string& name ) const;
    /** \brief */
    const OperationValue* getValue( const std::string& name ) const;

protected:
    friend DataSet;
    virtual void addInput( ChannelDataPtr input );
    virtual void setInputs( ChannelDataList inputList );
    virtual ChannelDataList getInputs();

    /** List of actual inputs. During data processing, DataSet assigns ChannelData
    objects to this array based on names stored in _inputNames. */
    ChannelDataList _inputs;
    /** List of input names assigned by the application. */
    StringList _inputNames;

    OperationType _opType;
    bool _enable;
    NameValueMap _nameValueMap;
};

typedef boost::shared_ptr< OperationBase > OperationBasePtr;


// lfx
}


// __LATTICEFX_OPERATION_BASE_H__
#endif
