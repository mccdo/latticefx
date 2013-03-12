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

#include <latticefx/core/DBUtils.h>
#include <latticefx/core/LogMacros.h>

#ifdef LFX_USE_CRUNCHSTORE
#  include <crunchstore/Persistable.h>
#else
#  include <osgDB/ReadFile>
#  include <osgDB/WriteFile>
#endif

#include <osg/Image>
#include <osg/Array>

#include <iomanip>
#include <sstream>


namespace lfx
{
namespace core
{



bool DBUsesCrunchStore()
{
#ifdef LFX_USE_CRUNCHSTORE
    return( true );
#else
    return( false );
#endif
}

DBKey generateDBKey()
{
    static unsigned int keyCounter( 0 );

    std::ostringstream ostr;
    ostr << "dbKey" << std::setfill( '0' ) <<
         std::setw( 10 ) << keyCounter++;
#ifndef LFX_USE_CRUNCHSTORE
    ostr << ".ive";
#endif
    return( DBKey( ostr.str() ) );
}
DBKey generateDBKey( const std::string& baseName, const TimeValue time )
{
    std::ostringstream ostr;
    ostr.precision( 20 );
    ostr << baseName << "-" << time;
    return( ( DBKey ) ostr.str() );
}



#ifdef LFX_USE_CRUNCHSTORE


namespace db = crunchstore;


PersistPtr s_persist( db::PersistablePtr( ( db::Persistable* )NULL ) );

void s_setPersistable( PersistPtr persist )
{
    s_persist = persist;
}
PersistPtr s_getPersistable()
{
    if( s_persist == NULL )
    {
        s_persist = db::PersistablePtr( new db::Persistable );
    }
    return( s_persist );
}


/** \class RefPtrAllocator
\brief STL container allocator that allocates memory from an osg::ref_ptr.
*/
template <class T>
class RefPtrAllocator
{
public:
    // type definitions
    typedef T        value_type;
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U
    template <class U>
    struct rebind
    {
        typedef RefPtrAllocator<U> other;
    };

    // return address of values
    pointer address( reference value ) const
    {
        return &value;
    }
    const_pointer address( const_reference value ) const
    {
        return &value;
    }


    typedef osg::ref_ptr< osg::Referenced > RefPtr;

    /* constructors and destructor
    */
    RefPtrAllocator( RefPtr refAddress = NULL, const size_type size = 0 )
        : _refAddress( refAddress ),
          _size( size )
    {
    }
    RefPtrAllocator( const RefPtrAllocator& rhs )
        : _refAddress( rhs._refAddress ),
          _size( rhs._size )
    {
    }
    template <class U>
    RefPtrAllocator( const RefPtrAllocator<U>& rhs )
        : _size( 0 )
    {
    }
    ~RefPtrAllocator()
    {
    }

    // return maximum number of elements that can be allocated
    size_type max_size() const throw()
    {
        return( _size );
    }

