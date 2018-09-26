package com.seth.project; 
public class NativeLoader extends android.app.NativeActivity { 
    static {
        System.loadLibrary("project"); 
    } 
} 
