/*
 * Class:     Phoenix
 * Method:    setContentPath
 * Sets  the content path, where tasks, grammars, frames reside.
 * NOTE: this method must be called before init() to have any affect
 */
JNIEXPORT jboolean JNICALL Java_blt_vht_Phoenix_setContentPathJNI(JNIEnv *env, jobject jobj, jstring path);

/*
 * Class:     Phoenix
 * Method:    init
 * This method initializes the Phoenix app - loads config and the content files
 */
JNIEXPORT jboolean JNICALL Java_blt_vht_Phoenix_initJNI(JNIEnv *env, jobject jobj);

/*
 * Class:     Phoenix
 * Method:    readCommand
 * This method reads a command from Phoenix
 */
JNIEXPORT jstring JNICALL Java_blt_vht_Phoenix_readCommandJNI(JNIEnv *env, jobject jobj);

/*
 * Class:     Phoenix
 * Method:    writeCommand
 * This method writes a command to Phoenix
 */
JNIEXPORT void JNICALL Java_blt_vht_Phoenix_writeCommandJNI(JNIEnv *env, jobject jobj, jstring command);

/*
 * Class:     Phoenix
 * Method:    processCommand
 * This method processes a command string and gives back a string command
 */
JNIEXPORT jstring JNICALL Java_blt_vht_Phoenix_processCommandJNI(JNIEnv *env, jobject jobj, jstring command);

/*
 * Class:     Phoenix
 * Method:    getFrames
 * This method mines the data from the global task file ontology and returns an array of 
 * flattened Java Frame objects that contain only the name, description, and array of actions (prompts)
 * 
 */
JNIEXPORT jobjectArray JNICALL Java_blt_vht_Phoenix_getFramesJNI(JNIEnv *env, jobject jobj); 