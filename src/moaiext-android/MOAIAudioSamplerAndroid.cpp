#include "pch.h"
#include <jni.h>

#include <moaiext-android/moaiext-jni.h>
#include <moaiext-android/MOAIAudioSamplerAndroid.h>

#define BITS_PER_FRAME 16
#define BYTES_PER_FRAME 16

#define LOG_NO_CLASS(cls)	USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find java class %s", cls)
#define LOG_NO_METH(meth)	USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find java method %s", meth)
#define LOG_NO_FIELD(field)	USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find static field %s", field)

extern JavaVM* jvm;

static MOAIAudioSamplerAndroid *currentInstance = NULL;

MOAIAudioSamplerAndroid::MOAIAudioSamplerAndroid ( ) :
  mNumFrequency(0),
  numChannels(0),
  mNumChannels(0),
  mBitEncoding(0),
  mMaxBufferSizeInBytes(0),
  mFramesPerSample(0),
  mBufferAryLen(0),
  mBufferAry(NULL),
  mBufferReadSizeInBytes(NULL),
  isActive(false),
  mAudioRecorder(NULL)
{
  USLog::Print ( "Calling audio sampler constructor" );
  RTTI_SINGLE ( MOAINode )
    ;
  USLog::Print ( "Assigning current instance" );
  currentInstance = this;
  USLog::Print ( "Done constructor" );
}

MOAIAudioSamplerAndroid::~MOAIAudioSamplerAndroid ( ) {
  USLog::Print ( "Calling audio sampler destructor" );
  if (this->mBufferAry){
    for ( u32 i = 0; i < this->mBufferAryLen; i++ ) {
      free( this->mBufferAry[i] );
    }
    free( this->mBufferAry );
    free( this->mBufferReadSizeInBytes );
  }

  if ( this->mAudioRecorder != NULL ) {
    USLog::Print ( "Releasing instance" );
    JNI_GET_ENV ( jvm, env );
    jclass recorder = env->FindClass ( "android/media/AudioRecord" );
    if ( recorder == NULL ) {
      fprintf ( stderr, "Unable to find java class %s", "android/media/AudioRecord" );
    } else {
      jmethodID releaseID = env->GetMethodID ( recorder, "release", "()V" );
      if ( releaseID == NULL ) {
        fprintf ( stderr, "Unable to find method %s", "release" );
      } else {
        env->CallVoidMethod ( this->mAudioRecorder, releaseID );
        this->mAudioRecorder = NULL;
      }
    }
  }
  USLog::Print ( "Finished audio sampler destructor" );
}

int MOAIAudioSamplerAndroid::_setFrequency ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "UN" )
    ;
  self->mNumFrequency = state.GetValue < u32 >( 2, 44100 );
  return 0;
}

int MOAIAudioSamplerAndroid::_setNumChannels ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "UN" )
    ;
  self->numChannels = state.GetValue < u32 >(2, 2 );
  /* Need to translate the number of channels into the equivalent java-side
   * constant */
  cc8* channelFieldName;
  switch ( self->numChannels ) {
    case 1:
      channelFieldName = "CHANNEL_IN_MONO";
      break;
    case 2:
      channelFieldName = "CHANNEL_IN_STEREO";
      break;
    default:
      channelFieldName = "CHANNEL_IN_DEFAULT";
      break;
  }

  JNI_GET_ENV ( jvm, env );

  jclass audioFmt = env->FindClass ( "android/media/AudioFormat" );
  if ( audioFmt == NULL ) {
    USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find java class %s",
                   "android/media/AudioFormat" );
    return 0;
  }
  jfieldID fid = env->GetStaticFieldID ( audioFmt, channelFieldName, "I" );
  if ( fid == NULL ) {
    USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find java field %s",
                   channelFieldName );
    return 0;
  }
  self->mNumChannels = (u32)env->GetStaticIntField ( audioFmt, fid );

  return 0;
}

int MOAIAudioSamplerAndroid::_prepareBuffer ( lua_State *L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "UNN" )
    ;
  double sec = state.GetValue < float > ( 2, 1 );
  self->mBufferAryLen = state.GetValue < u32 > ( 3, 5 );

  JNI_GET_ENV ( jvm, env );

  /* Get the constant for the encoding */
  jclass audioFmt = env->FindClass ( "android/media/AudioFormat" );
  if ( audioFmt == NULL ) {
    LOG_NO_CLASS( "android/media/AudioFormat" );
    return 0;
  }
  jfieldID fmtFid = env->GetStaticFieldID ( audioFmt, "ENCODING_PCM_16BIT", "I" );
  if ( fmtFid == NULL ) {
    LOG_NO_FIELD( "ENCODING_PCM_16BIT" );
    return 0;
  }
  self->mBitEncoding = (u32)env->GetStaticIntField ( audioFmt, fmtFid );

  self->mFramesPerSample = (int)ceil( sec * self->mNumFrequency );

  self->mBufferAry = (short**) malloc ( self->mBufferAryLen * sizeof(short*) );
  self->mBufferReadSizeInBytes = (size_t*) malloc ( self->mBufferAryLen * sizeof(size_t) );
  memset ( self->mBufferReadSizeInBytes, 0, self->mBufferAryLen * sizeof(size_t) );

  u32 bytesPerFrame = ( BITS_PER_FRAME / 8 ) * self->numChannels;
  u32 bufsize = bytesPerFrame * self->mFramesPerSample;
  self->mMaxBufferSizeInBytes = bufsize;

  assert ( self->mBufferAry );
  for ( u32 i = 0; i < self->mBufferAryLen; ++i ) {
    self->mBufferAry[i] = (short*) malloc ( bufsize );
    assert ( self->mBufferAry[i] );
    /* TODO: Get a reference for the buffer? */
  }

  return 0;
}

