/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
/**
 * \class vtkSmartPtr vtkSmartPtr.h <latticefx/utils/vtk/vtkSmartPtr.h>
 *
 * \brief Class that handles the management of vtk data types.
 * \details Here is how it is used:
 *
 * \code
 * using namespace lfx::vtk_utils;
 * vtkSmartPtr< vtkType > ptr;
 * \endcode
 * Where \c vtkType is some VTK Class.  So,
 * vTKSmartPTr<vtkSocketCommunicator> sock;
 * would make a new vtkSocketCommunicator* and initialize it with
 * vtkSocketCommunicator::New();
 * Now for the cool part; when this pointer goes out of scope, it will automatically
 * call vtkSocketCommunicator::Delete();
 * Also, if you pass this smart ptr to another function that takes an argument
 * of type vtkSmartPtr, it will increment the reference count, so your objects
 * never delete themselves until all references are out of scope.
 * Some vtk classes are "special".  They don't have a vtk::New() method because
 * they're only returned from getter functions (ie, vtkUnstructuredGrid); on
 * these classes, vtkSmartPtr simply initializes the ptr to NULL; any attempt
 * to reference a NULL pointer will result in an ASSERT failure.  You can reset the
 * ptr using Reset(vTKSmarPtr<vtkType>, vtkType*);
 */

////////////////////////////////////////////////////////////////////////////////
// This code is based upon code written for:
//
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author or Addison-Wesley Longman make no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
// Author: Johnathan Gurley
////////////////////////////////////////////////////////////////////////////////

#ifndef _VTKUTIL_VTK_SMART_PTR_H_
#define _VTKUTIL_VTK_SMART_PTR_H_

#include <cassert>
#include <algorithm>
namespace lfx
{
namespace vtk_utils
{

///Pulled from Loki to make some constructs work

/**
 * class template Select
 * Selects one of two types based upon a boolean constant
 * Invocation: Select<flag, T, U>::Result
 * where:
 * flag is a compile-time boolean constant
 * T and U are types
 * Result evaluates to T if flag is true, and to U otherwise.
 */
template <bool flag, typename T, typename U>
struct Select
{
    typedef T Result;
};
template <typename T, typename U>
struct Select<false, T, U>
{
    typedef U Result;
};

/**
 * class template ByRef
 * Transports a reference as a value
 * Serves to implement the Colvin/Gibbons trick for SmartPtr
 */
template <class T>
class ByRef
{
public:
    ByRef( T& v ) : value_( v )
    {}
    operator T&()
    {
        return value_;
    }
    // gcc doesn't like this:
    // operator const T&() const { return value_; }
private:
    ByRef& operator=( const ByRef & );
    T& value_;
};

///Make a StoragePolicy for a VTKObject
/**
 * This class adheres to the semantices of a loki::SmartPtr StoragePolicy
 * It allows for the use of SmartPtrs with VTKObjects
 */
template <class T>
class VTKStoragePolicy
{
public:
    typedef T* StoredType;
    typedef T* PointerType;
    typedef T& ReferenceType;

    VTKStoragePolicy() : pointee_( Default() )
    {}

    VTKStoragePolicy( const VTKStoragePolicy& )
    {}

    template <class U>
    VTKStoragePolicy( const VTKStoragePolicy<U>& )
    {}

    VTKStoragePolicy( const StoredType& p ) : pointee_( p )
    {}

    PointerType operator->() const
    {
        return pointee_;
    }

    ReferenceType operator*() const
    {
        return *pointee_;
    }

    void Swap( VTKStoragePolicy& rhs )
    {
        std::swap( pointee_, rhs.pointee_ );
    }

    // Accessors
    friend inline PointerType GetImpl( const VTKStoragePolicy& sp )
    {
        return sp.pointee_;
    }

    friend inline const StoredType& GetImplRef( const VTKStoragePolicy& sp )
    {
        return sp.pointee_;
    }

    friend inline StoredType& GetImplRef( VTKStoragePolicy& sp )
    {
        return sp.pointee_;
    }

protected:

    // Destroys the data stored
    // (Destruction might be taken over by the OwnershipPolicy)
    void Destroy()
    {
        pointee_->Delete();
    }

    // Default value to initialize the pointer
    static StoredType Default()
    {
        return T::New();
    }

private:

