package com.quickrugby.gamerenderer;

import android.opengl.GLES20;

/**
 * Created by Andres Giraldo on 03.06.2016.
 */

public class Texture {

    private String TAG;
    private int igId;
    private int igUnit;
    private int igType;
    private float fgUnitX;
    private float fgUnitY;

    public Texture(int ipUnit, int ipId, int ipType, int ipWidth, int ipHeight) {
        igUnit = ipUnit;
        igId = ipId;
        igType = ipType;
        TAG = "";

        fgUnitX = (float) 1 / ipWidth;
        fgUnitY = (float) 1 / ipHeight;
    }

    public boolean isActive() {
        return GLES20.glIsTexture(igId);
    }

    public void setTAG(String TAG) {
        this.TAG = TAG;
    }

    public int getId() {
        return igId;
    }

    public int getUnit() {
        return igUnit;
    }

    public int getType() {
        return igType;
    }

    public String getTAG() {
        return TAG;
    }

    public float getUnitX() {
        return fgUnitX;
    }

    public float getUnitY() {
        return fgUnitY;
    }

}