int MOAIAudioSamplerAndroid::_start ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;

  JNI_GET_ENV ( jvm, env );

  /* Create the recorder object */
  jclass recorder = env->FindClass ( "android/media/AudioRecord" );
  if ( recorder == NULL ) {
    LOG_NO_CLASS( "android/media/AudioRecord" );
    return 0;
  }
  jmethodID getBufSize = env->GetStaticMethodID ( recorder, "getMinBufferSize",
      "(III)I");
  if ( getBufSize == NULL ) {
    LOG_NO_FIELD( "getMinBufferSize" );
    return 0;
  }
  size_t minAryLen = ( size_t )env->CallStaticObjectMethod (
      recorder, getBufSize, self->mNumFrequency, self->mNumChannels, self->mBitEncoding
  );
  if ( self->mBufferAryLen < minAryLen ) {
    self->mBufferAryLen = minAryLen;
  }

  /* Try to create the recorder object */
  jmethodID cons = env->GetMethodID ( recorder, "<init>", "(IIIII)V" );
  if ( cons == NULL ) {
    USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find constructor for AudioRecord" );
    return 0;
  }
  jclass audioSrc = env->FindClass ( "android/media/MediaRecorder$AudioSource" );
  if ( audioSrc == NULL ) {
    LOG_NO_CLASS( "android/media/MediaRecorder$AudioSource" );
    return 0;
  }
  jfieldID srcTypeFid = env->GetStaticFieldID ( audioSrc, "VOICE_RECOGNITION", "I" );
  if ( srcTypeFid == NULL ) {
    USLog::Print (
        "MOAIAudioSamplerAndroid: Unable to find static field %s, defaulting to %s",
        "VOICE_RECOGNITION", "MIC" );
    srcTypeFid = env->GetStaticFieldID ( audioSrc, "MIC", "I" );
    if ( srcTypeFid == NULL ) {
      LOG_NO_FIELD( "MIC" );
      return 0;
    }
  }
  u32 voiceRecogType = env->GetStaticIntField ( audioSrc, srcTypeFid );
  self->mAudioRecorder = env->NewObject ( recorder, cons,
      voiceRecogType, self->mNumFrequency, self->mNumChannels,
      self->mBitEncoding, self->mBufferAryLen );
  /* Catch exception */
  jthrowable exc = env->ExceptionOccurred();
  if ( exc ) {
    env->ExceptionDescribe ( );
    env->ExceptionClear ( );
    USLog::Print ( "MOAIAudioSamplerAndroid: Exception trying to create AudioRecord" );
    return 0;
  }

  /* Set a callback to periodically get the data */
  jmethodID setNotificationPeriodID = env->GetMethodID (
      recorder, "setPositionNotificationPeriod", "(I)I" );
  if ( setNotificationPeriodID == NULL ) {
    LOG_NO_METH( "setPositionNotificationPeriod" );
    return 0;
  }
  u32 success = env->CallIntMethod (
      self->mAudioRecorder, setNotificationPeriodID, self->mFramesPerSample );
  /* Set the interval */
  jmethodID setUpdateListenerID = env->GetMethodID (
      recorder, "setRecordPositionUpdateListener",
      "(Landroid/media/AudioRecord/OnRecordPositionUpdateListener;Landroid/os/Handler;)V"
  );
  /* Create the handler object */
  jclass posUpdateListener = env->FindClass (
      "com/ziplinegames/moai/MoaiUpdateListener" );
  if ( posUpdateListener == NULL ) {
    LOG_NO_CLASS( "com/ziplinegames/moai/MoaiUpdateListener" );
    return 0;
  }
  jmethodID createListenerID = env->GetStaticMethodID ( posUpdateListener, "createListener",
      "()Landroid/media/AudioRecord/OnRecordPositionUpdateListener;" );
  if ( createListenerID == NULL ) {
    LOG_NO_METH( "createListener" );
    return 0;
  }
  jobject listener = env->CallStaticObjectMethod ( posUpdateListener, createListenerID );
  /* Set the listener */
  if ( setUpdateListenerID == NULL ) {
    LOG_NO_METH( "setRecordPositionUpdateListener" );
    return 0;
  }
  env->CallVoidMethod ( self->mAudioRecorder, setUpdateListenerID, listener );

  /* Start recording */
  jmethodID startRecordingID = env->GetMethodID ( recorder, "startRecording", "()V" );
  if ( startRecordingID == NULL ) {
    LOG_NO_METH( "startRecording" );
    return 0;
  }
  env->CallVoidMethod ( self->mAudioRecorder, startRecordingID );
  /* Catch exception */
  exc = env->ExceptionOccurred();
  if ( exc ) {
    env->ExceptionDescribe ( );
    env->ExceptionClear ( );
    USLog::Print ( "MOAIAudioSamplerAndroid: Exception trying to start recording" );
    return 0;
  }

  self->isActive = true;

  return 0;
}

