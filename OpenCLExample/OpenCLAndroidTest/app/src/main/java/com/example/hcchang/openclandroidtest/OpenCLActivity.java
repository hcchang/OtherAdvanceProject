package com.example.hcchang.openclandroidtest;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class OpenCLActivity extends AppCompatActivity {

    private Button reset;
    private Button opencl;
    private Button nativeC;
    private ImageView imgView;
    private TextView textView;

    Bitmap bmpOrig, bmpOpenCL, bmpNativeC;

    final int info[] = new int[3]; // Width, Height, Execution time (ms)

    private void copyFile(final String f) {
        InputStream in;
        try {
            in = getAssets().open(f);
            final File of = new File(getDir("execdir",MODE_PRIVATE), f);

            final OutputStream out = new FileOutputStream(of);

            final byte b[] = new byte[65535];
            int sz = 0;
            while ((sz = in.read(b)) > 0) {
                out.write(b, 0, sz);
            }
            in.close();
            out.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_open_cl);

        reset = (Button)findViewById(R.id.reset);
        opencl = (Button)findViewById(R.id.openclButton);
        nativeC = (Button)findViewById(R.id.nativeCButton);
        imgView = (ImageView) findViewById(R.id.imageView);
        textView = (TextView)findViewById(R.id.textView);

        copyFile("bilateralKernel.cl");

        bmpOrig = BitmapFactory.decodeResource(this.getResources(), R.drawable.rena);
        //((BitmapDrawable) getResources().getDrawable(R.drawable.rena)).getBitmap(); //rgb565
        imgView.setImageBitmap(bmpOrig);

        opencl.setOnClickListener(new ProcessClickListenerOCL());
        reset.setOnClickListener(new ProcessClickListenerReset());
        nativeC.setOnClickListener(new ProcessClickListenerNativeC());

        info[0] = bmpOrig.getWidth();
        info[1] = bmpOrig.getHeight();

        bmpOpenCL = Bitmap.createBitmap(info[0], info[1], Bitmap.Config.ARGB_8888);
        bmpNativeC = Bitmap.createBitmap(info[0], info[1], Bitmap.Config.ARGB_8888);

    }

    public class ProcessClickListenerOCL implements View.OnClickListener{
        //private
        @Override
        public void onClick(View v) {
            // TODO Auto-generated method stub
            OpenCLClass.runOpenCL(bmpOrig, bmpOpenCL, info);
            textView.setText("Bilateral Filter, OpenCL, Processing time is " + info[2] + " ms");
            imgView.setImageBitmap(bmpOpenCL);
        }
    }

    public class ProcessClickListenerReset implements View.OnClickListener{
        //private
        @Override
        public void onClick(View v) {
            // TODO Auto-generated method stub
            imgView.setImageBitmap(bmpOrig);
        }
    }

    public class ProcessClickListenerNativeC implements View.OnClickListener{
        //private
        @Override
        public void onClick(View v) {
            // TODO Auto-generated method stub
            OpenCLClass.runNativeC(bmpOrig, bmpNativeC, info);
            textView.setText("Bilateral Filter, NativeC, Processing time is " + info[2] + " ms");
            imgView.setImageBitmap(bmpNativeC);
        }
    }



}
