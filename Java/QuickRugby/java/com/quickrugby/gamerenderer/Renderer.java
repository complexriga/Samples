package com.quickrugby.gamerenderer;

import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.EGLConfig;
import android.opengl.GLES20;
import android.opengl.GLU;
import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.opengl.GLUtils;
import android.opengl.EGLContext;
import android.content.Context;
import android.view.WindowManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.io.InputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.FloatBuffer;
import java.nio.ByteOrder;

import javax.microedition.khronos.opengles.GL10;

/**
 * Created by Andres Giraldo on 28.05.2016.
 */

public class Renderer implements GLSurfaceView.Renderer {

    final String TAG = "Renderer";
    final int COORDS_PER_VERTEX = 3;
    final int COORDS_PER_TEXEL = 2;
    final int MAX_TEXTURES = 100;
    final int INTEGER_BYTES = 4;
    final int FLOAT_BYTES = 4;

    final int PROGRAMS_QUANTITY = 3;
    public final int PRG_SOLID_COLOR = 0;
    public final int PRG_SIMPLE_TEXTURE = 1;
    public final int PRG_ALPHA_TEXTURE = 2;

    final String sgColorVShaderCode =   "#version 300 es\n" +
                                        "uniform mat4 vPMatrix;\n" +
                                        "uniform mat4 vVMatrix;\n" +
                                        "uniform mat4 vRotation;\n" +
                                        "uniform mat4 vTranslation;\n" +
                                        "in vec4 vPosition;\n" +
                                        "in vec4 vColor;\n" +
                                        "out vec4 tColor;\n" +
                                        "\n" +
                                        "void main() {\n" +
                                        "		gl_Position = vPMatrix * vVMatrix * vTranslation * vRotation * vPosition;\n" +
                                        "       tColor = vColor;\n" +
                                        "}";

    final String sgColorFShaderCode =   "#version 300 es\n" +
                                        "precision mediump float;\n" +
                                        "in vec4 tColor;\n" +
                                        "out vec4 gl_FragColor;\n" +
                                        "\n" +
                                        "void main() {\n" +
                                        "	gl_FragColor = tColor;\n" +
                                        "}";

    final String sgTextureVShaderCode = "#version 300 es\n" +
                                        "uniform mat4 vPMatrix;\n" +
                                        "uniform mat4 vVMatrix;\n" +
                                        "uniform mat4 vRotation;\n" +
                                        "uniform mat4 vTranslation;\n" +
                                        "in vec4 vPosition;\n" +
                                        "in vec2 vTexPosition;\n" +
                                        "out vec2 aTexPosition;\n" +
                                        "\n" +
                                        "void main() {\n" +
                                        "		gl_Position = vPMatrix * vVMatrix * vTranslation * vRotation * vPosition;\n" +
                                        "		aTexPosition = vTexPosition;\n" +
                                        "}";

    final String sgTextureFShaderCode = "#version 300 es\n" +
                                        "precision mediump float;\n" +
                                        "uniform usampler2D sTexture;\n" +
                                        "in vec2 aTexPosition;\n" +
                                        "out vec4 gl_FragColor;\n" +
                                        "\n" +
                                        "void main() {\n" +
                                        "	gl_FragColor = vec4(texture(sTexture, aTexPosition));\n" +
                                        "}";

    final String sgAlphaTextureVShaderCode =
                                        "#version 300 es\n" +
                                        "uniform mat4 vPMatrix;\n" +
                                        "uniform mat4 vVMatrix;\n" +
                                        "uniform mat4 vRotation;\n" +
                                        "uniform mat4 vTranslation;\n" +
                                        "in vec4 vPosition;\n" +
                                        "in vec2 vTexPosition;\n" +
                                        "out vec2 aTexPosition;\n" +
                                        "in float vAlpha;\n" +
                                        "out float aAlpha;\n" +
                                        "\n" +
                                        "void main() {\n" +
                                        "		gl_Position = vPMatrix * vVMatrix * vTranslation * vRotation * vPosition;\n" +
                                        "		aTexPosition = vTexPosition;\n" +
                                        "       aAlpha = vAlpha;\n" +
                                        "}";

