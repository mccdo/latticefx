_addExecutable( App meshViewer
    meshViewer.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
set_property(TARGET meshViewer PROPERTY FOLDER "App/VTK")

_addExecutable( App convertVTK2Ascii
    convertVTK2Ascii.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
set_property(TARGET convertVTK2Ascii PROPERTY FOLDER "App/VTK")

_addExecutable( App convertVTK2Binary
    convertVTK2Binary.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
set_property(TARGET convertVTK2Binary PROPERTY FOLDER "App/VTK")

_addExecutable( App convertSurfaceFileToStl
    convertSurfaceFileToStl.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
set_property(TARGET convertSurfaceFileToStl PROPERTY FOLDER "App/VTK")

_addExecutable( App whatIsScalarRange
    whatIsScalarRange.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
set_property(TARGET whatIsScalarRange PROPERTY FOLDER "App/VTK")
