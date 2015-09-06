#include "WinTimer.h"
#include "Windowsx.h"
using namespace GUIComponents;
//--------------------------------------------------------------------------------------
WinTimer::WinTimer(HWND hWnd, int id)
:   hHandle(hWnd),
    iTime(0),
	m_id(id)
{
}
//--------------------------------------------------------------------------------------
WinTimer::WinTimer(const WinTimer& o)
{

}
//--------------------------------------------------------------------------------------
WinTimer::~WinTimer()
{

}
//--------------------------------------------------------------------------------------
void WinTimer::Update(int max_range)
{
	iTime++;
	if (iTime == max_range)
		Kill();
}
//--------------------------------------------------------------------------------------
void WinTimer::Set(int val)
{
	iTime = 1;
	SetTimer(hHandle, m_id, val, NULL);
}
//--------------------------------------------------------------------------------------
void WinTimer::Kill()
{
	KillTimer(hHandle, m_id);
}
//--------------------------------------------------------------------------------------