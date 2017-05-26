// Needed directive for RS to work
#pragma version(1)

// The java_package_name directive needs to use your Activity's package path
#pragma rs java_package_name(com.tencent.parallelcomputedemo)

#pragma rs_fp_relaxed

float4 __attribute__((kernel)) linearFunc(float4 input, uint32_t x) {
    return input * 3.14159f + 0.5f;
}

float4 __attribute__((kernel)) exponentialFunc(float4 input, uint32_t x) {
    return (2.0f * exp(-3.0f * input) * (sin(0.4f * input) + cos(-1.7f * input)) + 3.7f);
}

static float2 accel(float2 p)
{
    float l = 1.0f /length(p);

    return -p * (l * l * l);
}


static float4 f(float4 x)
{
    float2 ExplosionPlace = (float2){540.0f, 960.0f};

    float4 p = ExplosionPlace.xyxy;
    float4 v = x;

    const float dt = 0.1f;

    for (int i = 0; i < 100; i++) {

        float2 a1 = accel((float2){p.x, p.y});
        float2 a2 = accel((float2){p.z, p.w});
        float4 a;
        a.xy = a1.xy;
        a.zw = a2.xy;

        p += v * dt;
        v += a * dt;
    }

    return p;
}

float4 __attribute__((kernel)) iterationFunc(float4 input, uint32_t x) {
    return f(input);
}
