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
#ifndef STARREADER_H
#define STARREADER_H
#include <vector>
#include <string>

class vtkUnstructuredGrid;
namespace lfx
{
namespace vtk_translator
{
class starReader
{
public:
    starReader( std::string paramFile );
    ~starReader( void );

    void SetDebugLevel( int );

    float* GetRotations( void );
    float* GetTranslations( void );
    int     GetScaleIndex( void );
    float   GetScaleFactor( void );
    int     GetWriteOption( void );
    std::string GetVTKFileName( void );

    void  ReadParameterFile( void );
    vtkUnstructuredGrid* GetUnsGrid();

private:
    std::string paramFileName;
    std::string starCellFileName;
    std::string starVertFileName;
    std::string starUsrFileName;
    std::string vtkFileName;
    char  textline[256];//std::string textline;//
    int   numScalars;
    std::vector< std::string > scalarName;
    int   numVectors;
    std::vector< std::string > vectorName;
    int   debug;
    int   writeOption;
    float rotations[ 3 ];
    float translations[ 3 ];
    int   scaleIndex;
    float scaleFactor;
};
}
}
#endif
