/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.tencent.parallelcomputedemo.TransformFeedback;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;
import android.util.Log;

import static android.opengl.GLES20.glGetString;

/**
 * Provides drawing instructions for a GLSurfaceView object. This class
 * must override the OpenGL ES drawing lifecycle methods:
 * <ul>
 *   <li>{@link GLSurfaceView.Renderer#onSurfaceCreated}</li>
 *   <li>{@link GLSurfaceView.Renderer#onDrawFrame}</li>
 *   <li>{@link GLSurfaceView.Renderer#onSurfaceChanged}</li>
 * </ul>
 */
public class MyGLRenderer implements GLSurfaceView.Renderer {

    private static final String TAG = "MyGLRenderer";
    private boolean mIsComputed = false;
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is do compute with OpenGL ES transform feedback
     */
    public native void computing();
    public native void setup();

    @Override
    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        // Create a minimum supported OpenGL ES context, then check:
        String version = glGetString(
                GL10.GL_VERSION);
        Log.w(TAG, "GLES Version: " + version );
        setup();
    }

    @Override
    public void onDrawFrame(GL10 unused) {
        if (!mIsComputed) {
            mIsComputed = true;
            Log.i(TAG, "Do computing in Renderer.onDrawFrame()");
            computing();
        }
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height) {

    }

}