    final String sgAlphaTextureFShaderCode =
                                        "#version 300 es\n" +
                                        "precision mediump float;\n" +
                                        "uniform usampler2D sTexture;\n" +
                                        "in vec2 aTexPosition;\n" +
                                        "in float aAlpha;\n" +
                                        "out vec4 gl_FragColor;\n" +
                                        "\n" +
                                        "void main() {\n" +
                                        "	gl_FragColor = vec4(texture(sTexture, aTexPosition));\n" +
                                        //"   gl_FragColor.a = (gl_FragColor.a / 14944.0) * aAlpha;\n" +
                                        "   gl_FragColor.a = gl_FragColor.a - aAlpha;\n" +
                                        "}";

    private Context ogContext;
    private int igScrWidth;
    private int igScrHeight;
    private boolean bgInit;
    private float fgRatio;
    private float fgUnitX;
    private float fgUnitY;

    private float[] agPMatrix;
    private float[] agVMatrix;
    private LinkedList <Element> agElements;
    private boolean[] agTexture2D;
    private int[] agProgramIds;

    private EGLDisplay ogDisplay;
    private EGLContext ogWorkContext;
    private EGLSurface ogSurface;

    public Renderer(Context opCtx) {
        super();

        ogContext = opCtx;
        WindowManager olWinMng = (WindowManager) ogContext.getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics olMetrics = new DisplayMetrics();

        olWinMng.getDefaultDisplay().getMetrics(olMetrics);
        igScrWidth = olMetrics.widthPixels;
        igScrHeight = olMetrics.heightPixels;
        fgRatio = (float) igScrWidth / igScrHeight;
        fgUnitX = (fgRatio * 2) / igScrWidth;
        fgUnitY = (float) 2 / igScrHeight;

        agElements = new LinkedList <> ();

        agTexture2D = new boolean[MAX_TEXTURES];
        for(int i = 0; i < agTexture2D.length; i++)
            agTexture2D[i] = false;

        agProgramIds = new int[PROGRAMS_QUANTITY];

        bgInit = false;
    }

    private int compileShader(int ipShaderType, String spSourceCode) {
        int ilShaderHandler = GLES20.glCreateShader(ipShaderType);
        int[] alStatus = new int[1];

        if(ilShaderHandler == 0) {
            Log.e(TAG, "Error Compiling Shader:");
            Log.e(TAG, "No Handler");
            return 0;
        }

        GLES20.glShaderSource(ilShaderHandler, spSourceCode);
        GLES20.glCompileShader(ilShaderHandler);

        GLES20.glGetShaderiv(ilShaderHandler, GLES20.GL_COMPILE_STATUS, alStatus, 0);
        if(alStatus[0] != GLES20.GL_TRUE) {
            Log.e(TAG, "Error Compiling Shader:");
            Log.e(TAG, GLES20.glGetShaderInfoLog(ilShaderHandler));
            GLES20.glDeleteShader(ilShaderHandler);
            return 0;
        }

        return ilShaderHandler;
    }

    private int compileProgram(int ipVertexShader, int ipFragmentShader) {
        int igProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(igProgram, ipVertexShader);
        GLES20.glAttachShader(igProgram, ipFragmentShader);
        GLES20.glLinkProgram(igProgram);

        int[] alStatus = new int[1];
        GLES20.glGetProgramiv(igProgram, GLES20.GL_LINK_STATUS, alStatus, 0);
        if(alStatus[0] != GLES20.GL_TRUE) {
            Log.e(TAG, "Error Linking Program:");
            Log.e(TAG, GLES20.glGetProgramInfoLog(igProgram));
            GLES20.glDeleteProgram(igProgram);

            return 0;
        }

        return igProgram;
    }