    // allocate but don't initialize num elements of type T
    pointer allocate( size_type num, const void* = 0 )
    {
        // print message and allocate memory with global new
        //std::cerr << "allocate " << num << " element(s)" << " of size " << sizeof(T) << std::endl;
        //std::cerr << "  _refAddress " << (void*)_refAddress.get() << std::endl;
        pointer ret;
        if( _refAddress == NULL )
        {
            ret = ( pointer )( ::operator new( num * sizeof( T ) ) );
        }
        else
        {
            ret = ( pointer )( _refAddress.get() );
        }
        //std::cerr << " allocated at: " << (void*)ret << std::endl;
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct( pointer p, const T& value )
    {
        // initialize memory with placement new
        if( ( p < ( pointer )( _refAddress.get() ) ) || ( p >= ( pointer )( _refAddress.get() ) + _size ) )
        {
            new( ( void* )p )T( value );
        }
    }

    // destroy elements of initialized storage p
    void destroy( pointer p )
    {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate( pointer p, size_type num )
    {
        // print message and deallocate memory with global delete
        //std::cerr << "deallocate " << num << " element(s)" << " of size " << sizeof(T) << " at: " << (void*)p << std::endl;
        //std::cerr << "  _refAddress " << (void*)_refAddress.get() << std::endl;
        if( p != ( pointer )( _refAddress.get() ) )
        {
            ::operator delete( ( void* )p );
        }
    }

    // \c size is the number of elements stored at \c address.
    void setAddress( RefPtr refAddress, const size_type size )
    {
        _refAddress = refAddress;
        _size = size;
    }

protected:
    RefPtr _refAddress;
    size_type _size;
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator== ( const RefPtrAllocator<T1>&,
                  const RefPtrAllocator<T2>& ) throw()
{
    return true;
}
template <class T1, class T2>
bool operator!= ( const RefPtrAllocator<T1>&,
                  const RefPtrAllocator<T2>& ) throw()
{
    return false;
}


template <class T>
class RawMemoryAllocator
{
public:
    // type definitions
    typedef T        value_type;
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U
    template <class U>
    struct rebind
    {
        typedef RawMemoryAllocator<U> other;
    };

    // return address of values
    pointer address( reference value ) const
    {
        return &value;
    }
    const_pointer address( const_reference value ) const
    {
        return &value;
    }


    /* constructors and destructor
    */
    RawMemoryAllocator( pointer address = ( pointer )NULL, const size_type size = 0 )
        : _address( address ),
          _size( size )
    {
    }
    RawMemoryAllocator( const RawMemoryAllocator& rhs )
        : _address( rhs._address ),
          _size( rhs._size )
    {
    }
    template <class U>
    RawMemoryAllocator( const RawMemoryAllocator<U>& rhs )
        : _address( ( pointer )NULL ),
          _size( 0 )
    {
    }
    ~RawMemoryAllocator()
    {
    }

    // return maximum number of elements that can be allocated
    size_type max_size() const throw()
    {
        return( _size );
    }

    // allocate but don't initialize num elements of type T
    pointer allocate( size_type num, const void* = 0 )
    {
        // print message and allocate memory with global new
        //std::cerr << "allocate " << num << " element(s)" << " of size " << sizeof(T) << std::endl;
        //std::cerr << "  _address " << (void*)_address << std::endl;
        pointer ret;
        if( _address == NULL )
        {
            ret = ( pointer )( ::operator new( num * sizeof( T ) ) );
        }
        else
        {
            ret = _address;
        }
        //std::cerr << " allocated at: " << (void*)ret << std::endl;
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct( pointer p, const T& value )
    {
        // initialize memory with placement new
        if( ( p < _address ) || ( p >= _address + _size ) )
        {
            new( ( void* )p )T( value );
        }
    }

    // destroy elements of initialized storage p
    void destroy( pointer p )
    {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate( pointer p, size_type num )
    {
        // print message and deallocate memory with global delete
        //std::cerr << "deallocate " << num << " element(s)" << " of size " << sizeof(T) << " at: " << (void*)p << std::endl;
        //std::cerr << "  _address " << (void*)_address << std::endl;
        if( p != _address )
        {
            ::operator delete( ( void* )p );
        }
    }


    // \c size is the number of elements stored at \c address.
    void setAddress( pointer address, const size_type size )
    {
        _address = address;
        _size = size;
    }

protected:
    pointer _address;
    size_type _size;
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator== ( const RawMemoryAllocator<T1>&,
                  const RawMemoryAllocator<T2>& ) throw()
{
    return true;
}
template <class T1, class T2>
bool operator!= ( const RawMemoryAllocator<T1>&,
                  const RawMemoryAllocator<T2>& ) throw()
{
    return false;
}


typedef RefPtrAllocator< char > RefCharAllocator;
typedef RawMemoryAllocator< char > RawCharAllocator;
typedef std::vector< char, RefCharAllocator > DBRefCharVec;
typedef std::vector< char, RawCharAllocator > DBRawCharVec;


bool storeImage( const osg::Image* image, const DBKey& dbKey )
{
    db::PersistablePtr persist( s_getPersistable() );

    {
        // Create an STL allocator that allocates memory using the address
        // stored in 'image'. No actual allocation is done.
        const size_t sz( sizeof( *image ) );
        RefCharAllocator localAllocator(
            RefCharAllocator::RefPtr( const_cast< osg::Image* >( image ) ), sz );

        // Create std::vector<char> for storing in DB. We use the custom
        // allocator to avoid the data copy. A simple resize()
        // changes the internal size member var and the array is immediately
        // filled with the storage referenced by 'image'.
        DBRefCharVec cv( localAllocator );
        cv.resize( sz );

        // Add to database. This mean's we now own a copy of this memory,
        // but it is not stored in a ref_ptr. So, to ensure it isn't deleted
        // when the last ref_ptr goes away, do an explicit call to ref().
        const DBKey key( dbKey + std::string( "-imageOverhead" ) );
        if( persist->DatumExists( key ) )
        {
            persist->SetDatumValue( key, cv );
        }
        else
        {
            persist->AddDatum( key, cv );
        }
        image->ref();
    }

    // Now do essentially the same thing with the Image's data, but no need
    // for a RefPtrAllocator. Instead use a RawMemoryAllocator to avoid
    // expensive data copy.
    {
        const size_t sz( image->getTotalSizeInBytes() );
        RawCharAllocator localAllocator(
            ( RawCharAllocator::pointer )( image->data() ), sz );

        DBRawCharVec cv( localAllocator );
        cv.resize( sz );

        const DBKey key( dbKey + std::string( "-imageData" ) );
        if( persist->DatumExists( key ) )
        {
            persist->SetDatumValue( key, cv );
        }
        else
        {
            persist->AddDatum( key, cv );
        }
    }

    return( true );
}
osg::Image* loadImage( const DBKey& dbKey )
{
    db::PersistablePtr persist( s_getPersistable() );

    const DBKey overheadKey( dbKey + std::string( "-imageOverhead" ) );
    const DBKey dataKey( dbKey + std::string( "-imageData" ) );
    if( !( persist->DatumExists( overheadKey ) ) ||
            !( persist->DatumExists( dataKey ) ) )
    {
        LFX_WARNING_STATIC( "lfx.core.db", "loadImage() failed to find key " + dbKey );
        return( NULL );
    }

    // Get the Image overhead block.
    const DBRefCharVec& overheadVec( persist->GetDatumValue< DBRefCharVec >( overheadKey ) );
    osg::Image* image( ( osg::Image* ) &overheadVec[0] );

    // We're going to overwrite whatever _data value we stored the image with, so
    // tell OSG to *not* try to delete that pointer, as it's almost certainly not
    // valid any longer (or if it is, it's pointing to the data we want to use!).
    image->setAllocationMode( osg::Image::NO_DELETE );

    // Get the image data block.
    const DBRawCharVec& dataVec( persist->GetDatumValue< DBRawCharVec >( dataKey ) );
    image->setImage( image->s(), image->t(), image->r(),
                     image->getInternalTextureFormat(), image->getPixelFormat(),
                     image->getDataType(),
                     ( unsigned char* ) &dataVec[0],
                     osg::Image::NO_DELETE, image->getPacking() );

    return( image );
}



#if 0
// This is just too much.
class RawMemoryArray : public osg::Array
{
public:
    RawMemoryArray( const osg::Array::Type arrayType, const GLint dataSize, const GLenum dataType )
        : osg::Array( arrayType, dataSize, dataType )
    {}
    RawMemoryArray( const RawMemoryArray& rhs )
        : osg::Array( rhs ),
          _address( rhs._address ),
          _size( rhs._size )
    {}

    virtual void accept( osg::ArrayVisitor& av )
    {
        av.apply( *this );
    }
    virtual void accept( osg::ConstArrayVisitor& cav ) const
    {
        cav.apply( *this );
    }
    virtual void accept( unsigned int index, osg::ValueVisitor& vv )
    {
        switch( _arrayType )
        {
        case osg::Array::ByteArrayType:
            vv.apply( ( ( GLbyte* )_address )[ index ] );
            break;
        case osg::Array::FloatArrayType:
            vv.apply( ( ( GLfloat* )_address )[ index ] );
            break;
        case osg::Array::Vec2ArrayType:
            vv.apply( ( ( osg::Vec2* )_address )[ index ] );
            break;
        case osg::Array::Vec3ArrayType:
            vv.apply( ( ( osg::Vec3* )_address )[ index ] );
            break;
        case osg::Array::Vec4ArrayType:
            vv.apply( ( ( osg::Vec4* )_address )[ index ] );
            break;
        }
    }
    virtual void accept( unsigned int index, osg::ConstValueVisitor& cvv ) const
    {
        switch( _arrayType )
        {
        case osg::Array::ByteArrayType:
            cvv.apply( ( ( GLbyte* )_address )[ index ] );
            break;
        case osg::Array::FloatArrayType:
            cvv.apply( ( ( GLfloat* )_address )[ index ] );
            break;
        case osg::Array::Vec2ArrayType:
            cvv.apply( ( ( osg::Vec2* )_address )[ index ] );
            break;
        case osg::Array::Vec3ArrayType:
            cvv.apply( ( ( osg::Vec3* )_address )[ index ] );
            break;
        case osg::Array::Vec4ArrayType:
            cvv.apply( ( ( osg::Vec4* )_address )[ index ] );
            break;
        }
    }

    /** Return -1 if lhs element is less than rhs element, 0 if equal,
        * 1 if lhs element is greater than rhs element. */
    virtual int compare( unsigned int lhs, unsigned int rhs ) const
    {
        switch( _arrayType )
        {
        case osg::Array::ByteArrayType:
        {
            const GLbyte leftVal( ( ( GLbyte* )_address )[ lhs ] );
            const GLbyte rightVal( ( ( GLbyte* )_address )[ rhs ] );
            if( leftVal < rightVal )
            {
                return( -1 );
            }
            else if( leftVal > rightVal )
            {
                return( 1 );
            }
            else
            {
                return( 0 );
            }
            break;
        }
        case osg::Array::FloatArrayType:
        {
            const GLfloat leftVal( ( ( GLfloat* )_address )[ lhs ] );
            const GLfloat rightVal( ( ( GLfloat* )_address )[ rhs ] );
            if( leftVal < rightVal )
            {
                return( -1 );
            }
            else if( leftVal > rightVal )
            {
                return( 1 );
            }
            else
            {
                return( 0 );
            }
            break;
        }
        case osg::Array::Vec2ArrayType:
        {
            const osg::Vec2 leftVal( ( ( osg::Vec2* )_address )[ lhs ] );
            const osg::Vec2 rightVal( ( ( osg::Vec2* )_address )[ rhs ] );
            if( leftVal < rightVal )
            {
                return( -1 );
            }
            else if( leftVal > rightVal )
            {
                return( 1 );
            }
            else
            {
                return( 0 );
            }
            break;
        }
        case osg::Array::Vec3ArrayType:
        {
            const osg::Vec3 leftVal( ( ( osg::Vec3* )_address )[ lhs ] );
            const osg::Vec3 rightVal( ( ( osg::Vec3* )_address )[ rhs ] );
            if( leftVal < rightVal )
            {
                return( -1 );
            }
            else if( leftVal > rightVal )
            {
                return( 1 );
            }
            else
            {
                return( 0 );
            }
            break;
        }
        case osg::Array::Vec4ArrayType:
        {
            const osg::Vec4 leftVal( ( ( osg::Vec4* )_address )[ lhs ] );
            const osg::Vec4 rightVal( ( ( osg::Vec4* )_address )[ rhs ] );
            if( leftVal < rightVal )
            {
                return( -1 );
            }
            else if( leftVal > rightVal )
            {
                return( 1 );
            }
            else
            {
                return( 0 );
            }
            break;
        }
        }
    }

    virtual const GLvoid* getDataPointer() const
    {
        return( _address );
    }
    virtual unsigned int getTotalDataSize() const
    {
        switch( _arrayType )
        {
        case osg::Array::ByteArrayType:
            return( sizeof( GLbyte ) * _size );
            break;
        case osg::Array::FloatArrayType:
            return( sizeof( GLfloat ) * _size );
            break;
        case osg::Array::Vec2ArrayType:
            return( sizeof( osg::Vec2 ) * _size );
            break;
        case osg::Array::Vec3ArrayType:
            return( sizeof( osg::Vec3 ) * _size );
            break;
        case osg::Array::Vec4ArrayType:
            return( sizeof( osg::Vec4 ) * _size );
            break;
        }
    }
    virtual unsigned int    getNumElements() const
    {
        return( _size );
    }

protected:
    virtual ~RawMemoryArray() {}

    void* _address;
    size_t _size;
};
#endif

bool storeArray( const osg::Array* array, const DBKey& dbKey )
{
    db::PersistablePtr persist( s_getPersistable() );

    {
        // Create an STL allocator that allocates memory using the address
        // stored in 'array'. No actual allocation is done.
        const size_t sz( sizeof( *array ) );
        RefCharAllocator localAllocator(
            RefCharAllocator::RefPtr( const_cast< osg::Array* >( array ) ), sz );

        // Create std::vector<char> for storing in DB. We use the custom
        // allocator to avoid the data copy. A simple resize()
        // changes the internal size member var and the array is immediately
        // filled with the storage referenced by 'array'.
        DBRefCharVec cv( localAllocator );
        cv.resize( sz );

        // Add to database. This mean's we now own a copy of this memory,
        // but it is not stored in a ref_ptr. So, to ensure it isn't deleted
        // when the last ref_ptr goes away, do an explicit call to ref().
        const DBKey key( dbKey + std::string( "-arrayOverhead" ) );
        if( persist->DatumExists( key ) )
        {
            persist->SetDatumValue( key, cv );
        }
        else
        {
            persist->AddDatum( key, cv );
        }
        array->ref();
    }

    // Now do essentially the same thing with the Array's data, but no need
    // for a RefPtrAllocator. Instead use a RawMemoryAllocator to avoid
    // expensive data copy.
    {
        const size_t sz( array->getTotalDataSize() );
        RawCharAllocator localAllocator(
            ( RawCharAllocator::pointer )( array->getDataPointer() ), sz );

        DBRawCharVec cv( localAllocator );
        cv.resize( sz );

        const DBKey key( dbKey + std::string( "-arrayData" ) );
        if( persist->DatumExists( key ) )
        {
            persist->SetDatumValue( key, cv );
        }
        else
        {
            persist->AddDatum( key, cv );
        }
    }

    return( true );
}
osg::Array* loadArray( const DBKey& dbKey )
{
    db::PersistablePtr persist( s_getPersistable() );

    const DBKey overheadKey( dbKey + std::string( "-arrayOverhead" ) );
    const DBKey dataKey( dbKey + std::string( "-arrayData" ) );
    if( !( persist->DatumExists( overheadKey ) ) ||
            !( persist->DatumExists( dataKey ) ) )
    {
        LFX_WARNING_STATIC( "lfx.core.db", "loadArray() failed to find key " + dbKey );
        return( NULL );
    }

    // Get the Array overhead block.
    const DBRefCharVec& overheadVec( persist->GetDatumValue< DBRefCharVec >( overheadKey ) );
    osg::Array* array( ( osg::Array* ) &overheadVec[0] );

    // Get the array data block.
    const DBRawCharVec& dataVec( persist->GetDatumValue< DBRawCharVec >( dataKey ) );

    // TBD
    // If there's a way to avoid a data copy here, it's going to be dang complicated.
    osg::ref_ptr< osg::Array > returnArray;
    switch( array->getType() )
    {
    case osg::Array::ByteArrayType:
        returnArray = new osg::ByteArray( array->getNumElements(), ( GLbyte* )&dataVec[0] );
        break;
    case osg::Array::FloatArrayType:
        returnArray = new osg::FloatArray( array->getNumElements(), ( GLfloat* )&dataVec[0] );
        break;
    case osg::Array::Vec2ArrayType:
        returnArray = new osg::Vec2Array( array->getNumElements(), ( osg::Vec2* )&dataVec[0] );
        break;
    case osg::Array::Vec3ArrayType:
        returnArray = new osg::Vec3Array( array->getNumElements(), ( osg::Vec3* )&dataVec[0] );
        break;
    case osg::Array::Vec4ArrayType:
        returnArray = new osg::Vec4Array( array->getNumElements(), ( osg::Vec4* )&dataVec[0] );
        break;
    default:
        break;
    }

    return( returnArray.release() );
}


#else


PersistPtr s_persist( ( void* )NULL );

void s_setPersistable( PersistPtr persist )
{
    s_persist = persist;
}
PersistPtr s_getPersistable()
{
    return( s_persist );
}

bool storeImage( const osg::Image* image, const DBKey& dbKey )
{
    return( osgDB::writeImageFile( *image, dbKey ) );
}
osg::Image* loadImage( const DBKey& dbKey )
{
    return( osgDB::readImageFile( dbKey ) );
}

bool storeArray( const osg::Array* array, const DBKey& dbKey )
{
    return( osgDB::writeObjectFile( *array, dbKey ) );
}
osg::Array* loadArray( const DBKey& dbKey )
{
    return( dynamic_cast< osg::Array* >( osgDB::readObjectFile( dbKey ) ) );
}


#endif


// core
}
// lfx
}
