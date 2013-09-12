package com.cmcc.wimo;

public class WiMoInterface {
	/** for wimo1.0, wimo2.0, game 
	* The UI need to implements the function controlCallback()
	*/
	public interface WimoControlCallbackPort
	{
		/**callback message
		* nVal:
		*	public final int
		*		PMSG_WIMO_FAILED = 1,
		*		PMSG_WIMO_SUCCESS = 2,
		*		PMSG_WIMO_SUSPEND = 3;
		* If it is PMSG_WIMO_SUCCESS, WiMo has already connected with RX,
		* and start screen transmit;
		* If it is PMSG_WIMO_FAILED, WiMo connect failed or else reason;
		* If it is PMSG_WIMO_SUSPEND, oneself has already been kicked, and
		* another model or device will connect to RX;
		* nType: reserve
		*/
		public abstract int controlCallbackPort(int nVal, int nType);
	};
}