    public void initWorkContext(javax.microedition.khronos.egl.EGLConfig opConfig) {
        javax.microedition.khronos.egl.EGL olOldEGL = javax.microedition.khronos.egl.EGLContext.getEGL();
        javax.microedition.khronos.egl.EGLDisplay olOldDisplay = ((javax.microedition.khronos.egl.EGL10) olOldEGL).eglGetCurrentDisplay();
        int[] alConfigId = new int[1];
        ((javax.microedition.khronos.egl.EGL10) olOldEGL).eglGetConfigAttrib(olOldDisplay, opConfig, ((javax.microedition.khronos.egl.EGL10) olOldEGL).EGL_CONFIG_ID, alConfigId);

        int alConfigurationAttribute[] = {
                EGL14.EGL_CONFIG_ID, alConfigId[0],
                EGL14.EGL_NONE
        };
        int alContextAttributes[] = {
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL14.EGL_NONE
        };
        int[] alSufaceAttributes = {
                EGL14.EGL_WIDTH, 2,
                EGL14.EGL_HEIGHT, 2,
                EGL14.EGL_NONE
        };
        int[] alConfigsNum = new int[1];
        int[] alVers = new int[2];
        EGLConfig[] olConfig = new EGLConfig[1];

        ogDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        if(ogDisplay.equals(EGL14.EGL_NO_DISPLAY)){
            Log.e(TAG, "Error getting Display.");
            return;
        }

        EGL14.eglInitialize(ogDisplay, alVers, 0, alVers, 1);

        EGL14.eglChooseConfig(ogDisplay, alConfigurationAttribute, 0, olConfig, 0, olConfig.length, alConfigsNum, 0);
        if(alConfigsNum[0] == 0) {
            Log.e(TAG, "Cannot find Configuration.");
            return;
        }

        if(EGL14.eglGetCurrentContext().equals(EGL14.EGL_NO_CONTEXT)) {
            Log.e(TAG, "No Main Context.");
            return;
        }

        ogWorkContext = EGL14.eglCreateContext(ogDisplay, olConfig[0], EGL14.eglGetCurrentContext(), alContextAttributes, 0);
        if(ogWorkContext.equals(EGL14.EGL_NO_CONTEXT)) {
            Log.e(TAG, "Error creating Working Context.");
            return;
        }

        ogSurface = EGL14.eglCreatePbufferSurface(ogDisplay, olConfig[0], alSufaceAttributes, 0);
        if(ogSurface.equals(EGL14.EGL_NO_SURFACE)) {
            Log.e(TAG, "Error creating Working Surface.");
            return;
        }
    }

