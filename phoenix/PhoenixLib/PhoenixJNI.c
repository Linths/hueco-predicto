#include "stdafx.h"
#include <jni.h>
#include <string.h>
#include "PhoenixJNI.h"
#include "phoenix.h"
#include "dm.h"

/* structure for configuration parameters */
Config	cnfg;

extern char* response;
extern aFrameDef  *ontology;

/* forward function declarations */
int process_input( char *line, char *response, Config *cnfg );
char *readCommand();
void writeCommand( char *cmd );
int init(Config *cnfg);
char**  get_frame_prompts(aFrameDef* frame, int* num_prompts_tot, Config* cnfg);

JNIEXPORT jboolean JNICALL Java_blt_vht_Phoenix_setContentPathJNI(JNIEnv *env, jobject jobj, jstring path)
{
	jboolean isCopy;
	strcpy(cnfg.task_dir, (*env)->GetStringUTFChars(env, path, &isCopy));
	return isCopy;
}

JNIEXPORT jboolean JNICALL Java_blt_vht_Phoenix_initJNI(JNIEnv *env, jobject jobj)
{
	if(init(&cnfg))
		return 0;

	return 1;
}

JNIEXPORT jstring JNICALL Java_blt_vht_Phoenix_readCommandJNI(JNIEnv *env, jobject jobj)
{
	return (*env)->NewStringUTF(env, readCommand());
}

JNIEXPORT void JNICALL Java_blt_vht_Phoenix_writeCommandJNI(JNIEnv *env, jobject jobj, jstring command)
{
	char *cCommand = (char *)(*env)->GetStringUTFChars(env, command, 0);
	writeCommand(cCommand);
}

JNIEXPORT jstring JNICALL Java_blt_vht_Phoenix_processCommandJNI(JNIEnv *env, jobject jobj, jstring command)
{
	char *cCommand = (char *)(*env)->GetStringUTFChars(env, command, 0);
	process_input(cCommand, response, &cnfg);
	return (*env)->NewStringUTF(env, response);
}


JNIEXPORT jobjectArray JNICALL Java_blt_vht_Phoenix_getFramesJNI(JNIEnv *env, jobject jobj) 
{
    jobjectArray    frames;
    jclass          frameClass;
    jmethodID       methodId;
    jfieldID        fieldId;
    jobject         obj;
    jobjectArray    actions;
    int i;
    int j;
    int num_prompts;

    char**         framePrompts;
    aFrameDef*     frame;

    // Create an array of Java Frame objects
    frames = (jobjectArray)(*env)->NewObjectArray(env, cnfg.task_frames, 
                                               (*env)->FindClass(env, "blt/vht/PhoenixFrame"), 
                                               NULL);
    
    // Iterate over frames in the global task file
    for (i = 0; i < cnfg.task_frames; i++) {
    
        frame = &ontology[i];
        framePrompts = get_frame_prompts(frame, &num_prompts, &cnfg);
        
        // An array of actions for this frame
        actions = (jobjectArray)(*env)->NewObjectArray(env, num_prompts,
                                              (*env)->FindClass(env, "java/lang/String"),
                                              (*env)->NewStringUTF(env, ""));
    
        // Convert array of C-strings to Java array of strings - ( Populate actions )
        for (j = 0; j < num_prompts; j++) {
            (*env)->SetObjectArrayElement(env, actions, j,
                                          (*env)->NewStringUTF(env, framePrompts[j]));
        }

        // Find the PhoenixFrame class
        frameClass = (*env)->FindClass(env, "blt/vht/PhoenixFrame");
        if (frameClass == 0) { printf("class not found"); }

        // Get the constructor for the PhoenixFrame class
        methodId = (*env)->GetMethodID(env, frameClass, 
                                       "<init>", 
                                       "(Ljava/lang/String;Ljava/lang/String;)V");
        if (methodId == 0) { printf("method not found"); }

        // Create a new PhoenixFrame object
        obj = (*env)->NewObject(env, frameClass, methodId, 
                                (*env)->NewStringUTF(env, frame->name), 
                                (*env)->NewStringUTF(env, frame->description));
        if (obj == 0) { printf("couldn't build the object"); }

        // Get the actions field of the PhoenixFrame object
        fieldId = (*env)->GetFieldID(env, frameClass, "actions",  "[Ljava/lang/String;");
        if (fieldId == 0) { printf("couldn't get field id"); }
   
        // Set the actions field to our actions array
        (*env)->SetObjectField(env, obj, fieldId, actions);

        // Add the newly created object to the frames array 
        (*env)->SetObjectArrayElement(env, frames, i, obj);
    }

    return frames;
}

