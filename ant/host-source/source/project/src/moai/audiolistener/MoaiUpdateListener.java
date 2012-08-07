package com.ziplinegames.moai;

import android.media.AudioRecord.OnRecordPositionUpdateListener;

public class MOAIUpdateListener {

  protected static native void AKUAudioListenerPeriodicNotification ( );
  protected static native void AKUAudioListenerMarkerReached ( );

  public static OnRecordPositionUpdateListener createListener ( ) {

    return new OnRecordPositionUpdateListener() {
      @Override
      public void onPeriodicNotification(AudioRecord recorder) {
        AKUAudioListenerPeriodicNotification();
      }

      @Override
      public void onMarkerReached(AudioRecord recorder) {
        AKUAudioListenerMarkerReached();
      }
    }
  }
}
