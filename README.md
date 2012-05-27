Gyroscope-Air-Native-Extension-for-Blackberry
=============================================

This is an Adobe Air Native Extension to access the gyroscope on the BlackBerry Playbook from an AIR application. This is meant to extend the Adobe Gyroscope ANE to support the BB Playbook:
http://www.adobe.com/devnet/air/native-extensions-for-air/extensions/gyroscope.html

You will have to package the ANE as described in the article above. You will have to add this into the extension.xml file:
  <platform name="QNX-ARM">
  	<applicationDeployment>
			<nativeLibrary>libGyroscope-arm.so</nativeLibrary>
			<initializer>ExtensionInitializer</initializer>
			<finalizer>ExtensionFinalizer</finalizer>
		</applicationDeployment>
	</platform>
  
