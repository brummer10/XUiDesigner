
// this is the a simple example cc file to been parsed by XUiDesigner

#include <cmath>
#include <algorithm>

typedef enum
{
   input0,
   output0,
   bypass,
   GAIN, // , 0.0, -60.0, 20.0, 0.1 
} PortIndex;


namespace gain {

class Dsp {
private:
    uint32_t fSampleRate;
    float Gain;
    float *Gain_;
    double Smooth[2];

public:
    void connect(uint32_t port,void* data);
    void del_instance(Dsp *p);
    void clear_state_f();
    void init(uint32_t sample_rate);
    void compute(int count, float *input0, float *output0);
    Dsp();
    ~Dsp();
};

Dsp::Dsp() {
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
    for (int l = 0; l < 2; l = l + 1) Smooth[l] = 0.0;
}

inline void Dsp::init(uint32_t sample_rate)
{
    fSampleRate = sample_rate;
    clear_state_f();
}

void always_inline Dsp::compute(int count, float *input0, float *output0)
{
#define Gain (*Gain_)
    double Slow = 0.0010000000000000009 * std::pow(10.0, 0.050000000000000003 * double(Gain));
    for (int i = 0; i < count; i = i + 1) {
        Smooth[0] = Slow + 0.999 * Smooth[1];
        output0[i] = float(double(input0[i]) * Smooth[0]);
        Smooth[1] = Smooth[0];
    }
#undef Gain
}

void Dsp::connect(uint32_t port,void* data)
{
    switch ((PortIndex)port)
    {
    case GAIN: 
        Gain_ = static_cast<float*>(data);
        break;
    default:
        break;
    }
}

Dsp *plugin() {
    return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
    delete p;
}

} // end namespace gain
