_addExecutable( Test meshViewer
    meshViewer.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( Test convertVTK2Ascii
    convertVTK2Ascii.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( Test convertVTK2Binary
    convertVTK2Binary.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( Test convertSurfaceFileToStl
    convertSurfaceFileToStl.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

_addExecutable( Test whatIsScalarRange
    whatIsScalarRange.cxx
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
