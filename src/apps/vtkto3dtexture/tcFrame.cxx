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
#include "tcFrame.h"
#include <wx/string.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <ves/xplorer/util/fileIO.h>


BEGIN_EVENT_TABLE(TCFrame,wxFrame)
   EVT_BUTTON(TRANSLATE_BUTTON,TCFrame::_onTranslateCallback)
   EVT_BUTTON(INPUT_BROWSE,TCFrame::_onBrowseCallback)
   EVT_BUTTON(OUTPUT_BROWSE,TCFrame::_onBrowseCallback)
   EVT_BUTTON(QUIT_BUTTON,TCFrame::_onQuitCallback)
   EVT_COMBOBOX(XRES_BOX,TCFrame::_onResolutionCallback)
   EVT_COMBOBOX(YRES_BOX,TCFrame::_onResolutionCallback)
   EVT_COMBOBOX(ZRES_BOX,TCFrame::_onResolutionCallback)
   EVT_SPINCTRL(NUM_FILES,TCFrame::_onNumFilesCallback)
   EVT_RADIOBOX(GRID_RBOX,TCFrame::_onGridTypeCallback)
   EVT_MENU(GRID_PROPERTY,TCFrame::_transientGridTypeSelection)
   EVT_MENU(TIME_STEP_RANGE,TCFrame::_onTransientMinimum)
   EVT_TEXT(INPUT_TEXT_BOX,TCFrame::_onUpdateDirectoryText)
   EVT_TEXT(OUTPUT_TEXT_BOX,TCFrame::_onUpdateDirectoryText)
END_EVENT_TABLE()
////////////////////////////////////////////////////
//Constructor                                     //
////////////////////////////////////////////////////
TCFrame::TCFrame(wxWindow* parent,
           wxWindowID id, 
           const wxString& title,
           const wxPoint& pos , 
           const wxSize& size, 
           long style)
