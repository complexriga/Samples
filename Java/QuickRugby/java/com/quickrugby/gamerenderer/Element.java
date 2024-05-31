package com.quickrugby.gamerenderer;

import android.opengl.Matrix;

/**
 * Created by Andres Giraldo on 03.06.2016.
 */

public class Element {
    private String TAG;
    private boolean bgActive;
    private boolean bgReady;
    private boolean bgDisplay;
    private int igTypeId;
    private int igProgramInx;
    private float fgLineWidth;
    private float fgAlpha;
    private Texture ogTexture;
    private float[] agRotationMatrix;
    private float[] agTranslationMatrix;
    private float[] agVertexBuffer;
    private int[] agIndexBuffer;
    private float[] agTexelBuffer;
    private float[] agColor;

    public Element() {
        bgActive = true;
        bgReady = false;
        bgDisplay = true;
        TAG = "";
        agRotationMatrix = new float[16];
        Matrix.setIdentityM(agRotationMatrix, 0);
        agTranslationMatrix = new float[16];
        Matrix.setIdentityM(agTranslationMatrix, 0);
        fgLineWidth = 1f;
        fgAlpha = 1f;
    }

    public void rotate(float fpAngle) {
        bgReady = false;
        Matrix.setIdentityM(agRotationMatrix, 0);
        Matrix.rotateM(agRotationMatrix, 0, fpAngle, 0, 0, 1);
    }

    public void translate(float fpX, float fpY, float fpZ) {
        bgReady = false;
        Matrix.setIdentityM(agTranslationMatrix, 0);
        Matrix.translateM(agTranslationMatrix, 0, fpX, fpY, fpZ);
    }

    public void ready() {
        bgReady = true;
    }

    public void inactive() {
        bgActive = false;
    }

    public void show() {
        bgDisplay = true;
    }

    public void hide() {
        bgDisplay = false;
    }

    public void setTAG(String TAG) {
        this.TAG = TAG;
    }

    public void setLineWidth(float fpLineWidth) {
        this.fgLineWidth = fpLineWidth;
    }

    public void setAlpha(float fpAlpha) { this.fgAlpha = fpAlpha; }

    public void setTexture(Texture opTexture) {
        bgReady = false;
        this.ogTexture = opTexture;
    }

    public void setVertex(float[] apVertex) {
        bgReady = false;
        this.agVertexBuffer = apVertex;
    }

    public void setIndex(int[] apIndex) {
        bgReady = false;
        this.agIndexBuffer = apIndex;
    }

    public void setColor(float[] apColor) {
        bgReady = false;
        this.agColor = apColor;
    }

    public void setTextel(float[] apTexCoord) {
        bgReady = false;
        this.agTexelBuffer = apTexCoord;
    }

    public void setProgramInx(int ipProgramInx) {
        bgReady = false;
        this.igProgramInx = ipProgramInx;
    }

    public void setTypeId(int ipTypeId) {
        bgReady = false;
        this.igTypeId = ipTypeId;
    }

    public boolean isActive() {
        return bgActive;
    }

    public boolean isReady() {
        return bgReady;
    }

    public boolean isDisplayed() {
        return bgDisplay;
    }

    public int getTypeId() {
        return igTypeId;
    }

    public int getProgramInx() {
        return igProgramInx;
    }

    public String getTAG() {
        return TAG;
    }

    public float getLineWidth() {
        return fgLineWidth;
    }

    public float getAlpha() { return fgAlpha; }

    public Texture getTexture() {
        return ogTexture;
    }

    public float[] getVertex() {
        bgReady = false;
        return agVertexBuffer;
    }

    public int[] getIndex() {
        bgReady = false;
        return agIndexBuffer;
    }

    public float[] getColor() {
        bgReady = false;
        return agColor;
    }

    public float[] getTexel() {
        bgReady = false;
        return agTexelBuffer;
    }

    public float[] getRotationMatrix() {
        bgReady = false;
        return agRotationMatrix;
    }

    public float[] getTranslationMatix() {
        bgReady = false;
        return agTranslationMatrix;
    }

    float getDotProduct(float fpXA, float fpYA, float fpXB, float fpYB) {
        return (fpXA * fpXB) + (fpYA * fpYB);
    }

    public boolean isWithin(float fpX, float fpY) {
        float flXA, flYA;
        float flXB, flYB;
        float flXC, flYC;
        float flDotAA, flDotAB, flDotAC, flDotBB, flDotBC;
        float flInv;
        float flU, flV;
        int ilInxAX, ilInxAY, ilInxBX, ilInxBY, ilInxCX, ilInxCY;

        for(int i = 0; i < agIndexBuffer.length ; i += 3) {
            ilInxAX = agIndexBuffer[i] * 3;
            ilInxAY = ilInxAX + 1;
            ilInxBX = agIndexBuffer[i + 1] * 3;
            ilInxBY = ilInxBX + 1;
            ilInxCX = agIndexBuffer[i + 2] * 3;
            ilInxCY = ilInxCX + 1;

            flXA = agVertexBuffer[ilInxBX] - agVertexBuffer[ilInxAX];
            flYA = agVertexBuffer[ilInxBY] - agVertexBuffer[ilInxAY];
            flXB = agVertexBuffer[ilInxCX] - agVertexBuffer[ilInxAX];
            flYB = agVertexBuffer[ilInxCY] - agVertexBuffer[ilInxAY];
            flXC = fpX - agVertexBuffer[ilInxAX];
            flYC = fpY - agVertexBuffer[ilInxAY];

            flDotAA = getDotProduct(flXA, flYA, flXA, flYA);
            flDotAB = getDotProduct(flXA, flYA, flXB, flYB);
            flDotAC = getDotProduct(flXA, flYA, flXC, flYC);
            flDotBB = getDotProduct(flXB, flYB, flXB, flYB);
            flDotBC = getDotProduct(flXB, flYB, flXC, flYC);

            flInv = 1 / ((flDotAA * flDotBB) - (flDotAB * flDotAB));

            flU = ((flDotBB * flDotAC) - (flDotAB * flDotBC)) * flInv;
            flV = ((flDotAA * flDotBC) - (flDotAB * flDotAC)) * flInv;

            if((flU >= 0) && (flV >= 0) && (flU + flV < 1f))
                return true;
        }

        return false;
    }
}
