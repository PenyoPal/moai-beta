#ifndef MOAIAUDIOSAMPLERANDROID_H
#define MOAIAUDIOSAMPLERANDROID_H

#include <jni.h>
#include <moaicore/moaicore.h>

class MOAIAudioSamplerAndroid : public virtual MOAINode {

  private:

    static int _setFrequency ( lua_State* L );
    static int _setNumChannels ( lua_State* L );
    static int _prepareBuffer ( lua_State* L );
    static int _start ( lua_State* L );
    static int _read ( lua_State* L );
    static int _stop ( lua_State* L );
    static int _pause ( lua_State* L );
    static int _resume ( lua_State* L );
    static int _flush ( lua_State* L );

    u32 mNumFrequency;
    u32 mNumChannels;
    u32 mBitEncoding;
    u32 mFramesPerSample;
    u32 numChannels;
    size_t mMaxBufferSizeInBytes;
    size_t mBufferAryLen;
    short **mBufferAry;
    size_t *mBufferReadSizeInBytes;
    jobject mAudioRecorder;

    bool isActive;
  public:

    DECL_LUA_FACTORY ( MOAIAudioSamplerAndroid )

    MOAIAudioSamplerAndroid ( );
    ~MOAIAudioSamplerAndroid ( );

    void HandlePeriodicNotification ( );

    void RegisterLuaClass ( MOAILuaState& state );
    void RegisterLuaFuncs ( MOAILuaState& state );
};

#endif
