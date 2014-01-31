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
#ifndef __LATTICEFX_CORE_VTK_STREAMLINE_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_STREAMLINE_RTP_OPERATION_H__ 1

#include <latticefx/core/vtk/VTKBaseRTP.h>
#include <latticefx/core/vtk/Export.h>
#include <vector>

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class VTKStreamlineRTP VTKStreamlineRTP.h <latticefx/core/vtk/VTKStreamlineRTP.h>
 \brief Class the creates an streamline polydata from a vtk dataset.
 \details This class takes a vtkDataObject in a ChannelDatavtkDataObject with the
 name vtkDataObject and creates a vtkPolyData with the vector field. */

class LATTICEFX_CORE_VTK_EXPORT VTKStreamlineRTP : public VTKBaseRTP
{
public:

    ///Default constructor
    VTKStreamlineRTP();

    ///Destructor
    virtual ~VTKStreamlineRTP();

	virtual std::string getClassName() const { return "VTKStreamlineRTP"; }

	void setMaxTime( float time );

	bool setSeedPtsBox( double minX, double maxX, double minY, double maxY, double minZ, double maxZ );
	bool setSeedPtsBox( const std::vector<double> &box );
	bool setSeedPtsCount( int x, int y, int z );
	bool setSeedPtsCount( const std::vector<int> &count );
	bool setIntegrationDir( int dir );
	bool setPropagationTime( float t );
	bool setIntegrationStepLen( float l );

	/// these may not be needed, currently not being used
	void setStreamArrows( int sa );
	void setStreamRibbons( int sr );
	void setLineDiameter( float d );
	void setArrowDiameter( float d );
	void setParticleDiameter( float d );

    ///We are going to be creating a ChannelDatavtkPolyData so we override the
    ///channel method since we do not have a ChannelData already
    virtual lfx::core::ChannelDataPtr channel( const lfx::core::ChannelDataPtr maskIn );

	virtual void dumpState( std::ostream &os );

protected:

	vtkPolyData* createSeedPoints( const std::vector<double> &bounds, const std::vector<double> &bbox, const std::vector<int> &numPts );

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

protected:
	std::vector<double> _bbox;
	std::vector<int> _numPts;
	int _integrationDirection;
    int _streamArrows;
    int _streamRibbons;

    float _propagationTime;
    float _integrationStepLength;
    float _lineDiameter;
    double _arrowDiameter;
    float _particleDiameter;

	float _maxTime;
};

typedef boost::shared_ptr< VTKStreamlineRTP > VTKStreamlineRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_STREAMLINE_RTP_OPERATION_H__
#endif