int MOAIAudioSamplerAndroid::_read ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  cc8 *tn = state.GetValue < cc8 * > ( 2, "float" );

  int tnid = -1;
  if( strcmp( tn, "float" ) == 0 ){
    tnid = 0;
  } else if( strcmp( tn, "char" ) == 0 ){
    tnid = 8;
  } else if( strcmp( tn, "short" ) == 0 ){
    tnid = 16;
  } else if( strcmp( tn, "raw" ) == 0 ){
    tnid = 255;
  } else {
    lua_pushnil(L);
    return 1;
  }

  for ( u32 i = 0; i < self->mBufferAryLen; i++ ) {
    /* TODO: Read audio out of buffer, return a lua table */
  }

  /* If no data read in loop, return nil */
  lua_pushnil ( L );
  return 1;
}

int MOAIAudioSamplerAndroid::_stop ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;

  JNI_GET_ENV ( jvm, env );

  jclass recorder = env->FindClass ( "android/media/AudioRecord" );
  if ( recorder == NULL ) {
    USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find java class %s",
                   "android/media/AudioRecord" );
    return 0;
  }

  jmethodID stopID = env->GetMethodID ( recorder, "stop", "()V" );
  if ( stopID == NULL ) {
    USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find method %s",
                   "stop" );
    return 0;
  }
  env->CallVoidMethod ( self->mAudioRecorder, stopID );
  /* Catch exception */
  jthrowable exc = env->ExceptionOccurred();
  if ( exc ) {
    env->ExceptionDescribe ( );
    env->ExceptionClear ( );
    USLog::Print ( "MOAIAudioSamplerAndroid: Exception trying to stop recording" );
    return 0;
  }

  self->isActive = false;
  return 0;
}

int MOAIAudioSamplerAndroid::_pause ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  self->isActive = false;
  return 0;
}

int MOAIAudioSamplerAndroid::_resume ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  self->isActive = true;
  return 0;
}

int MOAIAudioSamplerAndroid::_flush ( lua_State* L ) {
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  /* TODO: Flush the buffer of read samples */
  return 0;
}

void MOAIAudioSamplerAndroid::RegisterLuaClass ( MOAILuaState& state ) {
  USLog::Print ( "Registering AudioSamplerAndroid class..." );
  MOAINode::RegisterLuaClass ( state );
  USLog::Print ( "Registered AudioSamplerAndroid class!" );
}

void MOAIAudioSamplerAndroid::RegisterLuaFuncs ( MOAILuaState& state ) {

  USLog::Print ( "Registering functions for MOAIAudioSamplerAndroid..." );
  MOAINode::RegisterLuaFuncs ( state );

  luaL_Reg regTable [] = {
    { "setNumChannels", _setNumChannels },
    { "setFrequency", _setFrequency },
    { "prepareBuffer", _prepareBuffer },
    { "start", _start },
    { "read", _read },
    { "stop", _stop },
    { "pause", _pause },
    { "resume", _resume },
    { "flush", _flush },
    { NULL, NULL }
  };

  luaL_register ( state, 0, regTable );
  USLog::Print ( "Done registering functions for MOAIAudioSamplerAndroid" );
}

void MOAIAudioSamplerAndroid::HandlePeriodicNotification ( ) {
  USLog::Print ( "Time to flush data out" );
}

/* Callback JNI Methods */
extern "C" void Java_com_ziplinegames_moai_MoaiAudioListener_AKUAudioListenerPeriodicNotification (
    JNIEnv* env, jclass obj ) {
  USLog::Print ( "Entering periodic notification handler" );
  currentInstance->HandlePeriodicNotification ();
}

extern "C" void Java_com_ziplinegames_moai_MoaiAudioListener_AKUAudioListenerMarkerReached (
    JNIEnv* env, jclass obj ) {
  USLog::Print ( "Entering marker reached handler" );
}
