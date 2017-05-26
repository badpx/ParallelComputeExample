package com.tencent.parallelcomputedemo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.tencent.parallelcomputedemo.TransformFeedback.OpenGLES30Activity;

public class EntryActivity extends Activity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_entry);

        // Example of a call to a native method
        findViewById(R.id.tnf).setOnClickListener(this);
        findViewById(R.id.rs).setOnClickListener(this);

    }


    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.tnf:
                startActivity(new Intent(this, OpenGLES30Activity.class));
                break;
            case R.id.rs:
                startActivity(new Intent(this, RenderScriptActivity.class));
                break;
        }
    }
}
