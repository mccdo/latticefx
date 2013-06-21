#include <latticefx/core/CallbackProgress.h>

namespace lfx
{
namespace core
{

ICallbackProgress::ICallbackProgress()
	: _progCountTot ( 0 ),
	  _progCountCur ( 0 ),
	  _lastProg ( -1 )
{
}

void ICallbackProgress::clearProg()
{
	_progCountTot = 0;
	_progCountCur = 0;
	_lastProg = -1;
}

void ICallbackProgress::addToProgTot(int add) 
{ 
	_progCountTot += add; 
}

void ICallbackProgress::computeProgAndUpdate(int add)
{
	_progCountCur += add;
	validateProgUpdate(computeProg());
}

int ICallbackProgress::computeProg()
{
	float prog = (float)_progCountCur / (float)_progCountTot;
	if (prog > 1.0f) prog = 1.0f;

	return (int)(prog * 100.);
}

void ICallbackProgress::validateProgUpdate(int newProg)
{
	if (newProg == _lastProg) return;
	_lastProg = newProg;
	updateProgress(newProg);
}


// core
}
// lfx
}
