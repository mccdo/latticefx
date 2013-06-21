/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2013 by Ames Laboratory
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

#ifndef __LFX_CORE_CALLBACK_PROGRESS_H__
#define __LFX_CORE_CALLBACK_PROGRESS_H__ 1

#include <latticefx/core/Export.h>

namespace lfx
{
namespace core
{

/** \class ICallbackProgress CallbackProgress <latticefx/core/CallbackProgress.h>
\brief A interface class for that can be used to handle things like progress updates, cancel operations and messages
\details
A interface class for that can be used to handle things like
progress updates, cancel operations and messages. */
class LATTICEFX_EXPORT ICallbackProgress
{
public:
	ICallbackProgress();

	virtual bool checkCancel() { return false; }
	virtual void updateProgress(int /*0-100*/) {}
	virtual void sendMsg(const char* /*msg*/) {}

	void clearProg();
	void addToProgTot(int add);

	void computeProgAndUpdate(int add);

protected:
	int computeProg();
	void validateProgUpdate(int newProg);

protected:
	int _progCountTot;
	int _progCountCur;
	int _lastProg;
};

// core
}
// lfx
}


// __LFX_CORE_CALLBACK_PROGRESS_H__
#endif