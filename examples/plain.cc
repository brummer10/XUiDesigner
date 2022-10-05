
// this is the basic cc file format to been parsed by XUiDesigner
// don't use numbers or spaces in the filename!

#include <cmath>
#include <algorithm>

#define always_inline inline __attribute__((__always_inline__))

// PortIndex define all control audio and atom ports used in a plugin
// to implement a bypass switch just add "bypass," to the enum as shown below
// controlports should define as well the range and stepsize like in this example
//
// gain, // 0.5, 0.0, 1.0, 0.01
//
// were the order is // default, min, max, stepsize
// XUiDesigner will parse this values to create the Controllers for the UI
// and the needed ttl files

typedef enum
{
   input0,
   output0,
   bypass,
} PortIndex;

// namespace should be the basename of the file in lowercase

namespace plain {

// the basic DSP class to define all variables needed in the plugin 

class Dsp {
private:
    uint32_t fSampleRate;

public:
    void connect(uint32_t port, void* data);
    void del_instance(Dsp *p);
    void clear_state_f();
    void init(uint32_t sample_rate);
    void compute(int count, float *input0, float *output0);
    Dsp();
    ~Dsp();
};

// the class constructor, some variables may be inited here

Dsp::Dsp() {
}

// the class deconstructor, some variables may be deleted here

Dsp::~Dsp() {
}

// clear_state_f is called before the plugin run,
// so here all initial values could be set to the used variables

inline void Dsp::clear_state_f()
{

}

// this is the call to init the plugin before run

inline void Dsp::init(uint32_t sample_rate)
{
    fSampleRate = sample_rate;
    clear_state_f();
}

// this is the dsp run part, implement the math here

void always_inline Dsp::compute(int count, float *input0, float *output0)
{
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
        output0[i0] = input0[i0];
    }
}

// here variables could be connected with ports, so the host could forward
// controller values from the UI to the dsp, connect a variable like this
// in the switch
//
//    case gain:
//        gain_ = static_cast<float*>(data); 
//        break;
//
// Note that the variable needs to be a pointer defined in the DSP class
// and is only valid in the compute loop.
// The case switch statement must be part of the PortIndex enum.

void Dsp::connect(uint32_t port,void* data)
{
    switch ((PortIndex)port)
    {
    default:
        break;
    }
}

// this will forward a pointer to a new instance of the plugin

Dsp *plugin() {
    return new Dsp();
}

// this will delete the plugin instance

void Dsp::del_instance(Dsp *p)
{
    delete p;
}

} // end namespace
