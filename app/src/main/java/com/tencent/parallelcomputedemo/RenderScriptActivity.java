package com.tencent.parallelcomputedemo;

import android.app.Activity;
import android.os.Bundle;
import android.os.Looper;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.util.Log;

import java.util.Arrays;
import java.util.logging.Handler;

/**
 * Created by dk on 17-5-26.
 */

public class RenderScriptActivity extends Activity {
    private static final String TAG = "RenderScriptActivity";

    private RenderScript mRS;
    private static final int MIN_POW = 17;
    private static final int MAX_POW = 25;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mRS = RenderScript.create(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Timer timer = new Timer();
        timer.start();
        doTest();
        Log.d(TAG, "Do test cost = " + timer.duration());
    }

    private void doTest() {
        Timer timer = new Timer();
        Log.i(TAG, "pow, upload, computing, getResult, gpuTotalCost, cpuTotalCost, gpuNormalized, cpuNormalized");
        for (int p = MIN_POW; p <= MAX_POW; ++p) {
            final int inputElementNum = (int) Math.pow(2, p);
            float[] inputArray = null;
            float[] gpuOutputArray = null;
            float[] cpuOutputArray = null;

            try {
                inputArray = new float[inputElementNum];
                gpuOutputArray = new float[inputElementNum];
                cpuOutputArray = new float[inputElementNum];
            } catch (Throwable throwable) {
                throwable.printStackTrace();
                break;
            }

            // 填充输入数据
            for (int i = 0; i < inputElementNum; ++i) {
                inputArray[i] = i;
            }

            timer.start();
            /* ======================================= CPU ======================================*/
//            simpleComputingWithCPU(inputArray, cpuOutputArray);
            iterationComputingWithCPU(inputArray, cpuOutputArray);
            long cpuCost = timer.reset();

            /* ================================== RenderScript ===================================*/
            // 创建输入输出 Allocation，使用 4 维向量并行化
            Allocation outputAllocation =
                    Allocation.createSized(mRS, Element.F32_4(mRS), gpuOutputArray.length / 4);
            Allocation inputAllocation =
                    Allocation.createSized(mRS, Element.F32_4(mRS), inputArray.length / 4/*输入输出都是4元向量，长度要除以4*/);
            inputAllocation.copyFrom(inputArray);   // 将输入数组拷贝到输入 Allocation
            long uploadCost = timer.deltaMetering();

            // 执行计算 kernel
//            ScriptC_calc myScript = new ScriptC_calc(mRS);
            ScriptC_calc_fs myScript = new ScriptC_calc_fs(mRS);
            myScript.forEach_iterationFunc(inputAllocation, outputAllocation);
            mRS.finish();   // 等待执行完成，以准确统计耗时
            long computingCost = timer.deltaMetering();

            // 从 outputAllocation 拷贝计算结果
            outputAllocation.copyTo(gpuOutputArray);
            mRS.finish();   // 等待执行完成，以准确统计耗时
            long getResultCost = timer.deltaMetering();

            long gpuCost = timer.duration();
            Log.i(TAG, String.format("2^%d, %d, %d, %d, %d, %d, %f, 1.0",
                    p, uploadCost, computingCost, getResultCost, gpuCost, cpuCost, (float)cpuCost / (float)gpuCost));

            dataVerify(cpuOutputArray, gpuOutputArray);
        }
    }

    private void simpleComputingWithCPU(float[] inputArray, float[] cpuOutputArray) {
        if (null == inputArray || null == cpuOutputArray || inputArray.length != cpuOutputArray.length) {
            return;
        }
        int inputElementNum = inputArray.length;
        for (int n = 0; n < inputElementNum; ++n) {
            // Linear Func:
//                cpuOutputArray[n] = inputArray[n] * 3.14159f + 0.5f;

            // Exponential function test:
            float inValue = inputArray[n];
            cpuOutputArray[n] = (float) (2.0f * Math.exp(-3.0f * inValue)
                    * (Math.sin(0.4f * inValue) + Math.cos(-1.7f * inValue)) + 3.7f);
        }
    }

    void iterationComputingWithCPU(float[] inBuffer, float[] outBuffer) {
        if (null == inBuffer || null == outBuffer || inBuffer.length != outBuffer.length) {
            return;
        }

        int count = inBuffer.length;
        for (int i = 0; i < count; i += 2) {
            float vx = inBuffer[i];
            float vy = inBuffer[i+1];
            float x = 540.0f;
            float y = 960.0f;

            final float dt = 0.1f;

            for (int c = 0; c < 100; c++) {

                float l = (float) (1.0f/Math.sqrt(x*x + y*y));
                float ax = -x*l*l*l;
                float ay = -y*l*l*l;

                x += vx*dt;
                y += vy*dt;

                vx += ax*dt;
                vy += ay*dt;
            }

            outBuffer[i] = x;
            outBuffer[i+1] = y;
        }
    }

    private void dataVerify(float[] bufferA, float[] bufferB) {
        if (null != bufferA && null != bufferB) {
            int size = bufferA.length;
            Log.d(TAG, String.format(":[%d] = %f, [%d] = %f, [%d] = %f, [%d] = %f, [%d] = %f",
                    0, (bufferA)[0],
                    (size) / 2 - (size) / 4, (bufferA)[(size) / 2 - (size) / 4],
                    (size) / 2, (bufferA)[(size) / 2],
                    (size) / 2 + (size) / 4, (bufferA)[(size) / 2 + (size) / 4],
                    (size) - 1, (bufferA)[(size) - 1]));

            size = bufferB.length;
            Log.d(TAG, String.format(":[%d] = %f, [%d] = %f, [%d] = %f, [%d] = %f, [%d] = %f",
                    0, (bufferB)[0],
                    (size) / 2 - (size) / 4, (bufferB)[(size) / 2 - (size) / 4],
                    (size) / 2, (bufferB)[(size) / 2],
                    (size) / 2 + (size) / 4, (bufferB)[(size) / 2 + (size) / 4],
                    (size) - 1, (bufferB)[(size) - 1]));
        }
    }
}
