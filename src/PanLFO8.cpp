#include "PanLFO8.h"

//-----------------------------------------------------------------------------
CPanLFO8::CPanLFO8()
{
	Bpm = 0;
	StartPos = 0;
	pos = 0;
	xBeat = 0;
	lfofreq = 0;
	align = 0;
	Delay = 0;
	m_Dry = 0;
	m_Wet = 0;
	ZeroMemory(SliderValue, 2 * sizeof(float));
	ZeroMemory(coeff, NB_CHAN * sizeof(float));
	ZeroMemory(out, NB_CHAN * sizeof(float));
	ZeroMemory(in, NB_CHAN * sizeof(float));
	ZeroMemory(in_panLFO, NB_CHAN * sizeof(float));
}
//-----------------------------------------------------------------------------
CPanLFO8::~CPanLFO8()
{ 

}
//-----------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnLoad()
{	
	StartPos=-1;
	
	DeclareParameterSwitch(&align,ID_SWITCH_1,"Align to Beat","Beat",0.0f);
	DeclareParameterSlider(&SliderValue[0],ID_SLIDER_1,"Wet/Dry","D/W",1.0f);
	DeclareParameterSlider(&SliderValue[1],ID_SLIDER_2,"LFO Rate","Rate",0.5f);

	OnParameter(ID_SLIDER_1);
	OnParameter(ID_SLIDER_2);
	return S_OK;
}
//-----------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "DJ CEL";
	infos->PluginName = "PanLFO";
	infos->Description = "Pan synchonysed on the beat";
	infos->Version = "2.1";
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
	switch(id)
	{
		case ID_SLIDER_1:
			m_Wet = SliderValue[0];
			m_Dry = 1.0f - m_Wet;
			break;

		case ID_SLIDER_2:
				 if (SliderValue[1]==0.0f)                               Delay=0.125f;
			else if (SliderValue[1]>0.0f     && SliderValue[1]<0.125f)   Delay=0.25f;
			else if (SliderValue[1]>=0.125f  && SliderValue[1]<0.25f)    Delay=0.5f;
			else if (SliderValue[1]>=0.25f   && SliderValue[1]<0.375f)   Delay=1.0f;
			else if (SliderValue[1]>=0.375f  && SliderValue[1]<0.5f)     Delay=2.0f;
			else if (SliderValue[1]>=0.5f    && SliderValue[1]<0.625f)   Delay=4.0f;
			else if (SliderValue[1]>=0.625   && SliderValue[1]<0.75f)    Delay=6.0f;
			else if (SliderValue[1]>=0.75f   && SliderValue[1]<0.875f)   Delay=8.0f;
			else if (SliderValue[1]>=0.875f  && SliderValue[1]<1.0f)     Delay=16.0f;
			else if (SliderValue[1]==1.0f)                               Delay=32.0f;

			lfofreq = 1 / Delay;
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
			if (Delay == 0.0f) sprintf_s(outParam, outParamSize,"OFF");
			else if (Delay < 1.0f) sprintf_s(outParam, outParamSize,"1/%.0f beat", 1 / Delay);
			else sprintf_s(outParam, outParamSize,"%.0f beat(s)", Delay);
			break;

	}
	return S_OK;
}
//-------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnStart()
{
	Bpm = SongBpm?SongBpm:(SampleRate/2); // by default 120bpm
	StartPos = int(SongPosBeats * Bpm);

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnStop()
{
	StartPos = 0;
	
	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CPanLFO8::OnProcessSamples(float *buffer,int nb)
{
	float y;
	int i,ch;

	in[0] = 0.0f;
        in[1] = 0.0f;
	out[0] = 0.0f;
	out[1] = 0.0f;
	in_panLFO[0] = 0.0f;
	in_panLFO[1] = 0.0f;

	Bpm = SongBpm?SongBpm:(SampleRate/2);
	pos = int(SongPosBeats * Bpm);
	
	for(i=0;i<nb;i++)
	{
		xBeat = (pos + i - StartPos) / float(Bpm);  // 'x' beat(s) from StartPos
		y = LFO(SINE, lfofreq, xBeat, 0);  // LFO sur 'Delay' beat(s)   [on prend le sin pour que y(0)=0.5]
		
		UpdateCoeffPan(y);

		for(ch=0;ch<NB_CHAN;ch++) // 0: left channel, 1:right channel
		{
			in[ch] = buffer[2*i+ch];

			in_panLFO[ch] = coeff[ch] * in[ch];

		        out[ch] = m_Dry * in[ch] + m_Wet * in_panLFO[ch];

			buffer[2*i+ch] = out[ch];
		}
	}
	return S_OK;
}

//------------------------------------------------------------------------------
float CPanLFO8::LFO(LFOCURVE type, float freq, float x, float phi)
{
	float ValLFO = 0.0f;
	double phiRadian = 0.0f;
	float t = freq * x;
	double value;
	
	switch(type)
	{
		case SINE:
			phiRadian = 2.0f * M_PI * phi / 360.0f;
			value = 2.0f * M_PI * t + phiRadian;
			ValLFO = (1.0f - (float) sin(value) ) * 0.5f;
			break;

		case SAWTOOTH:
			ValLFO = 2.0f * (t - (float) floor(t + 0.5f));
			break;

		case TRIANGLE:
			ValLFO = 2.0f * (t - (float) floor(t + 0.5f)) * (float) pow(-1, floor(t + 0.5f));
			break;
	}
	
	return ValLFO;
}
//------------------------------------------------------------------------------
void CPanLFO8::UpdateCoeffPan(float x)
{
	if(x>=0 && x<=0.5f)
	{
		coeff[0] = 1.0f;
		coeff[1] = 2.0f * x;
	}
	else if (x>0.5f && x<=1.0f)
	{
		coeff[0] = 2.0f * (1.0f - x);
		coeff[1] = 1.0f;
	}
}

