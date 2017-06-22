package com.tencent.parallelcomputedemo;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.SystemClock;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.util.Log;
import android.view.View;

import java.util.Arrays;
import java.util.Random;

import static android.content.ContentValues.TAG;

public class EntryActivity extends Activity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private RenderScript mRS;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_entry);

        // Example of a call to a native method
        findViewById(R.id.tnf).setOnClickListener(this);
        findViewById(R.id.rs).setOnClickListener(this);

        mRS = RenderScript.create(this);
    }

    public static void getRandom(long seed, int max, int factor, int offset, int array[]) {
        Random r = new Random(seed);
        for (int i = 0; i < array.length; ++i) {
            array[i] = (r.nextInt(max) * factor + offset);
        }
    }

    int lookup(int color, int[] table) {
        int distance = 255;
        int match = -1;
//        for (int n = 0; n < Math.pow(2, 16); ++n)
        for (int i = 0; i < 256; ++i) {
            int c = table[i];
            int d = Math.abs(Color.red(color) - Color.red(c))
                    + Math.abs(Color.green(color) - Color.green(c))
                    + Math.abs(Color.blue(color) - Color.blue(c))
                    + Math.abs(Color.alpha(color) - Color.alpha(c));
            d /= 4;
            if (d < distance) {
                distance = d;
                match = i;
                if (0 == d) break;
            }
        }
        return match;
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tnf:
//                startActivity(new Intent(this, OpenGLES30Activity.class));

                ScriptC_LookupColorTable lct = new ScriptC_LookupColorTable(mRS);
                Allocation table = Allocation.createSized(mRS, Element.RGBA_8888(mRS), 256);
                int[] colors = new int[256];
                getRandom(SystemClock.uptimeMillis(), 255, 1, 0, colors);
                table.copyFromUnchecked(colors);
                lct.set_colorTable(table);

                long start = SystemClock.uptimeMillis();
                Allocation inputAlloc = Allocation.createSized(mRS, Element.U32(mRS), 1);
                Allocation outputAlloc = Allocation.createSized(mRS, Element.I32(mRS), 1);
                int[] target = {Color.argb(20, 10, 127, 129)};
                inputAlloc.copyFrom(target);
                lct.forEach_lookup(inputAlloc, outputAlloc);
                int[] output = new int[1];
                outputAlloc.copyTo(output);
                mRS.finish();
                Log.i(TAG, "RS Lookup result = " + Arrays.toString(output) + ", cost=" + (SystemClock.uptimeMillis() - start));

                start = SystemClock.uptimeMillis();
                int ret = lookup(target[0], colors);
                Log.i(TAG, "CPU Lookup result = " + Arrays.toString(output) + ", cost=" + (SystemClock.uptimeMillis() - start));
                break;
            case R.id.rs:
                startActivity(new Intent(this, RenderScriptActivity.class));
                break;
        }
    }
}
