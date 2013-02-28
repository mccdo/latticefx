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

#include <crunchstore/Persistable.h>
#include <crunchstore/DataManager.h>
#include <crunchstore/NullCache.h>
#include <crunchstore/NullBuffer.h>
#include <crunchstore/SQLiteStore.h>

#include <latticefx/core/DBDisk.h>
#include <latticefx/core/DBCrunchStore.h>

#include <osg/Array>
#include <osg/Notify>
#include <osg/io_utils>

#include <Poco/File.h>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
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
    typedef RefPtrAllocator< char > RefCharAllocator;
    typedef std::vector< char, RefCharAllocator > CharVec;


    std::cout << "--- Custom allocator, no DB ---" << std::endl;

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
            std::cout << (*a2)[ idx ] << std::endl;
    }


    std::cout << "--- Custom allocator, store in DB ---" << std::endl;

    {
        namespace csdb = crunchstore;
        csdb::PersistablePtr persist( csdb::PersistablePtr( new csdb::Persistable() ) );

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
                std::cout << (*array)[ idx ] << std::endl;
        }
    }


    std::cout << "--- Vec3Array test ---" << std::endl;

    {
        namespace csdb = crunchstore;
        csdb::PersistablePtr persist( csdb::PersistablePtr( new csdb::Persistable() ) );

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
                std::cout << (*array)[ idx ] << std::endl;
        }
    }


    std::cout << "--- Save and restore DB test ---" << std::endl;

    {
        namespace csdb = crunchstore;

        const std::string dbName( "SaveLoadTest.db" );
        Poco::File pocoFile( dbName );
        if( pocoFile.exists() )
            pocoFile.remove();

        csdb::DataManager manager;
        csdb::DataAbstractionLayerPtr cache( new csdb::NullCache );
        csdb::DataAbstractionLayerPtr buffer( new csdb::NullBuffer );
        manager.SetCache( cache );
        manager.SetBuffer( buffer );
        csdb::SQLiteStorePtr sqstore( new csdb::SQLiteStore );
        sqstore->SetStorePath( dbName );
        manager.AttachStore( sqstore, csdb::Store::BACKINGSTORE_ROLE );

        typedef std::vector<int> IntVec;
        typedef std::vector<char> CharVec;
        boost::uuids::uuid uuID;
        {
            // Save a Persistable into the SQLiteStore.
            csdb::Persistable q;
            uuID = q.GetUUID();
            q.SetTypeName( "TestType" );

            IntVec iv;
            iv.push_back( 13 ); iv.push_back( 42 ); iv.push_back( 77 );
            const char* start( (char*)&(*iv.begin()) );
            CharVec cv( start, start + iv.size() * sizeof(int) );

            q.AddDatum( "header", (int)cv.size() );
            q.AddDatum( "data", cv );
            manager.Save( q );
        }
        {
            // Create a Persistable with the same UUID and typename.
            csdb::Persistable q;
            q.SetUUID( uuID );
            q.SetTypeName( "TestType" );

            // Add datums to load
            q.AddDatum( "header", (int)0 );
            q.AddDatum( "data", (char*)NULL );
            // Load the Persistable from the SQLiteStore.
            manager.Load( q );
            // Prove that we loaded the data.
            if( q.DatumExists( "header" ) )
            {
                int d( q.GetDatumValue<int>( "header" ) );
                std::cout << "Found header: " << d << std::endl;;

                CharVec cv( q.GetDatumValue<CharVec>( "data" ) );
                const int* start( (int*)&(*cv.begin()) );
                IntVec iv( start, start + cv.size() / sizeof(int) );
                std::cout << "  iv[0]: " << iv[0] << std::endl;;
                std::cout << "  iv[1]: " << iv[1] << std::endl;;
                std::cout << "  iv[2]: " << iv[2] << std::endl;;
            }
        }
    }


    std::cout << "--- Query empty DB test ---" << std::endl;

    {
        namespace csdb = crunchstore;

        const std::string dbName( "NonExisting.db" );
        Poco::File pocoFile( dbName );
        if( pocoFile.exists() )
            pocoFile.remove();

        csdb::DataManager manager;
        csdb::DataAbstractionLayerPtr cache( new csdb::NullCache );
        csdb::DataAbstractionLayerPtr buffer( new csdb::NullBuffer );
        manager.SetCache( cache );
        manager.SetBuffer( buffer );
        csdb::SQLiteStorePtr sqstore( new csdb::SQLiteStore );
        sqstore->SetStorePath( dbName );
        manager.AttachStore( sqstore, csdb::Store::BACKINGSTORE_ROLE );

        if( !manager.HasIDForTypename( boost::uuids::random_generator()(), "DummyTypeName" ) )
            std::cout << "Correct results: Can't find DummyTypeName" << std::endl;
        else
            std::cout << "Failed." << std::endl;
    }


    std::cout << "--- Query saved ID/typename test ---" << std::endl;

    {
        namespace csdb = crunchstore;

        const std::string dbName( "QueryTest.db" );
        const Poco::File pocoFile( dbName );
        const bool fileExists( pocoFile.exists() );

        csdb::DataManager manager;
        csdb::DataAbstractionLayerPtr cache( new csdb::NullCache );
        csdb::DataAbstractionLayerPtr buffer( new csdb::NullBuffer );
        manager.SetCache( cache );
        manager.SetBuffer( buffer );
        csdb::SQLiteStorePtr sqstore( new csdb::SQLiteStore );
        sqstore->SetStorePath( dbName );
        manager.AttachStore( sqstore, csdb::Store::BACKINGSTORE_ROLE );

        const std::string genericTypeName( "Generic" );
        const std::string knownID( "00000000-0000-0000-0000-000000000000" );

        if( !fileExists )
        {
            // Save a Persistable into the SQLiteStore.
            csdb::Persistable persist( genericTypeName );
            persist.SetUUID( knownID );
            persist.AddDatum( "Number", 0.0 );
            manager.Save( persist );

            std::cout << "Saved ID/TypeName. Re-run to test query." << std::endl;
        }
        else
        {
            // Now query to see if it exists
            boost::uuids::string_generator strGen;
            boost::uuids::uuid knownUUID( strGen( knownID ) );
            if( manager.HasIDForTypename( knownUUID, genericTypeName ) )
                std::cout << "Correct results: Found ID/TypeName." << std::endl;
            else
                std::cout << "Failed: HasIDForTypename returned false." << std::endl;
        }
    }


    std::cout << "--- Image save / restore test ---" << std::endl;

    for( unsigned int testNum=0; testNum<2; ++testNum )
    {
        using namespace lfx::core;
        namespace csdb = crunchstore;

        DBKey key;
        bool dbExists;
        DBBasePtr dbBase;
        if( testNum == 0 )
        {
            DBDiskPtr dbDisk( DBDiskPtr( new DBDisk() ) );
            dbBase = dbDisk;

            key = dbBase->generateDBKey( "ImageTestKey" );
            Poco::File pocoFile( key );
            dbExists = pocoFile.exists();
        }
        else
        {
            const std::string dbName( "ImageTest.db" );
            Poco::File pocoFile( dbName );
            dbExists = pocoFile.exists();

            csdb::DataManagerPtr manager( new csdb::DataManager() );
            csdb::DataAbstractionLayerPtr cache( new csdb::NullCache );
            csdb::DataAbstractionLayerPtr buffer( new csdb::NullBuffer );
            manager->SetCache( cache );
            manager->SetBuffer( buffer );
            csdb::SQLiteStorePtr sqstore( new csdb::SQLiteStore );
            sqstore->SetStorePath( dbName );
            manager->AttachStore( sqstore, csdb::Store::BACKINGSTORE_ROLE );

            DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );
            try {
                cs->setDataManager( manager );
            }
            catch( std::exception exc ) {
                std::cout << "Test# " << testNum << ": setDataManager() throws exception." << std::endl;
                break;
            }
            dbBase = cs;

            key = dbBase->generateDBKey( "ImageTestKey" );
        }

        const int width( 6 ), height( 4 );
        if( !dbExists )
        {
            osg::ref_ptr< osg::Image > image( new osg::Image() );
            float* data = (float*) malloc( width * height * sizeof( float ) * 3 );
            float* ptr=data;
            float red[3] = { 1., 0., 0. };
            for( unsigned int idx=0; idx<width*height; ++idx )
            {
                memcpy( ptr, (void*)&red[0], sizeof(float)*3 );
                ptr += 3;
            }
            image->setImage( width, height, 1, GL_RGB, GL_RGB, GL_FLOAT,
                (unsigned char*)data, osg::Image::USE_MALLOC_FREE );

            dbBase->storeImage( image.get(), key );

            std::cout << "Test# " << testNum << ": Store test PASSED." << std::endl;
        }
        else
        {
            osg::ref_ptr< osg::Image > loadedImage( dbBase->loadImage( key ) );
            if( loadedImage == NULL )
            {
                std::cout << "Test# " << testNum << ": Failed, NULL loadedImage." << std::endl;
                continue;
            }
            if( ( loadedImage->s() != width ) || ( loadedImage->t() != height ) )
            {
                std::cout << "Test# " << testNum << ": Failed, Incorrect width/height." << std::endl;
                continue;
            }

            std::cout << "Test# " << testNum << ": Load test PASSED." << std::endl;
        }
    }


    std::cout << "--- Vec4Array and DBCrunchStore test ---" << std::endl;
    std::cout << "--- Not yet implemented ---" << std::endl;


    std::cout << "--- Finished ---" << std::endl;
}
