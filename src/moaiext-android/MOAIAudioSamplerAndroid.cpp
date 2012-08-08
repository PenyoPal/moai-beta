#include "pch.h"
#include <jni.h>

#include <netinet/in.h>

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
  mBufferPosition(0),
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
  USLog::Print ( "Entering _setFrequency" );
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "UN" )
    ;
  self->mNumFrequency = state.GetValue < u32 >( 2, 44100 );
  USLog::Print ( "Finished _setFrequency" );
  return 0;
}

int MOAIAudioSamplerAndroid::_setNumChannels ( lua_State* L ) {
  USLog::Print ( "Entering _setNumChannels" );
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

  USLog::Print ( "Done _setNumChannels" );
  return 0;
}

int MOAIAudioSamplerAndroid::_prepareBuffer ( lua_State *L ) {
  USLog::Print ( "Entering _prepareBuffer" );
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

  USLog::Print ( "Done _prepareBuffer" );
  return 0;
}

int MOAIAudioSamplerAndroid::_start ( lua_State* L ) {
  USLog::Print ( "Entering _start" );
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;

  JNI_GET_ENV ( jvm, env );

  /* Create the recorder object */
  jclass recorder = env->FindClass ( "android/media/AudioRecord" );
  if ( recorder == NULL ) {
    LOG_NO_CLASS( "android/media/AudioRecord" );
    return 0;
  }
  USLog::Print ( "Created recorder" );
  jmethodID getBufSize = env->GetStaticMethodID ( recorder, "getMinBufferSize",
      "(III)I");
  if ( getBufSize == NULL ) {
    LOG_NO_FIELD( "getMinBufferSize" );
    return 0;
  }
  USLog::Print ( "Getting min buffer size" );
  size_t minAryLen = ( size_t )env->CallStaticObjectMethod (
      recorder, getBufSize, self->mNumFrequency, self->mNumChannels, self->mBitEncoding
  );
  if ( self->mBufferAryLen < minAryLen ) {
    self->mBufferAryLen = minAryLen;
  }

  USLog::Print ( "Getting recorder constructor" );
  /* Try to create the recorder object */
  jmethodID cons = env->GetMethodID ( recorder, "<init>", "(IIIII)V" );
  if ( cons == NULL ) {
    USLog::Print ( "MOAIAudioSamplerAndroid: Unable to find constructor for AudioRecord" );
    return 0;
  }
  USLog::Print ( "Getting audio source info" );
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
  USLog::Print ( "Setting audio source type" );
  u32 voiceRecogType = env->GetStaticIntField ( audioSrc, srcTypeFid );
  USLog::Print ( "Creating audio recorder object with args %d %d %d %d %d",
      voiceRecogType, self->mNumFrequency, self->mNumChannels,
      self->mBitEncoding, self->mBufferAryLen );
  self->mAudioRecorder = env->NewObject ( recorder, cons,
      voiceRecogType, self->mNumFrequency, self->mNumChannels,
      self->mBitEncoding, self->mBufferAryLen );
  /* Catch exception */
  USLog::Print ( "Trying to catch exception" );
  jthrowable exc = env->ExceptionOccurred();
  if ( exc ) {
    env->ExceptionDescribe ( );
    env->ExceptionClear ( );
    USLog::Print ( "MOAIAudioSamplerAndroid: Exception trying to create AudioRecord" );
    return 0;
  }
  if ( self->mAudioRecorder == NULL ) {
    USLog::Print ( "Audio recorder is nulL!" );
    return 0;
  }
  /*
  jfieldID initedStateID = env->GetStaticFieldID ( recorder, "STATE_INITIALIZED", "I" );
  u32 initedState = env->GetStaticIntField ( recorder, initedStateID );
  jmethodID getStateID = env->GetMethodID ( recorder, "getState", "()I" );
  if ( getStateID == NULL ) {
    LOG_NO_METH ( "getState" );
    return 0;
  }
  u32 recorderState = env->CallIntMethod ( self->mAudioRecorder, getStateID );
  USLog::Print ( "Recorder state is %d", recorderState );
  if ( recorderState != initedState ) {
    USLog::Print ( "Recorder is not in an inited state!" );
    return 0;
  }
  */

  USLog::Print ( "Setting notification period" );
  /* Set a callback to periodically get the data */
  /* Set the interval */
  jmethodID setNotificationPeriodID = env->GetMethodID (
      recorder, "setPositionNotificationPeriod", "(I)I" );
  if ( setNotificationPeriodID == NULL ) {
    LOG_NO_METH( "setPositionNotificationPeriod" );
    return 0;
  }
  USLog::Print ( "Setting notification period to %d", self->mFramesPerSample / 10 );
  u32 success = env->CallIntMethod (
      self->mAudioRecorder, setNotificationPeriodID, self->mFramesPerSample / 10 );
  USLog::Print ( "Got success code from setting period %d", success );
  if ( success != 0 ) {
    USLog::Print ( "Error setting notification period: %d", success );
    return 0;
  }
  USLog::Print ( "Getting handler class" );
  /* Create the handler object */
  jclass posUpdateListener = env->FindClass (
      "com/ziplinegames/moai/MoaiAudioListener" );
  if ( posUpdateListener == NULL ) {
    LOG_NO_CLASS( "com/ziplinegames/moai/MoaiAudioListener" );
    return 0;
  }
  USLog::Print ( "Getting handler creater" );
  jmethodID createListenerID = env->GetStaticMethodID ( posUpdateListener, "createListener",
      "()Landroid/media/AudioRecord$OnRecordPositionUpdateListener;" );
  if ( createListenerID == NULL ) {
    LOG_NO_METH( "createListener" );
    return 0;
  }
  USLog::Print ( "Creating handler object" );
  self->updateListener = env->CallStaticObjectMethod ( posUpdateListener, createListenerID );
  if ( self->updateListener == NULL ) {
    USLog::Print ( "failed to create listener object" );
    return 0;
  }
  USLog::Print ( "Getting notification handler setter method" );
  jmethodID setUpdateListenerID = env->GetMethodID (
      recorder, "setRecordPositionUpdateListener",
      "(Landroid/media/AudioRecord$OnRecordPositionUpdateListener;)V"
  );
  /* Set the listener */
  if ( setUpdateListenerID == NULL ) {
    LOG_NO_METH( "setRecordPositionUpdateListener" );
    return 0;
  }
  USLog::Print ( "Setting handler to new object" );
  env->CallVoidMethod ( self->mAudioRecorder, setUpdateListenerID, self->updateListener );

  /* Start recording */
  USLog::Print ( "Getting starter method" );
  jmethodID startRecordingID = env->GetMethodID ( recorder, "startRecording", "()V" );
  if ( startRecordingID == NULL ) {
    LOG_NO_METH( "startRecording" );
    return 0;
  }
  USLog::Print ( "Start recording" );
  env->CallVoidMethod ( self->mAudioRecorder, startRecordingID );
  /* Catch exception */
  USLog::Print ( "Checking for exceptions" );
  exc = env->ExceptionOccurred();
  if ( exc ) {
    env->ExceptionDescribe ( );
    env->ExceptionClear ( );
    USLog::Print ( "MOAIAudioSamplerAndroid: Exception trying to start recording" );
    return 0;
  }

  self->isActive = true;

  USLog::Print ( "Done _start" );
  return 0;
}

