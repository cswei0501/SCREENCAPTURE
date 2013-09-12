package cidana.screencap;

/**
 * MediaPlayer class can be used to control playback of audio/video files and
 * streams. An example on how to use the methods in this class can be found in
 * {@link android.widget.VideoView}. Please see <a href="{@docRoot}
 * guide/topics/media/index.html">Audio and Video</a> for additional help using
 * MediaPlayer.
 * 
 */

public class Capture {
	static {
		System.loadLibrary("screencapture");
	}

	public native int initSC();

	public native int setVideoEnable(boolean bEnable);

	public native int setDisplayMode(int nMode);

	public native int setVideoRenderMode(int nMode);

	public native int setVideoRotate(boolean bRotate);
}
