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
#ifndef LFX_CUTTING_PLANE
#define LFX_CUTTING_PLANE

#include "vtkSmartPointer.h"

class vtkPlane;
class vtkDataSet;

/*!\file CuttingPlane.h
 * CuttingPlane API
 * \class lfx::core::vtk::CuttingPlane
 *
 */
class CuttingPlane
{
public:
    typedef enum
    {
        X_PLANE,
        Y_PLANE,
        Z_PLANE
    } SliceDirection;

    ///Constructor
    CuttingPlane( const double* bounds, SliceDirection xyz, const int numSteps = 10 );

    void SetBounds( const double* bounds );

    vtkSmartPointer<vtkPlane> GetPlane();

    ///This reads in the requested value for the cutting plane.
    ///\param requestedValue
    void Advance( double requestedValue );

    ///This reads in the requested value for the cutting plane.
    ///\param Origin
    void GetOrigin( double Origin[ 3 ] );

private:
    double origin[3];    // Position of cut.

    ///This computes the origin as requested from the user.
    ///\param requestedValue
    void ComputeOrigin( double requestedValue );

    ///Used to test if past range.
    int isPastEnd();

    ///Used to test if at end of range.
    int isAtEnd();

    ///Used to test if at start of range.
    int isAtStart();

    ///Used to reset the origin to low value.
    void ResetOriginToLow();

    ///Used to reset the origin to high value.
    void ResetOriginToHigh();

    ///Used to advance the origin.
    void IncrementOrigin();

    vtkSmartPointer<vtkPlane> plane;///<Plane for vtk.

    double normal[3];///<Normal direction to cut.

    double bd[6];///<Boundary of the whole data sets.

    float dx;///<Used only by blue menu.

    SliceDirection type;///<Plane direction: 0=X, 1=Y, 2=Z.
};

#endif
