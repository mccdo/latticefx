#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T15:20:50
#
#-------------------------------------------------

QT       += core gui

TARGET = shapeCreatorQT
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    UtlSettings.cpp \
    ../common/vtkCreator.cpp

HEADERS  += MainWindow.h \
    UtlSettings.h \
    ../common/shapeCreatorDefines.h \
    ../common/vtkCreator.h \
    ../common/shapeCreator.h

INCLUDEPATH += $$PWD/../common \
                D:/skewmatrix/osgworks/include \
                D:/skewmatrix/osgworks/bld \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11 \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Common \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/VolumeRendering \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Rendering \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Charts \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Chemistry \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities\vtkalglib \
                D:/skewmatrix/vtk/vtk-5.10.1/Infovis \
                D:/skewmatrix/vtk/vtk-5.10.1/Geovis \
                D:/skewmatrix/vtk/vtk-5.10.1/Views \
                D:/skewmatrix/vtk/vtk-5.10.1/VolumeRendering \
                D:/skewmatrix/vtk/vtk-5.10.1/Hybrid \
                D:/skewmatrix/vtk/vtk-5.10.1/Widgets \
                D:/skewmatrix/vtk/vtk-5.10.1/Rendering \
                D:/skewmatrix/vtk/vtk-5.10.1/Charts \
                D:/skewmatrix/vtk/vtk-5.10.1/Chemistry \
                D:/skewmatrix/vtk/vtk-5.10.1/Rendering/Testing/Cxx \
                D:/skewmatrix/vtk/vtk-5.10.1/IO \
                D:/skewmatrix/vtk/vtk-5.10.1/Imaging \
                D:/skewmatrix/vtk/vtk-5.10.1/Graphics \
                D:/skewmatrix/vtk/vtk-5.10.1/GenericFiltering \
                D:/skewmatrix/vtk/vtk-5.10.1/Filtering \
                D:/skewmatrix/vtk/vtk-5.10.1/Common \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities \
                D:/skewmatrix/vtk/vtk-5.10.1/Common/Testing/Cxx \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtknetcdf \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtknetcdf \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtknetcdf/include \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtklibproj4 \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtklibproj4 \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/DICOMParser \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/DICOMParser \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtkfreetype/include \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtkfreetype/include \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/LSDyna \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/LSDyna \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/MaterialLibrary \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/MaterialLibrary \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtkmetaio \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtkmetaio \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/verdict \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/verdict \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtkhdf5 \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtkhdf5 \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtkhdf5/src \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtkhdf5/src \
                D:/skewmatrix/vtk/vtk-5.10.1/bld11/Utilities/vtkhdf5/hl/src \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtkhdf5/hl/src \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/utf8/source \
                D:/skewmatrix/vtk/vtk-5.10.1/Utilities/vtkalglib \
                D:/skewmatrix/latticefx/src \
                D:/skewmatrix/poco/poco/include \
                D:/skewmatrix/boost/boost_1_53_0/boost \
                D:/skewmatrix/osg/osg_285/include

FORMS    += MainWindow.ui

CONFIG( debug, debug|release ) {
    LIBS += -LD:/skewmatrix/latticefx/bld/lib/Debug/ -llatticefx_cored \
            -LD:/skewmatrix/latticefx/bld/lib/Debug/ -llatticefx_utils_vtkd \
            -LD:/skewmatrix/latticefx/bld/lib/Debug/ -llatticefx_translators_vtkd \
            -LD:/skewmatrix/latticefx/bld/lib/Debug/ -llatticefx_core_vtkd \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkCommon \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkIO \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkFiltering \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkRendering \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkHybrid \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkGraphics \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtkImaging \
            -LD:/skewmatrix/vtk/vtk-5.10.1/bld11/bin/Debug/ -lvtksys \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoFoundationd \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoUtild \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoXMLd \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoFoundationd \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoFoundationd \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoUtild \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoXMLd \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoFoundationd \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoUtild \
            -LD:/skewmatrix/poco/poco/lib64/ -lPocoXMLd \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_thread-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_date_time-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_filesystem-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_system-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_chrono-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_program_options-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/boost/boost_1_53_0/stage/lib -lboost_serialization-vc110-mt-gd-1_53 \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgGAd \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgTextd \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgViewerd \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgSimd \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgDBd \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgUtild \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -losgd \
            -LD:/skewmatrix/osg/osg_285/bld/lib/ -lOpenThreadsd \
			-LD:/skewmatrix/osgworks/bld11/lib/Debug/ -losgwToolsd \
            -LD:/skewmatrix/osgworks/bld11/lib/Debug/osgwToolsd

} else {
    # release
}
