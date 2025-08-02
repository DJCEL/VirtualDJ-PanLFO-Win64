#include "PanLFO8.h"

//-----------------------------------------------------------------------------
CPanLFO8::CPanLFO8()
{
	m_Delay = 0;
	m_Dry = 0;
	m_Wet = 0;
	m_SongPosBeatsStart = 0;
	coeffPanLeft = 0;
	coeffPanRight = 0;
	m_LFOcurve = SINE;
	ZeroMemory(m_SliderValue, 3 * sizeof(float));
}
//-----------------------------------------------------------------------------
CPanLFO8::~CPanLFO8()
{ 

}
//-----------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnLoad()
{	
	HRESULT hr = S_FALSE;

	hr = DeclareParameterSlider(&m_SliderValue[0], ID_SLIDER_1, "Wet/Dry",  "D/W",1.0f);
	hr = DeclareParameterSlider(&m_SliderValue[1], ID_SLIDER_2, "LFO Rate", "R",0.5f);
	hr = DeclareParameterSlider(&m_SliderValue[2], ID_SLIDER_3, "LFO Curve", "C", 0.0f);
	
	OnParameter(ID_INIT);
	return S_OK;
}
//-----------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "DJ CEL";
	infos->PluginName = "PanLFO";
	infos->Description = "Pan synchronysed on the beat";
	infos->Version = "2.2";
	infos->Flags = 0x00;

	return S_OK;
}
//---------------------------------------------------------------------------
ULONG VDJ_API CPanLFO8::Release()
{
	delete this;
	return 0;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnGetUserInterface(TVdjPluginInterface8 *pluginInterface)
{
	pluginInterface->Type = VDJINTERFACE_DEFAULT;

	return S_OK;
}
//------------------------------------------------------------------------------
// User Interface
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnParameter(int id)
{
	if (id == ID_INIT)
	{
		for (int i = ID_SLIDER_1; i <= ID_SLIDER_3; i++) OnSlider(i);
	}
	else OnSlider(id);

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT CPanLFO8::OnSlider(int id)
{
	switch (id)
	{
		case ID_SLIDER_1:
			m_Wet = m_SliderValue[0];
			m_Dry = 1.0f - m_Wet;
			break;

		case ID_SLIDER_2:
			if (m_SliderValue[1] == 0.0f)										m_Delay = 0.125f;
			else if (m_SliderValue[1] > 0.0f && m_SliderValue[1] < 0.125f)		m_Delay = 0.25f;
			else if (m_SliderValue[1] >= 0.125f && m_SliderValue[1] < 0.25f)    m_Delay = 0.5f;
			else if (m_SliderValue[1] >= 0.25f && m_SliderValue[1] < 0.375f)	m_Delay = 1.0f;
			else if (m_SliderValue[1] >= 0.375f && m_SliderValue[1] < 0.5f)     m_Delay = 2.0f;
			else if (m_SliderValue[1] >= 0.5f && m_SliderValue[1] < 0.625f)		m_Delay = 4.0f;
			else if (m_SliderValue[1] >= 0.625 && m_SliderValue[1] < 0.75f)		m_Delay = 6.0f;
			else if (m_SliderValue[1] >= 0.75f && m_SliderValue[1] < 0.875f)	m_Delay = 8.0f;
			else if (m_SliderValue[1] >= 0.875f && m_SliderValue[1] < 1.0f)     m_Delay = 16.0f;
			else if (m_SliderValue[1] == 1.0f)									m_Delay = 32.0f;
			break;

		case ID_SLIDER_3:
			int LFOcurve_value = int(1.0f + m_SliderValue[2] * (MAX_LFOCURVE - 1.0f));
			switch (LFOcurve_value)
			{
				case 1:
					m_LFOcurve = SINE;
					break;
				case 2:
					m_LFOcurve = TRIANGLE;
					break;
				case 3:
					m_LFOcurve = SAWTOOTH;
					break;
				case 4:
					m_LFOcurve = SQUARE;
					break;
			}
			break;
	}

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnGetParameterString(int id, char *outParam, int outParamSize) 
{
	switch(id)
	{
		case ID_SLIDER_1:
			sprintf_s(outParam, outParamSize,"%d%%", int(m_Wet*100));
			break;

		case ID_SLIDER_2:
			if (m_Delay == 0.0f) sprintf_s(outParam, outParamSize,"OFF");
			else if (m_Delay < 1.0f) sprintf_s(outParam, outParamSize,"1/%.0f beat", 1 / m_Delay);
			else sprintf_s(outParam, outParamSize,"%.0f beat(s)", m_Delay);
			break;

		case ID_SLIDER_3:
			switch (m_LFOcurve)
			{
				case SINE:
					sprintf_s(outParam, outParamSize, "Sine");
					break;

				case TRIANGLE:
					sprintf_s(outParam, outParamSize, "Triangle");
					break;

				case SAWTOOTH:
					sprintf_s(outParam, outParamSize, "Sawtooth");
					break;

				case SQUARE:
					sprintf_s(outParam, outParamSize, "Square");
					break;
			}
			break;

	}
	return S_OK;
}
//-------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnStart()
{
	m_SongPosBeatsStart = SongPosBeats;

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnStop()
{
	m_SongPosBeatsStart = 0;
	
	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnProcessSamples(float *buffer, int nb)
{
	float y = 0;
	double xBeat = 0;
	double xDeltaBeat = 0;
	float inLeft, inRight, outLeft, outRight, PanLeft, PanRight = 0;
	int Bpm = SongBpm ? SongBpm : (SampleRate / 2);
	double xBeatFromStart = SongPosBeats - m_SongPosBeatsStart;

	for (int i = 0; i < nb; i++)
	{
		inLeft = buffer[2 * i];
		inRight = buffer[2 * i + 1];

		xDeltaBeat = double(i) / double(Bpm);
		xBeat = (xBeatFromStart + xDeltaBeat) / m_Delay;

		y = LFO(m_LFOcurve, xBeat);  // LFO on 'm_Delay' beat(s)
		UpdateAudioBalance(y);
		PanLeft = coeffPanLeft * inLeft;
		PanRight = coeffPanRight * inRight;

		outLeft = m_Dry * inLeft + m_Wet * PanLeft;
		outRight = m_Dry * inRight + m_Wet * PanRight;

		buffer[2 * i] = outLeft;
		buffer[2 * i + 1] = outRight;
	}
	
	return S_OK;
}
//------------------------------------------------------------------------------
void CPanLFO8::UpdateAudioBalance(float x)
{
	if(x>=0 && x<=0.5f)
	{
		coeffPanLeft = 1.0f;
		coeffPanRight = 2.0f * x;
	}
	else if (x>0.5f && x<=1.0f)
	{
		coeffPanLeft = 2.0f * (1.0f - x);
		coeffPanRight = 1.0f;
	}
}
//------------------------------------------------------------------------------
float CPanLFO8::LFO(LFOCURVE type, double x)
{
	// unipolar LFO [0, 1]

	double ValLFO = 0.0f;

	switch (type)
	{
		case SINE:
			ValLFO = (1.0f - sin(2.0f * M_PI * x)) * 0.5f;
			break;

		case SAWTOOTH:
			ValLFO = fmod(x, 1.0);
			//ValLFO = 1.0 - fmod(x, 1.0);
			break;

		case TRIANGLE:
			ValLFO = 1.0f - fabs(fmod(x + 0.25, 1.0) * 4.0 - 2.0);
			break;

		case SQUARE:
			ValLFO = (fmod(x, 1.0) < 0.5) ? 1.0f : 0.0f;
			break;
	}

	return float(ValLFO);
}



