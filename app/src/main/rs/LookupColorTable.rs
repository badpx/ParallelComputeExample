#pragma version(1)

#pragma rs java_package_name(com.tencent.parallelcomputedemo)
#pragma rs_fp_relaxed

rs_allocation colorTable;

void setColor(int index, int red, int green, int blue, int alpha) {
    rsSetElementAt_uchar4(colorTable, (uchar4){red, green, blue, alpha}, index);
}

//int32_t __attribute__((kernel)) lookup(uchar4 in, uint32_t x) {
void lookup(const uint32_t* v_in, int32_t* v_out, uint32_t x) {
    float distance = 255 * 255;
    int32_t match = -1;
    uchar4* in = (uchar4*)v_in;
    for (int i = 0; i < 256; ++i) {
        uchar4 c = rsGetElementAt_uchar4(colorTable, i);
        int d = abs(in->r - c.r) + abs(in->g - c.g) + abs(in->b - c.b) + abs(in->a - c.a);
        d /= 4;
        float dis = (pown((float)in->r - d, 2) + pown((float)in->g - d, 2) + pown((float)in->b - d, 2) + pown((float)in->a - d, 2)) / 4;
        if (dis < distance) {
            distance = dis;
            match = i;
            if (0 == d) break;
        }
    }
    //return match;
    *v_out = match;
}
