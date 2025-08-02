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
	HRESULT VDJ_API OnProcessSamples(float *buffer,int nb);

private:
	typedef enum _ID_Interface
	{
		ID_SWITCH_1,
		ID_SLIDER_1,
		ID_SLIDER_2,
	} ID_Interface;

	typedef enum _LFOCURVE
	{
		SINE,
		SQUARE,
		TRIANGLE,
		SAWTOOTH
	} LFOCURVE;


	#define NB_CHAN 2


	// Plugin Interface
	int align;
	float SliderValue[2];

	// Other variables & functions
	float m_Delay;
	float m_Dry, m_Wet;
	float coeffPan[NB_CHAN];
	float out[NB_CHAN], in[NB_CHAN], in_panLFO[NB_CHAN];
	
	// LFO
	int Bpm;
	int StartPos;
	int pos;
	float xBeat;
	float lfofreq;

        float LFO(LFOCURVE type,float frq,float x,float phi); // phi in degree
	void UpdateCoeffPan(float x);
};

#endif /* PANLFO8_H */ 

