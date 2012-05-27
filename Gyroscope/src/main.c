/*
 * Copyright (c) 2011 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "FlashRuntimeExtensions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/platform.h>
#include <stdbool.h>
#include "bps/bps.h"
#include "bps/event.h"
#include "bps/sensor.h"

#include <pthread.h>

// Variable declarations
pthread_t eventThread = NULL;
bool shutdown = false;

// Function declarations
void ExtensionInitializer(void** extDataToSet,
		FREContextInitializer* ctxInitializerToSet,
		FREContextFinalizer* ctxFinalizerToSet);
void ExtensionFinalizer();

void ContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx,
		uint32_t* numFunctionsToSet, const FRENamedFunction** functionsToSet);
void ContextFinalizer(FREContext ctx);

void* eventLoop(void* data);

FREObject init(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]);

FREObject gyroscopeStart(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]);

FREObject gyroscopeStop(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]);

FREObject gyroscopeSupport(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]);


// Function implementations
FREObject init(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]) {

	return NULL;
}

FREObject gyroscopeSupport(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]) {

	bool isSupported = sensor_is_supported(SENSOR_TYPE_GYROSCOPE);

	FREObject result;
	FRENewObjectFromBool(isSupported, &result);
	return result;
}

FREObject gyroscopeStart(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]) {

	bool isSuccess = false;

	// Set the maximum rate of delivery
	sensor_info_t *sensorInfo = NULL;

	if (sensor_info(SENSOR_TYPE_GYROSCOPE, &sensorInfo)) {
		unsigned int minimumDelay = sensor_info_get_delay_mininum(sensorInfo);
		sensor_set_rate(SENSOR_TYPE_GYROSCOPE, minimumDelay);

		sensor_info_destroy(sensorInfo);
	}

	// Starts the event loop thread
	shutdown = false;
	pthread_create(&eventThread, NULL, eventLoop, ctx);
	isSuccess = true;

	FREObject result;
	FRENewObjectFromBool(isSuccess, &result);
	return result;
}

FREObject gyroscopeStop(FREContext ctx, void* functionData, uint32_t argc,
		FREObject argv[]) {

	bool isSuccess = false;

	if (eventThread != NULL) {
		shutdown = true;
		eventThread = NULL;
	}

	FREObject result;
	FRENewObjectFromBool(isSuccess, &result);
	return result;
}

void* eventLoop(void* data) {
	FREContext ctx = (FREContext) data;
	float x, y, z;
	bps_event_t *event = NULL;
	char buffer[256];

    // Initialize BPS
    bps_initialize();

	// Start the gyroscope
	if (sensor_request_events(SENSOR_TYPE_GYROSCOPE) != BPS_SUCCESS) {
		shutdown = true;
	}

	// Receive events through the event channel
	while (!shutdown) {
		event = NULL;

		if (bps_get_event(&event, 50) != BPS_SUCCESS)
			return NULL;

		if (event) {
			if (bps_event_get_domain(event) == sensor_get_domain()) {
				if (bps_event_get_code(event) == SENSOR_GYROSCOPE_READING) {
					if (sensor_event_get_xyz(event, &x, &y, &z) == BPS_SUCCESS) {
						sprintf(buffer, "%f&%f&%f", x, y, z);
						fprintf(stdout, "Sensor event: %f&%f&%f\n", x, y, z);
					    fflush(stdout);

						if(ctx != NULL) {
							FREDispatchStatusEventAsync(ctx, (uint8_t*)"CHANGE", (uint8_t*)buffer);
						}
					}
				}
			}
		}
	}

	if (sensor_stop_events(SENSOR_TYPE_GYROSCOPE) != BPS_SUCCESS) {
		fprintf(stdout, "Unable to stop sensor\n");
	    fflush(stdout);
	}

	// Stop BPS
	bps_shutdown();

	return NULL;
}

/**
 * The runtime calls this method once when it loads an ActionScript extension.
 * Implement this function to do any initializations that your extension requires.
 * Then set the output parameters.
 *
 * @param extDataToSet
 *             A pointer to a pointer to the extension data of the ActionScript extension.
 *             Create a data structure to hold extension-specific data. For example, allocate
 *             the data from the heap, or provide global data. Set extDataToSet to a pointer
 *             to the allocated data.
 * @param ctxInitializerToSet
 *             A pointer to the pointer to the FREContextInitializer() function. Set
 *             ctxInitializerToSet to the FREContextInitializer() function you defined.
 * @param ctxFinalizerToSet
 *             A pointer to the pointer to the FREContextFinalizer() function. Set
 *             ctxFinalizerToSet to the FREContextFinalizer() function you defined. You can
 *             set this pointer to NULL.
 */
