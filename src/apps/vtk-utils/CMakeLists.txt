_addExecutable( App meshViewer
    meshViewer.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( App convertVTK2Ascii
    convertVTK2Ascii.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( App convertVTK2Binary
    convertVTK2Binary.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( App convertSurfaceFileToStl
    convertSurfaceFileToStl.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( App whatIsScalarRange
    whatIsScalarRange.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
