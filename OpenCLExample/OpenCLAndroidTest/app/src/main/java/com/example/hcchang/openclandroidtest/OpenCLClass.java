package com.example.hcchang.openclandroidtest;

import android.graphics.Bitmap;

/**
 * Created by hcchang on 2018/2/24.
 */

public class OpenCLClass {
    static boolean sfoundLibrary = true;
    static {
        try {
            System.loadLibrary("openclexample1");
        }
        catch (UnsatisfiedLinkError e) {
            sfoundLibrary = false;
        }
    }
    public static native int runOpenCL(Bitmap bmpIn, Bitmap bmpOut, int info[]);
    public static native int runNativeC(Bitmap bmpIn, Bitmap bmpOut, int info[]);
}
