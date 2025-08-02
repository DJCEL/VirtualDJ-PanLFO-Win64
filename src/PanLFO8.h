#ifndef PANLFO8_H
#define PANLFO8_H

#include "VdjDsp8.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>


//---------------------------------------------------------------------------
// Class definition
//---------------------------------------------------------------------------
class CPanLFO8 : public IVdjPluginDsp8
{
public:
	CPanLFO8();
	~CPanLFO8();
	HRESULT VDJ_API OnLoad();
	HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
	ULONG   VDJ_API Release();
	HRESULT VDJ_API OnParameter(int id);
	HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize);
	HRESULT VDJ_API OnGetUserInterface(TVdjPluginInterface8 *pluginInterface);
	HRESULT VDJ_API OnStart();
	HRESULT VDJ_API OnStop();
	HRESULT VDJ_API OnProcessSamples(float *buffer, int nb);

private:
	typedef enum _ID_Interface
	{
		ID_INIT,
		ID_SLIDER_1,
		ID_SLIDER_2,
		ID_SLIDER_3,
	} ID_Interface;

	typedef enum _LFOCURVE
	{
		SINE,
		TRIANGLE,
		SAWTOOTH,
		SQUARE
	} LFOCURVE;

	const int MAX_LFOCURVE = 4;

	// Plugin Interface
	HRESULT OnSlider(int id);
	float m_SliderValue[3];
	double m_Delay;
	float m_Dry, m_Wet;
	LFOCURVE m_LFOcurve;

	// Other variables & functions
	float coeffPanLeft, coeffPanRight;
	void UpdateAudioBalance(float x);

	// LFO
	double m_SongPosBeatsStart;
    	float LFO(LFOCURVE type, double x);
};

#endif /* PANLFO8_H */ 


