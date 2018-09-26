package com.seth.simple_game; 
public class NativeLoader extends android.app.NativeActivity { 
    static {
        System.loadLibrary("simple_game"); 
    } 
} 