    // Data
    StoredType pointee_;
};

/**
 * class template RefCounted
 * Implementation of the OwnershipPolicy used by SmartPtr
 * Provides a classic external reference counting implementation
 */
template <class P>
class RefCounted
{
public:
    RefCounted()
            : pCount_( 0 )
    {
        pCount_ = new unsigned int;
        *pCount_ = 1;
    }

    RefCounted( const RefCounted& rhs )
            : pCount_( rhs.pCount_ )
    {}

    P Clone( const P& val )
    {
        ++*pCount_;
        return val;
    }

    bool Release( const P& )
    {
        if( !--*pCount_ )
        {
            delete pCount_;
            return true;
        }
        return false;
    }

    void Swap( RefCounted& rhs )
    {
        std::swap( pCount_, rhs.pCount_ );
    }

    enum { destructiveCopy = false };

private:
    // Data
    unsigned int* pCount_;
};

/**
 * class template AllowConversion
 * Implementation of the ConversionPolicy used by SmartPtr
 * Allows implicit conversion from SmartPtr to the pointee type
 */
struct AllowConversion
{
    enum { allow = true };

    void Swap( AllowConversion& )
    {}
};

/**
 * class template DisallowConversion
 * Implementation of the ConversionPolicy used by SmartPtr
 * Does not allow implicit conversion from SmartPtr to the pointee type
 * You can initialize a DisallowConversion with an AllowConversion
 */
struct DisallowConversion
{
    DisallowConversion()
    {}

    DisallowConversion( const AllowConversion& )
    {}

    enum { allow = false };

    void Swap( DisallowConversion& )
    {}
};

/**
 * class template NoCheck
 * Implementation of the CheckingPolicy used by SmartPtr
 * Well, it's clear what it does :o)
 */
template <class P>
struct NoCheck
{
    NoCheck()
    {}

    template <class P1>
    NoCheck( const NoCheck<P1>& )
    {}

    static void OnDefault( const P& )
    {}

    static void OnInit( const P& )
    {}

    static void OnDereference( const P& )
    {}

    static void Swap( NoCheck& )
    {}
};


/**
 * class template AssertCheck
 * Implementation of the CheckingPolicy used by SmartPtr
 * Checks the pointer before dereference
 */
template <class P>
struct AssertCheck
{
    AssertCheck()
    {}

    template <class P1>
    AssertCheck( const AssertCheck<P1>& )
    {}

    template <class P1>
    AssertCheck( const NoCheck<P1>& )
    {}

    static void OnDefault( const P& )
    {}

    static void OnInit( const P& )
    {}

    static void OnDereference( P val )
    {
        assert( val );
        ( void )val;
    }