    @Override
    public void onSurfaceCreated(GL10 opContext, javax.microedition.khronos.egl.EGLConfig opConfig) {

        initWorkContext(opConfig);

        agPMatrix = new float[16];
        agVMatrix = new float[16];

        Matrix.orthoM(agPMatrix, 0, -fgRatio, fgRatio, -1, 1, 9, -9);
        Matrix.setLookAtM(agVMatrix, 0, 0, 0, 7, 0, 0, 0, 0, 1, 0);

        int ilVertexShader = compileShader(GLES20.GL_VERTEX_SHADER, sgColorVShaderCode);
        if(ilVertexShader == 0) {
            Log.e(TAG, "Error creating Color Vertex Shader.");
            return;
        }

        int ilFragmentShader = compileShader(GLES20.GL_FRAGMENT_SHADER, sgColorFShaderCode);
        if(ilFragmentShader == 0) {
            Log.e(TAG, "Error creating Color Fragment Shader.");
            return;
        }

        agProgramIds[PRG_SOLID_COLOR] = compileProgram(ilVertexShader, ilFragmentShader);
        GLES20.glDeleteShader(ilVertexShader);
        GLES20.glDeleteShader(ilFragmentShader);

        if(agProgramIds[PRG_SOLID_COLOR] == 0) {
            Log.e(TAG, "Error creating Color Program.");
            return;
        }


        ilVertexShader = compileShader(GLES20.GL_VERTEX_SHADER, sgTextureVShaderCode);
        if(ilVertexShader == 0) {
            Log.e(TAG, "Error creating Texture Vertex Shader.");
            return;
        }

        ilFragmentShader = compileShader(GLES20.GL_FRAGMENT_SHADER, sgTextureFShaderCode);
        if(ilFragmentShader == 0) {
            Log.e(TAG, "Error creating Texture Fragment Shader.");
            return;
        }

        agProgramIds[PRG_SIMPLE_TEXTURE] = compileProgram(ilVertexShader, ilFragmentShader);
        GLES20.glDeleteShader(ilVertexShader);
        GLES20.glDeleteShader(ilFragmentShader);

        if(agProgramIds[PRG_SIMPLE_TEXTURE] == 0) {
            Log.e(TAG, "Error creating Texture Program.");
            return;
        }

        ilVertexShader = compileShader(GLES20.GL_VERTEX_SHADER, sgAlphaTextureVShaderCode);
        if(ilVertexShader == 0) {
            Log.e(TAG, "Error creating Alpha Texture Vertex Shader.");
            return;
        }

        ilFragmentShader = compileShader(GLES20.GL_FRAGMENT_SHADER, sgAlphaTextureFShaderCode);
        if(ilFragmentShader == 0) {
            Log.e(TAG, "Error creating Alpha Texture Fragment Shader.");
            return;
        }

        agProgramIds[PRG_ALPHA_TEXTURE] = compileProgram(ilVertexShader, ilFragmentShader);
        GLES20.glDeleteShader(ilVertexShader);
        GLES20.glDeleteShader(ilFragmentShader);

        if(agProgramIds[PRG_ALPHA_TEXTURE] == 0) {
            Log.e(TAG, "Error creating Alpha Texture Program.");
            return;
        }

        GLES20.glViewport(0, 0, igScrWidth, igScrHeight);
        GLES20.glEnable(GLES20.GL_DEPTH_TEST);
        GLES20.glEnable(GLES20.GL_BLEND);
        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_DST_ALPHA);

