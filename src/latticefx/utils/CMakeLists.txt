if(VTK_FOUND)
    add_subdirectory(vtk)
endif(VTK_FOUND)

install( FILES CompilerGuards.h 
         DESTINATION include/latticefx/utils
         COMPONENT latticefx-dev )