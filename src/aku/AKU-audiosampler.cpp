#ifndef MOAI_OS_QNX
#include <aku/AKU-audiosampler.h>

#include <moaiext-audiosampler/MOAIAudioSampler.h>

void AKUAudioSamplerInit () {

    REGISTER_LUA_CLASS ( MOAIAudioSampler )
}
#endif
