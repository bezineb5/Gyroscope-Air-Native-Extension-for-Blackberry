Gyroscope-Air-Native-Extension-for-Blackberry
=============================================

This is an Adobe Air Native Extension to access the gyroscope on the BlackBerry Playbook from an AIR application. This is meant to extend the Adobe Gyroscope ANE to support the BB Playbook:<br/>
http://www.adobe.com/devnet/air/native-extensions-for-air/extensions/gyroscope.html

Usage:<br/>
Get the compiled ANE from the Gyroscope/ANE directory.<br/>
The Gyroscope.ane file is compatible with BlackBerry Playbook, iOS and Android.<br/>
Use it as described in the Adobe link above.<br/>

To compile:<br/>
Get the source code and compile it with the BlackBerry Native SDK 2.0.0+<br/>
Get the Adobe files and add this into the extension.xml file:<br/>
&lt;platform name="QNX-ARM"&gt;<br/>
  	&lt;applicationDeployment&gt;<br/>
		&lt;nativeLibrary&gt;libGyroscope-arm.so&lt;/nativeLibrary&gt;<br/>
		&lt;initializer&gt;ExtensionInitializer&lt;/initializer&gt;<br/>
		&lt;finalizer&gt;ExtensionFinalizer&lt;/finalizer&gt;<br/>
	&lt;/applicationDeployment&gt;<br/>
&lt;/platform&gt;<br/>
  
You will have to package the ANE as described in the article above.
