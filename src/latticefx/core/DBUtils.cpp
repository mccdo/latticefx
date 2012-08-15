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

#include <latticefx/core/DBUtils.h>
#include <latticefx/core/LogMacros.h>

#ifdef LFX_USE_CRUNCHSTORE
#  include <Persistence/Persistable.h>
#else
#  include <osgDB/ReadFile>
#  include <osgDB/WriteFile>
#endif

#include <iomanip>
#include <sstream>


namespace lfx {
namespace core {



DBKey generateDBKey()
{
    static unsigned int keyCounter( 0 );

    std::ostringstream ostr;
    ostr << "dbKey" << std::setfill( '0' ) <<
        std::setw( 5 ) << keyCounter++;
#ifndef LFX_USE_CRUNCHSTORE
    ostr << ".ive";
#endif
    return( DBKey( ostr.str() ) );
}


#ifdef LFX_USE_CRUNCHSTORE


namespace db = Persistence;


PersistPtr s_persist( db::PersistablePtr( (db::Persistable*)NULL ) );

void s_setPersistable( PersistPtr persist )
{
    s_persist = persist;
}
PersistPtr s_getPersistable()
{
    if( s_persist == NULL )
        s_persist = db::PersistablePtr( new db::Persistable );
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
    struct rebind {
        typedef RefPtrAllocator<U> other;
    };

    // return address of values
    pointer address (reference value) const {
        return &value;
    }
    const_pointer address (const_reference value) const {
        return &value;
    }


    typedef osg::ref_ptr< osg::Referenced > RefPtr;

    /* constructors and destructor
    */
    RefPtrAllocator( RefPtr refAddress=NULL, const size_type size=0 )
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
    size_type max_size () const throw() {
        return( _size );
    }

    // allocate but don't initialize num elements of type T
    pointer allocate (size_type num, const void* = 0)
    {
        // print message and allocate memory with global new
        //std::cerr << "allocate " << num << " element(s)" << " of size " << sizeof(T) << std::endl;
        //std::cerr << "  _refAddress " << (void*)_refAddress.get() << std::endl;
        pointer ret;
        if( _refAddress == NULL )
            ret = (pointer)(::operator new(num*sizeof(T)));
        else
            ret = (pointer)(_refAddress.get());
        //std::cerr << " allocated at: " << (void*)ret << std::endl;
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct (pointer p, const T& value) {
        // initialize memory with placement new
        if( ( p < (pointer)(_refAddress.get()) ) || ( p >= (pointer)(_refAddress.get()) + _size ) )
            new((void*)p)T(value);
    }

    // destroy elements of initialized storage p
    void destroy (pointer p) {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate (pointer p, size_type num) {
        // print message and deallocate memory with global delete
        //std::cerr << "deallocate " << num << " element(s)" << " of size " << sizeof(T) << " at: " << (void*)p << std::endl;
        //std::cerr << "  _refAddress " << (void*)_refAddress.get() << std::endl;
        if( p != (pointer)(_refAddress.get()) )
            ::operator delete((void*)p);
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
bool operator== (const RefPtrAllocator<T1>&,
    const RefPtrAllocator<T2>&) throw() {
        return true;
}
template <class T1, class T2>
bool operator!= (const RefPtrAllocator<T1>&,
    const RefPtrAllocator<T2>&) throw() {
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
    struct rebind {
        typedef RawMemoryAllocator<U> other;
    };

    // return address of values
    pointer address (reference value) const {
        return &value;
    }
    const_pointer address (const_reference value) const {
        return &value;
    }


    /* constructors and destructor
    */
    RawMemoryAllocator( pointer address=(pointer)(NULL), const size_type size=0 )
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
      : _size( 0 )
    {
    }
    ~RawMemoryAllocator()
    {
    }

    // return maximum number of elements that can be allocated
    size_type max_size () const throw() {
        return( _size );
    }

    // allocate but don't initialize num elements of type T
    pointer allocate (size_type num, const void* = 0)
    {
        // print message and allocate memory with global new
        //std::cerr << "allocate " << num << " element(s)" << " of size " << sizeof(T) << std::endl;
        //std::cerr << "  _address " << (void*)_address << std::endl;
        pointer ret;
        if( _address == NULL )
            ret = (pointer)(::operator new(num*sizeof(T)));
        else
            ret = _address;
        //std::cerr << " allocated at: " << (void*)ret << std::endl;
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct (pointer p, const T& value) {
        // initialize memory with placement new
        if( ( p < _address ) || ( p >= _address + _size ) )
            new((void*)p)T(value);
    }

    // destroy elements of initialized storage p
    void destroy (pointer p) {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate (pointer p, size_type num) {
        // print message and deallocate memory with global delete
        //std::cerr << "deallocate " << num << " element(s)" << " of size " << sizeof(T) << " at: " << (void*)p << std::endl;
        //std::cerr << "  _address " << (void*)_address << std::endl;
        if( p != _address )
            ::operator delete((void*)p);
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
bool operator== (const RawMemoryAllocator<T1>&,
    const RawMemoryAllocator<T2>&) throw() {
        return true;
}
template <class T1, class T2>
bool operator!= (const RawMemoryAllocator<T1>&,
    const RawMemoryAllocator<T2>&) throw() {
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
            RefCharAllocator::RefPtr( const_cast< osg::Image* >(image) ), sz );

        // Create std::vector<char> for storing in DB. We use the custom
        // allocator to avoid an expensive data copy. A simple resize()
        // changes the internal size member var and the array is immediately
        // filled the the storage referenced by 'image'.
        DBRefCharVec cv( localAllocator );
        cv.resize( sz );

        // Add to database. This mean's we now own a copy of this memory,
        // but it is not stored in a ref_ptr. So, to ensure it isn't deleted
        // when the last ref_ptr goes away, do an explicit call to ref().
        persist->AddDatum( dbKey+std::string("-imageoverhead"), cv );
        image->ref();
    }

    // Now do essentially the same thing with the Image's data, but no need
    // for a RefPtrAllocator. Instead use a RawMemoryAllocator to avoid
    // initial data copy.
    {
        const size_t sz( image->getTotalSizeInBytes() );
        RawCharAllocator localAllocator(
            (RawCharAllocator::pointer)( image->data() ), sz );

        DBRawCharVec cv( localAllocator );
        cv.resize( sz );

        persist->AddDatum( dbKey+std::string("-imagedata"), cv );
    }

    return( true );
}
osg::Image* loadImage( const DBKey& dbKey )
{
    db::PersistablePtr persist( s_getPersistable() );

    const DBKey overheadKey( dbKey+std::string("-imageoverhead") );
    const DBKey dataKey( dbKey+std::string("-imagedata") );
    if( !( persist->DatumExists( overheadKey ) ) ||
        !( persist->DatumExists( dataKey ) ) )
        return( NULL );

    // We do not need to use a ref_ptr here, or explicitly call ref(),
    // because the Image object should already have a ref count > 0
    // from when it was first stored in the DB.

    // Get the Image overhead block.
    const DBRefCharVec& overheadVec( persist->GetDatumValue< DBRefCharVec >( overheadKey ) );
    osg::Image* image( (osg::Image*) &overheadVec[0] );

    // Get the image data block.
    const DBRawCharVec& dataVec( persist->GetDatumValue< DBRawCharVec >( dataKey ) );
    // Funny. osg::Image has a setData() function that just sets the pointer,
    // but it's protected; we have to use the full-blown setImage() instead.
    image->setImage( image->s(), image->t(), image->r(),
        image->getInternalTextureFormat(), image->getPixelFormat(),
        image->getDataType(),
        (unsigned char*) &dataVec[0],
        osg::Image::NO_DELETE, image->getPacking() );

    return( image );
}


#else


PersistPtr s_persist( (void*)NULL );

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

#endif


// core
}
// lfx
}
