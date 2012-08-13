#include <Persistence/Persistable.h>
//#include <latticefx/core/ChannelDataDB.h>
#include <osg/Array>
#include <osg/Notify>

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

    /* constructors and destructor
    * - nothing to do because the allocator has no state
    */
    RefPtrAllocator() : _size( 0 ) {
    }
    RefPtrAllocator( const RefPtrAllocator& rhs )
      : _refAddress( rhs._refAddress ),
        _size( rhs._size )
    {
    }
    template <class U>
    RefPtrAllocator( const RefPtrAllocator<U>& rhs ) : _size( 0 ) {
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


    typedef osg::ref_ptr< osg::Referenced > RefPtr;

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
    Persistence::PersistablePtr persist( new Persistence::Persistable );

    OSG_ALWAYS << "--- Simple copy ---" << std::endl;

    typedef std::vector< char > SCharVec;
    {
        osg::ref_ptr< osg::FloatArray > array( new osg::FloatArray );
        array->push_back( 1.f );
        array->push_back( 3.14f );
        array->push_back( 9.99999f );

        const size_t sz( sizeof( *array ) );
        const char* start( (char*)(array.get()) );
        SCharVec scv( start, start+sz );
        persist->AddDatum( "arraycopy", scv );
    }
    {
        SCharVec& scv( persist->GetDatumValue< SCharVec >( "arraycopy" ) );

        const size_t sz( scv.size() );
        osg::ref_ptr< osg::FloatArray > array( (osg::FloatArray*)&scv[0] );
        array->ref();

        OSG_ALWAYS << (*array)[0] << std::endl;
        OSG_ALWAYS << (*array)[1] << std::endl;
        OSG_ALWAYS << (*array)[2] << std::endl;
    }

    typedef std::vector< char, RefPtrAllocator< char > > CharVec;

    OSG_ALWAYS << "--- Custom allocator, no DB ---" << std::endl;

    {
        RefPtrAllocator< char > localAllocator;

        {
            osg::ref_ptr< osg::FloatArray > array( new osg::FloatArray );
            array->push_back( 1.f );
            array->push_back( 3.14f );
            array->push_back( 9.99999f );

            const size_t sz( sizeof( *array ) );
            localAllocator.setAddress( RefPtrAllocator< char >::RefPtr( array.get() ), sz );

            CharVec cv( localAllocator );
            cv.resize( sz );

            osg::ref_ptr< osg::FloatArray > a2( (osg::FloatArray*)&cv[0] );

            OSG_ALWAYS << (*a2)[0] << std::endl;
            OSG_ALWAYS << (*a2)[1] << std::endl;
            OSG_ALWAYS << (*a2)[2] << std::endl;
        }
    }

    OSG_ALWAYS << "--- Custom allocator, store in DB ---" << std::endl;

    {
        RefPtrAllocator< char > localAllocator;

        {
            osg::ref_ptr< osg::FloatArray > array( new osg::FloatArray );
            array->push_back( 1.f );
            array->push_back( 3.14f );
            array->push_back( 9.99999f );

            const size_t sz( sizeof( *array ) );
            localAllocator.setAddress( RefPtrAllocator< char >::RefPtr( array.get() ), sz );

            CharVec cv( localAllocator );
            cv.resize( sz );
            persist->AddDatum( "array", cv );
        }
        {
            CharVec& scv( persist->GetDatumValue< CharVec >( "array" ) );

            const size_t sz( scv.size() );
            osg::ref_ptr< osg::FloatArray > array( (osg::FloatArray*)&scv[0] );
            array->ref();

            OSG_ALWAYS << (*array)[0] << std::endl;
            OSG_ALWAYS << (*array)[1] << std::endl;
            OSG_ALWAYS << (*array)[2] << std::endl;
        }
    }

    OSG_ALWAYS << "--- Finished ---" << std::endl;
}