void ExtensionInitializer(void** extDataToSet,
		FREContextInitializer* ctxInitializerToSet,
		FREContextFinalizer* ctxFinalizerToSet) {
	fprintf(stdout, "ExtensionInitializer\n");
    fflush(stdout);

    *extDataToSet = NULL;
	*ctxInitializerToSet = &ContextInitializer;
	*ctxFinalizerToSet = &ContextFinalizer;
}

/**
 * The runtime calls this function when it disposes of the ExtensionContext instance
 * for this extension context.
 */
void ExtensionFinalizer() {
}

/**
 * The runtime calls this method when the ActionScript side calls ExtensionContext.createExtensionContext().
 *
 * @param extData
 *             A pointer to the extension data of the ActionScript extension.
 * @param ctxType
 *             A string identifying the type of the context. You define this string as
 *             required by your extension. The context type can indicate any agreed-to meaning
 *             between the ActionScript side and native side of the extension. If your extension
 *             has no use for context types, this value can be Null. This value is a UTF-8
 *             encoded string, terminated with the null character.
 * @param ctx
 *             An FREContext variable. The runtime creates this value and passes it to FREContextInitializer().
 * @param numFunctionsToSet
 *             A pointer to a unint32_t. Set numFunctionsToSet to a unint32_t variable containing
 *             the number of functions in the functionsToSet parameter.
 * @param functionsToSet
 *             A pointer to an array of FRNamedFunction elements. Each element contains a pointer
 *             to a native function, and the string the ActionScript side uses in the ExtensionContext
 *             instance's call() method.
 */
void ContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx,
		uint32_t* numFunctionsToSet, const FRENamedFunction** functionsToSet) {
	char *temp = NULL;
	int i;

	fprintf(stdout, "ContextInitializer\n");
    fflush(stdout);

	// define an array of functions
	const char *functionNames[] = { "init", "gyroscopeStart", "gyroscopeStop", "gyroscopeSupport", NULL };
	FREFunction functionPtrs[] = { init, gyroscopeStart, gyroscopeStop, gyroscopeSupport, NULL };

	// count number of functions
	*numFunctionsToSet = 0;
	while (functionPtrs[*numFunctionsToSet] != NULL) {
		(*numFunctionsToSet)++;
	}

	FRENamedFunction *functionSet = calloc(*numFunctionsToSet,
			sizeof(FRENamedFunction));

	for (i = 0; i < *numFunctionsToSet; i++) {
		int bufferSize = sizeof(char) * (strlen(functionNames[i]) + 1);
		temp = (char*) malloc(bufferSize);
		strncpy(temp, functionNames[i], bufferSize);
		temp[strlen(functionNames[i])] = '\0';
		functionSet[i].name = (uint8_t*) temp;
		functionSet[i].functionData = NULL;
		functionSet[i].function = functionPtrs[i];
	}

	*functionsToSet = functionSet;
}

/**
 * The runtime calls this function when it disposes of the ExtensionContext instance for this extension context.
 *
 * @param ctx
 *             The FREContext variable that represents this extension context.
 */
void ContextFinalizer(FREContext ctx) {

	if (eventThread != NULL) {
		shutdown = true;
		pthread_cancel(eventThread);
		eventThread = NULL;
	}

}
