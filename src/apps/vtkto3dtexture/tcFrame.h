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
#ifndef _TC_FRAME_H_
#define _TC_FRAME_H_
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/progdlg.h>
#include "textureCreator.h"

#include <vector>
#include <string>

enum TCFrameIDs
{
    TRANSLATE_BUTTON,
    INPUT_TEXT_BOX,
    OUTPUT_TEXT_BOX,
    INPUT_BROWSE,
    OUTPUT_BROWSE,
    XRES_BOX,
    YRES_BOX,
    ZRES_BOX,
    NUM_FILES,
    GRID_RBOX,
    QUIT_BUTTON,
    GRID_PROPERTY,
    TIME_STEP_RANGE
};
class TCFrame: public wxFrame
{
public:
    TCFrame( wxWindow* parent,
             wxWindowID id,
             const wxString& title,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_FRAME_STYLE );

    virtual ~TCFrame();
    enum GridType {STRUCTURED, UNSTRUCTURED, RECTILINEAR};
    void UpdateStatus( const std::string statusString );
    void SetGridType( GridType type );
    void UpdateProgressDialog( const std::string msg );

    unsigned int GetNumberOfTimeSteps()
    {
        return _numFiles;
    }

    ///////////////////////////////
    //Batch translation interface//
    ///////////////////////////////
    void SetInputDirectory( const std::string inDirectory );
    void SetOutputDirectory( const std::string outDirectory );
    void SetTextureResolution( int x, int y, int z );
    void BatchTranslation();

    void SetMPIVariables( int, int );

    ///Set the minimum timestep
    ///\param minimumTimestep The minimum output timestep
    void SetMinimumTimeStep( unsigned long minimumTimestep );

    ///Set the transient grid property
    ///\param type Static == 0\n Dynamic == 1\n
    void SetTransientGridProperty( long int type );

    ///Find the nearest power of 2 for a given integer\n
    ///This is clamped at 512
    ///\param input The input number
    int NearestPowerOfTwo( int input );

protected:
    GridType _type;
    void _buildGUI();
    int _numFiles;
    int _resolution[3];
    int _currentFile;
    unsigned long _minTimeStepIndex;
    std::vector<const char*> _inputFiles;
    wxString _inputDir;
    wxString _outputDir;

    wxString _outputTextFile;

    wxDirDialog* _dirDialog;

    //wxArrayString _gridFiles;
    std::vector< std::string > gridFiles;

    wxButton* _browseInputDir;
    wxButton* _browseOutputDir;
    wxButton* _goButton;
    wxButton* _quitButton;

    wxGauge* _transProgress;
    wxProgressDialog* _fileProgress;

    wxTextCtrl* _inputDirBox;
    wxTextCtrl* _outputDirBox;
    wxSpinCtrl* _numFilesBox;

    wxComboBox* _xResBox;
    wxComboBox* _yResBox;
    wxComboBox* _zResBox;

    wxRadioBox* _gridTypeBox;

    VTKDataToTexture* _translator;

    wxMenuBar* _menuBar;
    //event callbacks
    void _onQuitCallback( wxCommandEvent& event );
    void _onTranslateCallback( wxCommandEvent& event );
    void _onBrowseCallback( wxCommandEvent& event );
    void _chooseDirectory( int style, int browseID );
    void _onResolutionCallback( wxCommandEvent& event );
    void _onNumFilesCallback( wxSpinEvent& event );
    void _onGridTypeCallback( wxCommandEvent& event );
    void _transientGridTypeSelection( wxCommandEvent& event );
    void _onTransientMinimum( wxCommandEvent& event );

    ///Update the directories based on text input
    void _onUpdateDirectoryText( wxCommandEvent& event );
private:
    int numProcessors;
    int rank;

    std::string ConvertUnicode( const wxChar* data )
    {
        std::string tempStr( static_cast< const char* >( wxConvCurrent->cWX2MB( data ) ) );
        return tempStr;
    }
    DECLARE_EVENT_TABLE()
};
#endif//_TC_FRAME_H_
