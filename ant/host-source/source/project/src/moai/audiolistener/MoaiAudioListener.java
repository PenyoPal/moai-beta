package com.ziplinegames.moai;

import android.media.AudioRecord;
import android.media.AudioRecord.OnRecordPositionUpdateListener;

public class MoaiAudioListener {

  protected static native void AKUAudioListenerPeriodicNotification ( );
  protected static native void AKUAudioListenerMarkerReached ( );

  public static OnRecordPositionUpdateListener createListener ( ) {

    MoaiLog.i ( "Java-side: Creating listener object" );

    return new OnRecordPositionUpdateListener() {
      @Override
      public void onPeriodicNotification(AudioRecord recorder) {
        MoaiLog.i ( "Sending periodic notification" );
        AKUAudioListenerPeriodicNotification();
      }

      @Override
      public void onMarkerReached(AudioRecord recorder) {
        MoaiLog.i ( "Sending marker reached notification" );
        AKUAudioListenerMarkerReached();
      }
    };
  }
}
