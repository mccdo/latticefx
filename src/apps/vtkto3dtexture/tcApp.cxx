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
#include "tcApp.h"
#include <wx/cmdline.h>
#include <cassert>
#ifdef USE_MPI
#include <mpi.h>
#endif
//#include <omp.h>

IMPLEMENT_APP( TCApp )

#include <iostream>
//////////////////////////////
//Initialize the application//
//////////////////////////////
bool TCApp::OnInit()
{
    //This function is entered first and then the command line
    // parser function is called
    if( !wxApp::OnInit() )
    {
        return false;
    }
    //For commandline parsing we are returning from the command line function
    int ierror;
    p = 1;
    rank = 0;
    //void omp_set_num_threads( 2 );
    //void omp_set_dynamic( 0 );
    //void omp_set_nested( 0 );

    /* // Initialize MPI
     ierror = MPI_Init( &(wxGetApp().argc), &(wxGetApp().argv) );
     ierror = MPI_Comm_size( MPI_COMM_WORLD, &p  );
     ierror = MPI_Comm_rank( MPI_COMM_WORLD, &rank );
     */
    //Create the main window
    if( _isBatch )
    {
        return ( _translateFromCmdLine() );
    }
    else
    {
        _frame = new TCFrame( 0, -1, wxT( "Texture Creator" ) );
        // Problem with generic wxNotebook implementation whereby it doesn't size
        // properly unless you set the size again
#if defined(__WIN16__) || defined(__WXMOTIF__)
        int width, height;
        _frame->GetSize( & width, & height );
        _frame->SetSize( -1, -1, width, height );
#endif
        _frame->SetMPIVariables( rank, p );
        //display the UI
        _frame->Show();
        return TRUE;
    }
}
////////////////////////////////////
bool TCApp::_translateFromCmdLine()
{
    _frame->BatchTranslation();
#ifdef USE_MPI
    int ierror;
    ierror = MPI_Barrier( MPI_COMM_WORLD );
    ierror = MPI_Finalize();
#endif
    return false;
}
//////////////////////////////////////////////////
void TCApp::OnInitCmdLine( wxCmdLineParser& parser )
{
    if( argc < 2 )
    {
        _isBatch = false;
        return;
    }
    _isBatch = true;
    p = 1;
    rank = 0;
#ifdef USE_MPI
    // Initialize MPI
    int ierror;
    ierror = MPI_Init( &( wxGetApp().argc ), &( wxGetApp().argv ) );
    ierror = MPI_Comm_size( MPI_COMM_WORLD, &p );
    ierror = MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#endif
    static const wxCmdLineEntryDesc cmdLineOpts[] =
    {
#if wxCHECK_VERSION( 2, 9, 0 )
        { wxCMD_LINE_SWITCH, wxS( "v" ), wxS( "verbose" ), wxS( "be verbose" ) },
        { wxCMD_LINE_SWITCH, wxS( "q" ), wxS( "quiet" ),   wxS( "be quiet" ) },
        { wxCMD_LINE_SWITCH, wxS( "h" ), wxS( "help" ), wxS( "print help" ) },

        {
            wxCMD_LINE_OPTION,
            wxS( "o" ), wxS( "odir" ),
            wxS( "Directory to write results,defaults to input directory. Default is current dir." )
        },


        {
            wxCMD_LINE_OPTION,
            wxS( "g" ), wxS( "gridType" ),
            wxS( "VTK input dataset grid type\n r == Rectilinear\n s == Structured\n u ==Unstructured" )
        },

        {
            wxCMD_LINE_OPTION,
            wxS( "x" ), wxS( "xRes" ),
            wxS( "x texture resolution specified in power of two. Default is 32." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            wxS( "y" ), wxS( "yRes" ),
            wxS( "y texture resolution specified in power of two. Default is 32." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            wxS( "z" ), wxS( "zRes" ),
            wxS( "z texture resolution specified in power of two. Default is 32." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            wxS( "tgp" ), wxS( "tGridProp" ),
            wxS( "Transient Grid Property (Static=0,Dynamic=1). Default is 0(Static)." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            wxS( "ms" ), wxS( "minStep" ),
            wxS( "Minimum Time Step to begin translations.  Default is 0." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,//wxCMD_LINE_PARAM,
            wxS( "i" ), wxS( "inputDir" ),
            wxS( "Input directory" ),
            wxCMD_LINE_VAL_STRING
        },
#else
        { wxCMD_LINE_SWITCH, _T( "v" ), _T( "verbose" ), _T( "be verbose" ) },
        { wxCMD_LINE_SWITCH, _T( "q" ), _T( "quiet" ),   _T( "be quiet" ) },
        { wxCMD_LINE_SWITCH, _T( "h" ), _T( "help" ), _T( "print help" ) },

        {
            wxCMD_LINE_OPTION,
            _T( "o" ), _T( "odir" ),
            _T( "Directory to write results,defaults to input directory. Default is current dir." )
        },

        {
            wxCMD_LINE_OPTION,
            _T( "g" ), _T( "gridType" ),
            _T( "VTK input dataset grid type\n r == Rectilinear\n s == Structured\n u ==Unstructured" )
        },

        {
            wxCMD_LINE_OPTION,
            _T( "x" ), _T( "xRes" ),
            _T( "x texture resolution specified in power of two. Default is 32." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            _T( "y" ), _T( "yRes" ),
            _T( "y texture resolution specified in power of two. Default is 32." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            _T( "z" ), _T( "zRes" ),
            _T( "z texture resolution specified in power of two. Default is 32." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            _T( "tgp" ), _T( "tGridProp" ),
            _T( "Transient Grid Property (Static=0,Dynamic=1). Default is 0(Static)." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,
            _T( "ms" ), _T( "minStep" ),
            _T( "Minimum Time Step to begin translations.  Default is 0." ),
            wxCMD_LINE_VAL_NUMBER
        },

        {
            wxCMD_LINE_OPTION,//wxCMD_LINE_PARAM,
            _T( "i" ), _T( "inputDir" ),
            _T( "Input directory" ),
            wxCMD_LINE_VAL_STRING
        },
#endif

        {wxCMD_LINE_NONE}
    };
    parser.SetDesc( cmdLineOpts );
}
////////////////////////////////////////////////////
bool TCApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
    if( _isBatch )
    {
        _frame = new TCFrame( 0, -1, wxT( "Texture Creator" ) );
        _frame->SetMPIVariables( rank, p );
        wxApp::OnCmdLineParsed( parser );
        //set all the options on the translator
        wxString inputDir;
        wxString outputDir( _( "./" ) );
        long int resolution[3] = {32, 32, 32};
        long int transientGridProp = 0;
        long int minStep = 0;

        /**/
        if( parser.Found( _( "i" ), &inputDir ) )
        {
            //inputDir = parser.GetParam(0);
            _frame->SetInputDirectory( ConvertUnicode( inputDir.c_str() ) );
        }
        else
        {
            parser.Usage();
            return false;
        }

        if( !parser.Found( _( "y" ), &resolution[1] ) )
        {
            return false;
        }
        if( !parser.Found( _( "x" ), &resolution[0] ) )
        {
            return false;
        }
        if( !parser.Found( _( "z" ), &resolution[2] ) )
        {
            return false;
        }
        if( !parser.Found( _( "ms" ), &minStep ) )
        {
            minStep = 0;
        }
        if( !parser.Found( _( "tgp" ), &transientGridProp ) )
        {
            transientGridProp = 0;
        }
        if( parser.Found( _( "h" ) ) )
        {
            parser.Usage();
            return false;
        }
        if( !parser.Found( _( "o" ), &outputDir ) )
        {
            _frame->SetOutputDirectory( ConvertUnicode( inputDir.c_str() ) );
        }
        else
        {
            _frame->SetOutputDirectory( ConvertUnicode( outputDir.c_str() ) );
        }
        _frame->SetTransientGridProperty( transientGridProp );
        _frame->SetMinimumTimeStep( minStep );
        _frame->SetTextureResolution( resolution[0], resolution[1], resolution[2] );

    }
    return true;
}
