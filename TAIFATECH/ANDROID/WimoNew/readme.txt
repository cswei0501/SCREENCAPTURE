the two version:
1, release version in my signed;
2, debug version apply for platform signed, 
   the three notices:
   a, the MAKEFILE in the JNI with LOCAL_CERTIFICATE := platform ;                 
   b, the android:sharedUserId="android.uid.system" in the Manifest.xml;
   c, must set the mUnsignedCustomer is "false" in the MainActivity.java;
   