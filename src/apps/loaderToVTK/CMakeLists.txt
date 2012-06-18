_addExecutable( App loaderToVTK
    main.cpp
    LATTICEFX_LIBRARIES
        vtk_utils
        vtk_translator
        ${_vtkLibraries}
)
