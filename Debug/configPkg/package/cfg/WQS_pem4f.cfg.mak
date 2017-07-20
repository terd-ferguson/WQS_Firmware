# invoke SourceDir generated makefile for WQS.pem4f
WQS.pem4f: .libraries,WQS.pem4f
.libraries,WQS.pem4f: package/cfg/WQS_pem4f.xdl
	$(MAKE) -f C:\Users\joseph\workspace_v7\WaterQualitySensor_EK_TM4C1294XL_updated/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\joseph\workspace_v7\WaterQualitySensor_EK_TM4C1294XL_updated/src/makefile.libs clean