:wxFrame(parent, id, title,pos, size, style)
{
   _numFiles = 0;
   
   _minTimeStepIndex = 0;
   _inputDir = wxT("./");
   _outputDir = wxT("./");

   _outputTextFile = wxT("./out.txt");

   _dirDialog = 0;

   _browseInputDir = 0;
   _browseOutputDir = 0;
   _goButton = 0;
   _quitButton = 0;
   _translator = 0;

   _transProgress = 0;
   _fileProgress = 0;

   _inputDirBox = 0;
   _outputDirBox = 0;

   _xResBox = 0;
   _yResBox = 0;
   _zResBox = 0;

   _numFilesBox = 0;

   _resolution[0] = 2;
   _resolution[1] = 2;
   _resolution[2] = 2;
   _type = UNSTRUCTURED;
   _translator = new VTKDataToTexture();
   _translator->setParentGUI(this);
   _currentFile = 0;

   rank = -1;
   _menuBar = 0;
   numProcessors = -1;
   _buildGUI();

}
///////////////////
TCFrame::~TCFrame()
{
   if(_translator){
      delete _translator;
      _translator = 0;
   }

   if(_dirDialog){
      delete _dirDialog;
      _dirDialog = 0;
   }
   if(_browseInputDir){
      delete _browseInputDir;
      _browseInputDir = 0;
   }
   if(_browseOutputDir){
      delete _browseOutputDir;
      _browseOutputDir = 0;
   }
   if(_goButton){
      delete _goButton;
      _goButton = 0;
   }

   if(_transProgress){
      delete _transProgress;
      _transProgress = 0;
   }
   if(_fileProgress){
      delete _fileProgress;
      _fileProgress = 0;
    
   }
   if(_inputDirBox){
      delete _inputDirBox;
      _inputDirBox = 0;
   }
   if(_outputDirBox){
      delete _outputDirBox;
      _outputDirBox = 0; 
   }
   if(_quitButton){
      delete _quitButton;
      _quitButton = 0;
   }

   if(_xResBox){
      delete _xResBox;
      _xResBox = 0;
   }
   if(_yResBox){
      delete _yResBox;
      _yResBox = 0;
   }
   if(_zResBox){
      delete _zResBox;
      _zResBox = 0;
   }
   if(_numFilesBox){
      delete _numFilesBox;
      _numFilesBox = 0;
   }
   if(_gridTypeBox){
      delete _gridTypeBox;
      _gridTypeBox = 0;
   }
   //_gridFiles.Clear();
     
   if(!_inputFiles.empty()){
      for ( unsigned int i = 0; i < _inputFiles.size(); ++i )
         delete [] _inputFiles.at( i );
      _inputFiles.clear();
   }
}
/////////////////////////
void TCFrame::_buildGUI()
{
   //sizers for our gui's main sections
   wxBoxSizer* frameSizer = new wxBoxSizer(wxHORIZONTAL);

   wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
   
   wxStaticBox* dimensionsGroup = new wxStaticBox(this, -1, _("Texture Dimensions") );
   wxStaticBoxSizer* textureDimensionsSizer = new wxStaticBoxSizer(dimensionsGroup,
                                                            wxHORIZONTAL);

   //the right section
   wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
   wxBoxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
   wxBoxSizer* row2 = new wxBoxSizer(wxHORIZONTAL);

   wxArrayString factorsOfTwo;

   factorsOfTwo.Add(_("2") );
   factorsOfTwo.Add(_("4") );
   factorsOfTwo.Add(_("8") );
   factorsOfTwo.Add(_("16") );
   factorsOfTwo.Add(_("32") );
   factorsOfTwo.Add(_("64") );
   factorsOfTwo.Add(_("128") );
   factorsOfTwo.Add(_("256") );
   factorsOfTwo.Add(_("512") );

   wxString choices[] = {_("Rectilinear"),_("Structured"),_("Unstructured")};
   wxBoxSizer* rBoxSizer = new wxBoxSizer(wxHORIZONTAL);
   _gridTypeBox = new wxRadioBox(this, 
                              GRID_RBOX, 
                              _("Grid Type"), 
                              wxDefaultPosition, 
                              wxDefaultSize, 
                              3, choices,wxRA_SPECIFY_COLS);
   rBoxSizer->Add(_gridTypeBox,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND);
   //the resolution input boxes
   _xResBox = new wxComboBox(this, XRES_BOX, _(""), 
                                wxDefaultPosition,
                                wxDefaultSize,
                                factorsOfTwo,
                                wxCB_READONLY,
                                wxDefaultValidator,
                                  _("X") );
   _xResBox->SetSelection(0);
   
   _yResBox = new wxComboBox(this, YRES_BOX, _(""), 
                                wxDefaultPosition,
                                wxDefaultSize,
                                factorsOfTwo,
                                wxCB_READONLY,
                                wxDefaultValidator,
                                  _("Y") );
   _yResBox->SetSelection(0);
   
   _zResBox = new wxComboBox(this, ZRES_BOX, _(""), 
                                wxDefaultPosition,
                                wxDefaultSize,
                                factorsOfTwo,
                                wxCB_READONLY,
                                wxDefaultValidator,
                                  _("Z") );
   _zResBox->SetSelection(0);
   
   //resolution row
   textureDimensionsSizer->Add(_xResBox,1,wxALIGN_CENTER_HORIZONTAL);
   textureDimensionsSizer->Add(_yResBox,1,wxALIGN_CENTER_HORIZONTAL);
   textureDimensionsSizer->Add(_zResBox,1,wxALIGN_CENTER_HORIZONTAL);

   //add the input text box and browse button to the two row
   //row 1 
   _inputDirBox =  new wxTextCtrl(this,INPUT_TEXT_BOX);
   _browseInputDir = new wxButton(this,INPUT_BROWSE,wxT("Input dir"));

   row1->Add(_inputDirBox,3,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);
   row1->Add(_browseInputDir,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);

   //row 2
   _outputDirBox =  new wxTextCtrl(this,OUTPUT_TEXT_BOX);
   _browseOutputDir = new wxButton(this,OUTPUT_BROWSE,wxT("Output dir"));
 
   row2->Add(_outputDirBox,3,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);
   row2->Add(_browseOutputDir,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);

   //quit button
   _quitButton = new wxButton(this,QUIT_BUTTON,wxT("Quit"));
   _goButton = new wxButton(this,TRANSLATE_BUTTON,wxT("Translate"));

   buttonRow->Add(_goButton,0,wxALIGN_CENTER);
   buttonRow->Add(_quitButton,0,wxALIGN_CENTER);

   rightSizer->Add(textureDimensionsSizer,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND);
   rightSizer->Add(rBoxSizer,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND);
   rightSizer->Add(row1,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);
   rightSizer->Add(row2,1,wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL);
   rightSizer->Add(buttonRow,0,wxALIGN_CENTER);
  
   frameSizer->Add(rightSizer,1,wxEXPAND|wxALIGN_CENTER_HORIZONTAL);

   frameSizer->Layout();
   SetSizer(frameSizer);

   
   SetStatusBar(new wxStatusBar(this,-1)); 
   GetStatusBar()->SetStatusText(_("..."));

   _menuBar = new wxMenuBar();
   wxMenu* transientPropertiesMenu = new wxMenu();
   transientPropertiesMenu->Append(GRID_PROPERTY,_("Grid Property..."));
   transientPropertiesMenu->Append(TIME_STEP_RANGE,_("Time Steps..."));
   _menuBar->Append(transientPropertiesMenu,_("&Transient Properties"));
   SetMenuBar(_menuBar);

   SetAutoLayout(true); 
   frameSizer->Fit(this);
}
///////////////////////////////////////////////////
void TCFrame::UpdateProgressDialog(const std::string msg)
{
   if(_fileProgress)
   {
      _fileProgress->Update(_currentFile, wxString( msg.c_str(), wxConvUTF8) );
   }
}
/////////////////////////////////////////////////////////
void TCFrame::_onTransientMinimum(wxCommandEvent& WXUNUSED( event ) )
{
   wxTextEntryDialog minimumDlg(this, 
                        _("Beginning Transient Time Step"),
                        _("Enter the minimum timestep:"),
                        _("0"),wxOK);
   minimumDlg.ShowModal();
   minimumDlg.GetValue().ToULong(&_minTimeStepIndex);
}
////////////////////////////////////////////////////////////////
void TCFrame::_transientGridTypeSelection(wxCommandEvent& WXUNUSED( event ) )
{
    wxArrayString transientType;
    transientType.Add( _("Static") );
    transientType.Add( _("Dynamic") );
    wxSingleChoiceDialog typeSelector(this,
                                      _T("Select trasient grid type"),
                                      _T("Grid Type"),
                                      transientType);

   if (typeSelector.ShowModal() == wxID_OK)
   {
      //std::cout<<"Selecting face: "<<faceSelector.GetStringSelection()<<std::endl;
      if(typeSelector.GetStringSelection()== _("Dynamic") )
      {
         _translator->TurnOnDynamicGridResampling();
      }
      else
      {
         _translator->TurnOffDynamicGridResampling();
      }
   }
}
///////////////////////////////////////////////////////////////
void TCFrame::SetMinimumTimeStep(unsigned long minimumTimestep)
{
   _minTimeStepIndex = minimumTimestep;
}
/////////////////////////////////////////////////////
void TCFrame::SetTransientGridProperty(long int type)
{
   if(type)
   {
      _translator->TurnOnDynamicGridResampling();
   }
   else
   {
      _translator->TurnOffDynamicGridResampling();
   }
}
/////////////////////////////////////////////////////
void TCFrame::UpdateStatus(const std::string statusString)
{
   GetStatusBar()->SetStatusText(wxString(statusString.c_str(), wxConvUTF8 ));
}
/////////////////////////////////////////////////////////
void TCFrame::SetMPIVariables( int iRank, int inumProcessors )
{
   rank = iRank;
   numProcessors = inumProcessors;
}
/////////////////////////////////////////////////////////
//event callbacks                                      //
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
void TCFrame::_onGridTypeCallback(wxCommandEvent& WXUNUSED( event ) )
{
   UpdateStatus("Switching grid type. . .");
   int type = _gridTypeBox->GetSelection();
   switch (type){
      case 0:
         _type = RECTILINEAR;
         _translator->setRectilinearGrid();
         UpdateStatus("Grid type: RECTILINEAR");   
         break;
      case 1:
         _type = STRUCTURED;
         _translator->setStructuredGrid();
         UpdateStatus("Grid type: STRUCTURED");
         break;
      case 2:
         _type = UNSTRUCTURED;
         _translator->setUnstructuredGrid();
         UpdateStatus("Grid type: UNSTRUCTURED");
      default:
         break;
   };
}
/////////////////////////////////////////////////////////
void TCFrame::_onTranslateCallback(wxCommandEvent& WXUNUSED( event ) )
{
   _translator->setBatchOff();
   
   UpdateStatus( "Translating to 3D texture files. . ." );
   if(!_numFiles)
   {
      wxString errorMsg = _("No files found in: ") + _inputDir;
      wxMessageDialog(this,errorMsg, 
                        _("Texture Translator"),
                      wxICON_EXCLAMATION|wxICON_HAND).ShowModal(); 
      UpdateStatus( "No files found!!!" );
   }

   wxString statusMsg = _("");

   _fileProgress = new wxProgressDialog(_("Translation Progress"),
                  _(" "), 
                  _numFiles,this,
                  wxPD_AUTO_HIDE|wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_ESTIMATED_TIME);
   //process the files
   for(int i = 0; i < _numFiles; i++)
   {
      _currentFile = i;
      statusMsg = wxString("Translating ", wxConvUTF8) + wxString( gridFiles.at( i ).c_str(), wxConvUTF8 );
      UpdateStatus( ConvertUnicode( statusMsg.c_str() ) );
      _fileProgress->Update(i, statusMsg);
      _translator->reset();
      _translator->setOutputDirectory( ConvertUnicode( _outputDir.c_str() ) );
      std::ostringstream dirStringStream;
      dirStringStream << "_" << std::setfill( '0' ) << std::setw( 6 ) << _minTimeStepIndex+i;
      std::string dirString = dirStringStream.str();

      if(_translator->createDataSetFromFile( gridFiles.at( i ) ))
      {
         _translator->setOutputResolution(_resolution[0],
                                          _resolution[1],
                                          _resolution[2]);
         _translator->setVelocityFileName( dirString );
         _translator->createTextures();
         statusMsg = wxString("Files written to: ", wxConvUTF8) + _outputDir;
         UpdateStatus( ConvertUnicode( statusMsg.c_str() ) );
      }
      else
      { 
         statusMsg = wxString("Invalide file: ", wxConvUTF8) + wxString( gridFiles.at( i ).c_str(), wxConvUTF8) ;
         UpdateStatus( ConvertUnicode( statusMsg.c_str() ) );
      }
   }
   
   if(_fileProgress)
   {
      delete _fileProgress;
      _fileProgress = 0;
   }
   _translator->reInit();
}
//////////////////////////////////////////////////////////
void TCFrame::_onResolutionCallback(wxCommandEvent& event )
{
   int value = event.GetSelection()+1;
   int id = event.GetId();

   switch (id){
      case XRES_BOX:
         _resolution[0] = (int)std::pow(2.,value);
         break;
      case YRES_BOX:
         _resolution[1] = (int)std::pow(2.,value);
         break;
      case ZRES_BOX:
         _resolution[2] = (int)std::pow(2.,value);
         break;
      default:
         break;
   };
}
///////////////////////////////////////////////////////////
void TCFrame::_onUpdateDirectoryText(wxCommandEvent& WXUNUSED( event ) )
{
   SetInputDirectory( ConvertUnicode( _inputDirBox->GetValue().c_str() ) );
   SetOutputDirectory( ConvertUnicode( _outputDirBox->GetValue().c_str() ) );
}
////////////////////////////////////////////////////
void TCFrame::_onQuitCallback(wxCommandEvent& WXUNUSED( event ) )
{
   Destroy();
   exit(0);
}
//////////////////////////////////////////////////////
void TCFrame::_onNumFilesCallback(wxSpinEvent& event)
{
   _numFiles = event.GetPosition();
}
////////////////////////////////////////////////////////
void TCFrame::_chooseDirectory(int style, int browseID)
{
   if(!_dirDialog){
      _dirDialog = new wxDirDialog(this,
          _T("Choose a directory"), wxT("./"), style);
   }else{
      //set the path to the current stored path
      if(browseID == INPUT_BROWSE){
         _dirDialog->SetPath(_inputDir);
      }else{
         _dirDialog->SetPath(_outputDir);
      }
   }
   wxDir inputFileDirectory;
   //get the input from the user 
   if ( _dirDialog->ShowModal() == wxID_OK )
   {
      if ( browseID == INPUT_BROWSE)
      {
         //_gridFiles.Clear();
         _numFiles = 0;
#if wxCHECK_VERSION( 2, 9, 0 )
         _inputDir = _dirDialog->GetPath();
#else
         _inputDir = wxString(_dirDialog->GetPath().c_str(), wxConvUTF8);
#endif
         
         //make the output dir == to the current input directory
#if wxCHECK_VERSION( 2, 9, 0 )
         _outputDir = _inputDir;
#else
         _outputDir = wxString(_inputDir.c_str(), wxConvUTF8);
#endif
         inputFileDirectory.Open(wxString(_inputDir));
         if ( inputFileDirectory.IsOpened() )
         {
           gridFiles = ves::xplorer::util::fileIO::GetFilesInDirectory( ConvertUnicode( _inputDir.c_str() ), ".vtu" );
           if ( gridFiles.size() == 0 )
              gridFiles = ves::xplorer::util::fileIO::GetFilesInDirectory( ConvertUnicode( _inputDir.c_str() ), ".vtk" );
         }
         _numFiles = gridFiles.size();
         //set the text in the text box
         _inputDirBox->SetValue(_inputDir);
      }
      else
      {
#if wxCHECK_VERSION( 2, 9, 0 )
         _outputDir = _dirDialog->GetPath();
#else
         _outputDir = wxString(_dirDialog->GetPath().c_str(), wxConvUTF8);
#endif
         _outputDirBox->SetValue(_outputDir);
      }
   }
}
//////////////////////////////////////////////////////
void TCFrame::_onBrowseCallback(wxCommandEvent& event )
{
    _chooseDirectory(wxDD_DEFAULT_STYLE & ~wxDD_NEW_DIR_BUTTON,
                   event.GetId());
}
////////////////////////////////////////////////////////
//Batch translation interfaces                        //
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
void TCFrame::SetInputDirectory(const std::string inDirectory)
{
   if ( !inDirectory.empty() )
   {
      /*if ( !_gridFiles.IsEmpty() )
      {
        _gridFiles.Clear();
      }*/
      _numFiles = 0;

      wxDir inputFileDirectory;
      inputFileDirectory.Open( wxString( inDirectory.c_str(), wxConvUTF8 ) );

      if ( inputFileDirectory.IsOpened() )
      {
        _inputDir = wxString(inDirectory.c_str(), wxConvUTF8 );

        gridFiles = ves::xplorer::util::fileIO::GetFilesInDirectory( inDirectory, ".vtu" );
        if ( gridFiles.size() == 0 )
           gridFiles = ves::xplorer::util::fileIO::GetFilesInDirectory( inDirectory, ".vtk" );
      }
      else
      {
        std::cout<<"Couldn't open directory: "<<inDirectory<<std::endl;
      }
      _numFiles = gridFiles.size();
   }
}
//////////////////////////////////////////////////////////
void TCFrame::SetOutputDirectory(const std::string outDirectory)
{
   _outputDir = wxString( outDirectory.c_str(), wxConvUTF8 );
}
//////////////////////////////////////////////////////
void TCFrame::SetTextureResolution(int x,int y, int z)
{
   
   _resolution[0] = NearestPowerOfTwo(x);
   _resolution[1] = NearestPowerOfTwo(y);
   _resolution[2] = NearestPowerOfTwo(z);
}
///////////////////////////////////////////
int TCFrame::NearestPowerOfTwo(int input)
{
    int value = 1;
    while (value < input)
    {
       value = value << 1;
    }
    return (value > 512)?512:value;
}
/////////////////////////////////////////////////
void TCFrame::SetGridType(GridType type)
{
   switch (type){
      case RECTILINEAR:
         _translator->setRectilinearGrid();
         break;
      case STRUCTURED:
         _translator->setStructuredGrid();
         break;
      case UNSTRUCTURED:
         _translator->setUnstructuredGrid();
      default:
         break;
   };
   _type = type;
}
////////////////////////////////
void TCFrame::BatchTranslation()
{
   _translator->setBatchOn();
   //process the files
   /*std::cout << " rannk = " << rank << std::endl 
               << " number of files = " << _numFiles << std::endl 
               << " number of processors = " << numProcessors << std::endl;*/
   for ( int i = rank; i < _numFiles; i += numProcessors )
   {
      _translator->reset();
      _translator->setOutputDirectory( ConvertUnicode( _outputDir.c_str() ) );

      std::ostringstream dirStringStream;
      dirStringStream << "_" << std::setfill( '0' ) << std::setw( 6 ) << _minTimeStepIndex + i;
      std::string dirString = dirStringStream.str();
      //std::string tempString( _gridFiles[ i ].c_str() );   
      std::string tempString = gridFiles.at( i );   
      if(_translator->createDataSetFromFile( tempString ))
      {
         _translator->setOutputResolution(_resolution[0],
                                          _resolution[1],
                                          _resolution[2]);
         _translator->setVelocityFileName( dirString );
         _translator->createTextures();
      }
   }
}
