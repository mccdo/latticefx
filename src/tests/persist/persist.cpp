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

#include <crunchstore/Persistable.h>
#include <latticefx/core/DBUtils.h>
//#include <latticefx/core/ChannelDataDB.h>
#include <osg/Array>
#include <osg/Notify>
#include <osg/io_utils>

#include <iostream>



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



int main( int argc, char** argv )
{
    crunchstore::PersistablePtr persist( new crunchstore::Persistable );

    typedef RefPtrAllocator< char > RefCharAllocator;
    typedef std::vector< char, RefCharAllocator > CharVec;

    OSG_ALWAYS << "--- Custom allocator, no DB ---" << std::endl;

    {
        osg::ref_ptr< osg::FloatArray > array( new osg::FloatArray );
        array->push_back( 1.f );
        array->push_back( 3.14f );
        array->push_back( 9.99999f );

        const size_t sz( sizeof( *array ) );
        RefCharAllocator localAllocator(
            RefCharAllocator::RefPtr( array.get() ), sz );

        CharVec cv( localAllocator );
        cv.resize( sz );

        osg::ref_ptr< osg::FloatArray > a2( (osg::FloatArray*)&cv[0] );

        for( unsigned int idx=0; idx<array->size(); ++idx )
            OSG_ALWAYS << (*a2)[ idx ] << std::endl;
    }

    OSG_ALWAYS << "--- Custom allocator, store in DB ---" << std::endl;

    {
        osg::ref_ptr< osg::FloatArray > array( new osg::FloatArray );
        array->push_back( 1.f );
        array->push_back( 3.14f );
        array->push_back( 9.99999f );

        const size_t sz( sizeof( *array ) );
        RefCharAllocator localAllocator(
            RefCharAllocator::RefPtr( array.get() ), sz );

        CharVec cv( localAllocator );
        cv.resize( sz );
        persist->AddDatum( "array", cv );
        // Extra ref now that the data is stored in the DB.
        array->ref();
    }
    {
        const CharVec& scv( persist->GetDatumValue< CharVec >( "array" ) );

        const size_t sz( scv.size() );
        osg::ref_ptr< osg::FloatArray > array( (osg::FloatArray*)&scv[0] );

        for( unsigned int idx=0; idx<array->size(); ++idx )
            OSG_ALWAYS << (*array)[ idx ] << std::endl;
    }

    OSG_ALWAYS << "--- Vec3Array test ---" << std::endl;

    {
        osg::ref_ptr< osg::Vec3Array > array( new osg::Vec3Array );
        array->push_back( osg::Vec3( 1., 2., 3. ) );
        array->push_back( osg::Vec3( 4., 5., 6. ) );
        array->push_back( osg::Vec3( 7., 8., 9. ) );
        array->push_back( osg::Vec3( 10., 11., 12. ) );

        const size_t sz( sizeof( *array ) );
        RefCharAllocator localAllocator(
            RefCharAllocator::RefPtr( array.get() ), sz );

        CharVec cv( localAllocator );
        cv.resize( sz );
        persist->AddDatum( "v3array", cv );
        // Extra ref now that the data is stored in the DB.
        array->ref();
    }
    {
        const CharVec& scv( persist->GetDatumValue< CharVec >( "v3array" ) );

        const size_t sz( scv.size() );
        osg::ref_ptr< osg::Vec3Array > array( (osg::Vec3Array*)&scv[0] );

        for( unsigned int idx=0; idx<array->size(); ++idx )
            OSG_ALWAYS << (*array)[ idx ] << std::endl;
    }

    OSG_ALWAYS << "--- Vec4Array and DBUtils test ---" << std::endl;

    {
        osg::ref_ptr< osg::Vec4Array > array( new osg::Vec4Array );
        array->push_back( osg::Vec4( 1., 2., 3., 1. ) );
        array->push_back( osg::Vec4( 4., 5., 6., 1. ) );
        array->push_back( osg::Vec4( 7., 8., 9., 1. ) );
        array->push_back( osg::Vec4( 10., 11., 12., 1. ) );

        lfx::core::storeArray( array.get(), "foo" );
    }
    {
        osg::ref_ptr< osg::Array > array( lfx::core::loadArray( "foo" ) );
        osg::Vec4Array* v4a( dynamic_cast< osg::Vec4Array* >( array.get() ) );

        for( unsigned int idx=0; idx<v4a->size(); ++idx )
            OSG_ALWAYS << (*v4a)[ idx ] << std::endl;
    }

    OSG_ALWAYS << "--- Finished ---" << std::endl;
}