    static void Swap( AssertCheck& )
    {}
};

/**
 * A complete rewrite (mostly cut and paste) of Loki::SmartPtr w/ the
 * exception that the default arguments are set to values for VTKObjects.
 * Yes this is bad coding practice, yes you shouldn't do this, but you
 * try and convince a group of mechanical engineers that they should
 * declare pointers as vtkSmartPtr<VTKObject>::type ptr and see how
 * far you get.
 * As an addendum to this, CC sucks and will not compile template template
 * parameters...gcc 2.95.3 doesn't seem to have a problem though :-\
 */
template
<
typename T,
class StoragePolicy = VTKStoragePolicy<T>,
class OwnershipPolicy = RefCounted< typename VTKStoragePolicy<T>::PointerType > ,
class ConversionPolicy = AllowConversion,
class CheckingPolicy = AssertCheck< typename VTKStoragePolicy<T>::StoredType >
>
class vtkSmartPtr
            : public StoragePolicy
            , public OwnershipPolicy
            , public CheckingPolicy
            , public ConversionPolicy
{
    typedef StoragePolicy SP;
    typedef OwnershipPolicy OP;
    typedef CheckingPolicy KP;
    typedef ConversionPolicy CP;

public:
    typedef typename SP::PointerType PointerType;
    typedef typename SP::StoredType StoredType;
    typedef typename SP::ReferenceType ReferenceType;

    typedef typename Select < OP::destructiveCopy,
    vtkSmartPtr, const vtkSmartPtr >::Result
    CopyArg;

    vtkSmartPtr()
    {
        KP::OnDefault( GetImpl( *this ) );
    }

    vtkSmartPtr( const StoredType& p ) : SP( p )
    {
        KP::OnInit( GetImpl( *this ) );
    }

    vtkSmartPtr( CopyArg& rhs )
            : SP( rhs ), OP( rhs ), KP( rhs ), CP( rhs )
    {
        GetImplRef( *this ) = OP::Clone( GetImplRef( rhs ) );
    }

    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    vtkSmartPtr( const vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs )
            : SP( rhs ), OP( rhs ), KP( rhs ), CP( rhs )
    {
        GetImplRef( *this ) = OP::Clone( GetImplRef( rhs ) );
    }

    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    vtkSmartPtr( vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs )
            : SP( rhs ), OP( rhs ), KP( rhs ), CP( rhs )
    {
        GetImplRef( *this ) = OP::Clone( GetImplRef( rhs ) );
    }

    vtkSmartPtr( ByRef<vtkSmartPtr> rhs )
            : SP( rhs ), OP( rhs ), KP( rhs ), CP( rhs )
    {}

    operator ByRef<vtkSmartPtr>()
    {
        return ByRef<vtkSmartPtr>( *this );
    }

    vtkSmartPtr& operator=( CopyArg& rhs )
    {
        vtkSmartPtr temp( rhs );
        temp.Swap( *this );
        return *this;
    }

    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    vtkSmartPtr& operator=( const vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs )
    {
        vtkSmartPtr temp( rhs );
        temp.Swap( *this );
        return *this;
    }

    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    vtkSmartPtr& operator=( vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs )
    {
        vtkSmartPtr temp( rhs );
        temp.Swap( *this );
        return *this;
    }

    void Swap( vtkSmartPtr& rhs )
    {
        OP::Swap( rhs );
        CP::Swap( rhs );
        KP::Swap( rhs );
        SP::Swap( rhs );
    }

    ~vtkSmartPtr()
    {
        if( OP::Release( GetImpl( *static_cast<SP*>( this ) ) ) )
        {
            SP::Destroy();
        }
    }

    friend inline void Release( vtkSmartPtr& sp, typename SP::StoredType& p )
    {
        p = GetImplRef( sp );
        GetImplRef( sp ) = SP::Default();
    }

    friend inline void Reset( vtkSmartPtr& sp, typename SP::StoredType p )
    {
        vtkSmartPtr( p ).Swap( sp );
    }

    PointerType operator->()
    {
        KP::OnDereference( GetImplRef( *this ) );
        return SP::operator->();
    }

    PointerType operator->() const
    {
        KP::OnDereference( GetImplRef( *this ) );
        return SP::operator->();
    }

    ReferenceType operator*()
    {
        KP::OnDereference( GetImplRef( *this ) );
        return SP::operator*();
    }

    ReferenceType operator*() const
    {
        KP::OnDereference( GetImplRef( *this ) );
        return SP::operator*();
    }

    bool operator!() const // Enables "if (!sp) ..."
    {
        return GetImpl( *this ) == 0;
    }

    inline friend bool operator==( const vtkSmartPtr& lhs,
                                   const T* rhs )
    {
        return GetImpl( lhs ) == rhs;
    }

    inline friend bool operator==( const T* lhs,
                                   const vtkSmartPtr& rhs )
    {
        return rhs == lhs;
    }

    inline friend bool operator!=( const vtkSmartPtr& lhs,
                                   const T* rhs )
    {
        return !( lhs == rhs );
    }

    inline friend bool operator!=( const T* lhs,
                                   const vtkSmartPtr& rhs )
    {
        return rhs != lhs;
    }

    // Ambiguity buster
    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    bool operator==( const vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs ) const
    {
        return *this == GetImpl( rhs );
    }

    // Ambiguity buster
    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    bool operator!=( const vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs ) const
    {
        return !( *this == rhs );
    }

    // Ambiguity buster
    template
    <
    typename T1,
    class OP1,
    class CP1,
    class KP1,
    class SP1
    >
    bool operator<( const vtkSmartPtr<T1, OP1, CP1, KP1, SP1>& rhs ) const
    {
        return *this < GetImpl( rhs );
    }

private:
    // Helper for enabling 'if (sp)'
    struct Tester
    {
        Tester()
        {}
private:
        void operator delete( void* );
    };

public:
    // enable 'if (sp)'
    operator Tester*() const
    {
        if( !*this ) return 0;
        static Tester t;
        return &t;
    }

private:
    // Helper for disallowing automatic conversion
    struct Insipid
    {
        Insipid( PointerType )
        {}
    };

    typedef typename Select<CP::allow, PointerType, Insipid>::Result AutomaticConversionResult;

public:
    operator AutomaticConversionResult() const
    {
        return GetImpl( *this );
    }
};
}
}
#endif