int MOAIAudioSamplerAndroid::_read ( lua_State* L ) {
  USLog::Print ( "Entering _read" );
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

  JNI_GET_ENV ( jvm, env );

  jbyteArray readData = env->NewByteArray ( self->mBufferAryLen );

  jclass recorder = env->FindClass ( "android/media/AudioRecord" );
  if ( recorder == NULL ) {
    LOG_NO_CLASS( "android/media/AudioRecord" );
    lua_pushnil ( L );
    return 1;
  }
  jmethodID readID = env->GetMethodID ( recorder, "read", "([BII)I" );
  if ( readID == NULL ) {
    LOG_NO_METH ( "read" );
    lua_pushnil ( L );
    return 1;
  }
  u32 numRead = env->CallIntMethod ( self->mAudioRecorder, readID,
      readData, self->mBufferPosition, self->mBufferAryLen );
  if ( numRead < 0 ) {
    USLog::Print ( "Error reading data" );
    lua_pushnil ( L );
    return 1;
  }
  jboolean isCopy;
  jbyte *buf = env->GetByteArrayElements ( readData, &isCopy );

  if ( tnid == 255 ) {
    short *outdata = (short*) malloc( sizeof(short) * numRead );
    for ( int i = 0; i < numRead; i++ ) {
      outdata[i] = ntohs( buf[i] );
    }
    lua_pushlstring ( L, (char*)outdata, sizeof(short) * numRead );
    free( outdata );
  } else {
    lua_createtable ( L, numRead, 0 );
    for ( int i = 0; i < numRead; i++ ) {
      short sval = ntohs( buf[i] );
      switch ( tnid ) {
        case 0:
          lua_pushnumber ( L, (double)( sval / 32768.0 ) );
          break;
        case 8:
          lua_pushinteger ( L, (char)( sval / 256 ) );
          break;
        case 16:
          lua_pushinteger ( L, sval );
          break;
      }
      lua_rawseti ( L, -2, i + 1 );
    }
  }
  self->mBufferPosition += numRead;
  env->ReleaseByteArrayElements ( readData, buf, 0 );
  return 1;

  USLog::Print ( "Done _read" );
  /* If no data read in loop, return nil */
  lua_pushnil ( L );
  return 1;
}

int MOAIAudioSamplerAndroid::_stop ( lua_State* L ) {
  USLog::Print ( "Entering _stop" );
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
  USLog::Print ( "Done _stop" );
  return 0;
}

int MOAIAudioSamplerAndroid::_pause ( lua_State* L ) {
  USLog::Print ( "Entering _pause" );
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  self->isActive = false;
  USLog::Print ( "Done _pause" );
  return 0;
}

int MOAIAudioSamplerAndroid::_resume ( lua_State* L ) {
  USLog::Print ( "Entering _resume" );
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  self->isActive = true;
  USLog::Print ( "Done _resume" );
  return 0;
}

int MOAIAudioSamplerAndroid::_flush ( lua_State* L ) {
  USLog::Print ( "Entering _flush" );
  MOAI_LUA_SETUP ( MOAIAudioSamplerAndroid, "U" )
    ;
  /* TODO: Flush the buffer of read samples */
  USLog::Print ( "Done _flush" );
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