        bgInit = true;

    }

    @Override
    public void onDrawFrame(GL10 opContext) {

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        if(!bgInit || agElements.size() == 0) return;

        synchronized(agElements) {
            ListIterator <Element> olNext = agElements.listIterator();

            while(olNext.hasNext()) {
                Element olElement = olNext.next();

                synchronized (olElement) {
                    if (!olElement.isActive()) {
                        olNext.remove();
                        continue;
                    }

                    if (!olElement.isReady()|| !olElement.isDisplayed() || (olElement.getTexture() != null && !olElement.getTexture().isActive()))
                        continue;

                    int ilPosition = -1;
                    int ilColor = -1;
                    int ilTexPosition = -1;

                    ByteBuffer alTmp = ByteBuffer.allocateDirect(olElement.getVertex().length * FLOAT_BYTES);
                    alTmp.order(ByteOrder.nativeOrder());
                    FloatBuffer alVertex = alTmp.asFloatBuffer();
                    alVertex.put(olElement.getVertex());
                    alVertex.position(0);

                    alTmp = ByteBuffer.allocateDirect(olElement.getIndex().length * INTEGER_BYTES);
                    alTmp.order(ByteOrder.nativeOrder());
                    IntBuffer alIndex = alTmp.asIntBuffer();
                    alIndex.put(olElement.getIndex());
                    alIndex.position(0);

                    int ilProgramId = agProgramIds[olElement.getProgramInx()];

                    GLES20.glUseProgram(ilProgramId);

                    ilPosition = GLES20.glGetAttribLocation(ilProgramId, "vPosition");
                    GLES20.glEnableVertexAttribArray(ilPosition);
                    GLES20.glVertexAttribPointer(
                            ilPosition,
                            COORDS_PER_VERTEX,
                            GLES20.GL_FLOAT,
                            false,
                            COORDS_PER_VERTEX * 4,
                            alVertex
                    );

                    GLES20.glUniformMatrix4fv(
                            GLES20.glGetUniformLocation(ilProgramId, "vPMatrix"),
                            1,
                            false,
                            agPMatrix,
                            0
                    );

                    GLES20.glUniformMatrix4fv(
                            GLES20.glGetUniformLocation(ilProgramId, "vVMatrix"),
                            1,
                            false,
                            agVMatrix,
                            0
                    );

                    GLES20.glUniformMatrix4fv(
                            GLES20.glGetUniformLocation(ilProgramId, "vRotation"),
                            1,
                            false,
                            olElement.getRotationMatrix(),
                            0
                    );

                    GLES20.glUniformMatrix4fv(
                            GLES20.glGetUniformLocation(ilProgramId, "vTranslation"),
                            1,
                            false,
                            olElement.getTranslationMatix(),
                            0
                    );

                    if(olElement.getProgramInx()== PRG_SOLID_COLOR) {
                        ilColor = GLES20.glGetAttribLocation(ilProgramId, "vColor");
                        GLES20.glVertexAttrib4fv(ilColor, olElement.getColor(), 0);
                        GLES20.glLineWidth(olElement.getLineWidth());
                    }

                    if(olElement.getProgramInx() == PRG_SIMPLE_TEXTURE || olElement.getProgramInx() == PRG_ALPHA_TEXTURE) {
                        alTmp = ByteBuffer.allocateDirect(olElement.getTexel().length * FLOAT_BYTES);
                        alTmp.order(ByteOrder.nativeOrder());
                        FloatBuffer alTexel = alTmp.asFloatBuffer();
                        alTexel.put(olElement.getTexel());
                        alTexel.position(0);

                        ilTexPosition = GLES20.glGetAttribLocation(ilProgramId, "vTexPosition");
                        GLES20.glEnableVertexAttribArray(ilTexPosition);
                        GLES20.glVertexAttribPointer(
                                ilTexPosition,
                                COORDS_PER_TEXEL,
                                GLES20.GL_FLOAT,
                                false,
                                COORDS_PER_TEXEL * FLOAT_BYTES,
                                alTexel
                        );

                        Texture olTex = olElement.getTexture();
                        GLES20.glActiveTexture(GLES20.GL_TEXTURE0 + olTex.getUnit());
                        GLES20.glBindTexture(olTex.getType(), olTex.getId());

                        GLES20.glUniform1i(
                                GLES20.glGetUniformLocation(ilProgramId, "sTexture"),
                                olTex.getUnit()
                        );

                        if(olElement.getProgramInx() == PRG_ALPHA_TEXTURE)
                            GLES20.glVertexAttrib1f(
                                    GLES20.glGetAttribLocation(ilProgramId, "vAlpha"),
                                    olElement.getAlpha()
                            );
                    }

                    GLES20.glDrawElements(
                            olElement.getTypeId(),
                            olElement.getIndex().length,
                            GLES20.GL_UNSIGNED_INT,
                            alIndex
                    );

                    int ilError = GLES20.glGetError();
                    if(ilError != GLES20.GL_NO_ERROR)
                        Log.e(TAG, "Error Drawing: " + GLU.gluErrorString(ilError));

                    GLES20.glDisableVertexAttribArray(ilPosition);
                    if(olElement.getProgramInx() == PRG_SIMPLE_TEXTURE || olElement.getProgramInx() == PRG_ALPHA_TEXTURE)
                        GLES20.glDisableVertexAttribArray(ilTexPosition);

                    olElement.ready();
                }
            }
        }
        GLES20.glFinish();

    }

    @Override
    public void onSurfaceChanged(GL10 opCtx, int ipWidth, int ipHeight) {
        // Necessary Method since Upper Class is abstract.
    }

    public boolean isInitiated() { return bgInit; }

    public float getUnitX() { return fgUnitX; }

    public float getUnitY() { return fgUnitY; }

    public float toModelX(float ipX) {
        return (ipX * fgUnitX) - fgRatio;
    }

    public float toModelY(float ipY) {
        return 1 - (ipY * fgUnitY);
    }

    public void addElement(Element opElement) {
        synchronized(agElements) {
            agElements.add(opElement);
        }
    }

    public Texture loadTexture2d(int ipResource, boolean bpDoMipmap) {

        int ilUnit = Integer.MIN_VALUE;
        for(int i = 0; i < agTexture2D.length; i++)
            if(!agTexture2D[i]) {
                agTexture2D[i] = true;
                ilUnit = i;
                break;
            }
        if(ilUnit == Integer.MIN_VALUE)
            return null;

        InputStream olReader = ogContext.getResources().openRawResource(ipResource);
        Bitmap olBitmap;

        try{
            olBitmap = BitmapFactory.decodeStream(olReader);
        } finally {
            try {
                olReader.close();
            } catch(IOException e){

            }
        }

        int[] alTmp = new int[1];
        GLES20.glGenTextures(1, alTmp, 0);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0 + ilUnit);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, alTmp[0]);

        Texture olTex = new Texture(ilUnit, alTmp[0], GLES20.GL_TEXTURE_2D, olBitmap.getWidth(), olBitmap.getHeight());

        if(bpDoMipmap) {

            GLES20.glHint(GLES20.GL_GENERATE_MIPMAP_HINT, GLES20.GL_NICEST);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR_MIPMAP_NEAREST);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);

            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, olBitmap, GLES20.GL_UNSIGNED_BYTE, 0);
            int ilError = GLES20.glGetError();
            if(ilError != GLES20.GL_NO_ERROR)
                Log.e(TAG, "Error Loading Image: " + GLU.gluErrorString(ilError));

            GLES20.glGenerateMipmap(GLES20.GL_TEXTURE_2D);

        } else {

            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);

            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, olBitmap, GLES20.GL_UNSIGNED_BYTE, 0);
            int ilError = GLES20.glGetError();
            if(ilError != GLES20.GL_NO_ERROR)
                Log.e(TAG, "Error Loading Image: " + GLU.gluErrorString(ilError));
        }

        olBitmap.recycle();
        return olTex;

    }

    public void freeTexture2D(Texture opTexture) {
        int[] alTmp = {opTexture.getId()};
        GLES20.glDeleteTextures(1, alTmp, 0);
        agTexture2D[opTexture.getUnit()] = false;
    }

    public void setWorkContext() {
        if(!EGL14.eglGetCurrentContext().equals(EGL14.EGL_NO_CONTEXT)) {
            Log.e(TAG, "Actual Thread has already a Context assigned, free actual Context.");
            return;
        }

        if(!EGL14.eglMakeCurrent(ogDisplay, ogSurface, ogSurface, ogWorkContext))
            Log.e("TAG", "Error Setting Work Context: " + GLUtils.getEGLErrorString(EGL14.eglGetError()));
    }

    public void freeWorkContext() {
        if(EGL14.eglGetCurrentContext().equals(EGL14.EGL_NO_CONTEXT)) {
            Log.e(TAG, "Actual Thread has not Context.");
            return;
        }

        if(!EGL14.eglGetCurrentContext().equals(ogWorkContext)) {
            Log.e(TAG, "Actual Thread has not a Work Context.");
            return;
        }

        GLES20.glFinish();
        if(!EGL14.eglMakeCurrent(ogDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT))
            Log.e("TAG", "Error Setting Work Context: " + GLU.gluErrorString(EGL14.eglGetError()));
    }

    public void destroyWorkContext() {
        if(ogDisplay.equals(EGL14.EGL_NO_DISPLAY))
            return;

        GLES20.glFinish();

        if(!ogSurface.equals(EGL14.EGL_NO_SURFACE))
            EGL14.eglDestroySurface(ogDisplay, ogSurface);

        if(!ogWorkContext.equals(EGL14.EGL_NO_CONTEXT))
            EGL14.eglDestroyContext(ogDisplay, ogWorkContext);

        EGL14.eglTerminate(ogDisplay);

        ogDisplay = null;
        ogSurface = null;
        ogWorkContext = null;
    }

}
