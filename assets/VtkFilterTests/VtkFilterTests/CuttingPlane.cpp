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
#include "CuttingPlane.h"

#include <vtkPlane.h>


CuttingPlane::CuttingPlane( const double bounds[6], SliceDirection xyz,
                            const int numSteps )
    :
    type( xyz )
{
    // set the boundary of current data set.
    this->SetBounds( bounds );


    // specify the normal to sweep, the step-size, and the origin...
    if( type == X_PLANE )
    {
        this->normal[0] = 1.0f;
        this->normal[1] = 0.0f;
        this->normal[2] = 0.0f;
        this->dx = ( this->bd[1] - this->bd[0] ) / ( float )numSteps;
        //this->origin[0] = this->bd[0];
        this->origin[0] = ( this->bd[1] + this->bd[0] ) / 2.0;
        this->origin[1] = ( this->bd[3] + this->bd[2] ) / 2.0;
        this->origin[2] = ( this->bd[5] + this->bd[4] ) / 2.0;
    }
    else if( type == Y_PLANE )
    {
        this->normal[0] = 0.0f;
        this->normal[1] = 1.0f;
        this->normal[2] = 0.0f;
        this->dx = ( this->bd[3] - this->bd[2] ) / ( float )numSteps;
        this->origin[0] = ( this->bd[1] + this->bd[0] ) / 2.0;
        //this->origin[1] = this->bd[2];
        this->origin[1] = ( this->bd[3] + this->bd[2] ) / 2.0;
        this->origin[2] = ( this->bd[5] + this->bd[4] ) / 2.0;
    }
    else if( type == Z_PLANE )
    {
        this->normal[0] = 0.0f;
        this->normal[1] = 0.0f;
        this->normal[2] = 1.0f;
        this->dx = ( this->bd[5] - this->bd[4] ) / ( float )numSteps;
        this->origin[0] = ( this->bd[1] + this->bd[0] ) / 2.0;
        this->origin[1] = ( this->bd[3] + this->bd[2] ) / 2.0;
        //this->origin[2] = this->bd[4];
        this->origin[2] = ( this->bd[5] + this->bd[4] ) / 2.0;
    }
    else
    {
        std::cerr << "|\tin CuttingPlane type WAS NOT 0, 1, or 2!" << std::endl;
        exit( 1 );
    }

    //vprDEBUG( vesDBG, 1 ) << "|\t\tCuttingPlane origin = " << this->origin[0] << " : "
    //                      << this->origin[1] << " : " << this->origin[2]
    //                      << std::endl << vprDEBUG_FLUSH;
    this->plane = vtkSmartPointer<vtkPlane>::New();
    this->plane->SetOrigin( this->origin );
    this->plane->SetNormal( this->normal );

    // reset the origin
    this->ResetOriginToLow();
}

void CuttingPlane::SetBounds( const double* bounds )
{
    for( int i = 0; i < 6; i++ )
    {
        this->bd[i] = ( double )bounds[i];
    }
}

vtkSmartPointer<vtkPlane> CuttingPlane::GetPlane( )
{
    return this->plane;
}

void CuttingPlane::Advance( double requestedValue )
{
    // with tablet, requestedValue will be an integer from 0-100
    // with old menu system, requestedValue will be 999
    if( requestedValue == 999 )
    {
        // advance the origin
        this->IncrementOrigin();
    }
    else
    {
        // The jave slider bar returns integers 0-100 representing percentile.
        this->ComputeOrigin( requestedValue );

        // if too low error will occur, so reset close to bottom of range
        if( this->isAtStart() )
        {
            this->ResetOriginToLow();
        }
    }
    // if over the limit, reset close to bottom of range
    // if at the limit, reset close to end of range
    if( this->isPastEnd() )
    {
        this->ResetOriginToLow();
    }
    else if( this->isAtEnd() )
    {
        this->ResetOriginToHigh();
    }

    this->plane->SetOrigin( this->origin );
    this->plane->Modified();
    //this->plane->Update();
}

void CuttingPlane::GetOrigin( double Origin[ 3 ] )
{
    Origin[0] = this->origin[0];
    Origin[1] = this->origin[1];
    Origin[2] = this->origin[2];
}

void CuttingPlane::ComputeOrigin( double requestedValue )
{
    if( requestedValue < 0 )
    {
        requestedValue = 0;
    }
    if( requestedValue > 100 )
    {
        requestedValue = 100;
    }

    // bd is an array of 6 values xmin, xmax, ymin, ymax, zmin, zmax
    // bd is calculated from the raw DataSet NOT from the precalc values
    // type is either 0,1,2 representing x,y,z
    this->origin[this->type] = this->bd[2 * this->type] +
                               ( requestedValue * 0.010f ) *
                               ( this->bd[2 * this->type + 1] - this->bd[2 * this->type] );
}

int CuttingPlane::isPastEnd()
{
    if( this->origin[this->type] > ( this->bd[2 * this->type + 1] + 0.5 * this->dx ) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int CuttingPlane::isAtEnd()
{
    if( this->origin[this->type] > ( this->bd[2 * this->type + 1] - ( this->bd[2 * this->type + 1] - this->bd[2 * this->type] ) / 100.0 ) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int CuttingPlane::isAtStart()
{
    if( this->origin[this->type] < ( this->bd[2 * this->type] + ( this->bd[2 * this->type + 1] - this->bd[2 * this->type] ) / 100.0 ) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void CuttingPlane::ResetOriginToLow()
{
    this->origin[this->type] = this->bd[2 * this->type] +
                               ( this->bd[2 * this->type + 1] - this->bd[2 * this->type] ) / 100.0;
}

void CuttingPlane::ResetOriginToHigh()
{
    this->origin[this->type] = this->bd[2 * this->type + 1] -
                               ( this->bd[2 * this->type + 1] - this->bd[2 * this->type] ) / 100.0;
}

void CuttingPlane::IncrementOrigin()
{
    this->origin[this->type] += this->dx;
}
