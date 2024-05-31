package com.quickrugby;

import java.util.Vector;
import android.content.Context;
import android.util.Log;
import android.view.MotionEvent;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.quickrugby.boardelement.Arrow;
import com.quickrugby.boardelement.Line;
import com.quickrugby.gamerenderer.Element;
import com.quickrugby.gamerenderer.Texture;

import android.opengl.GLES20;

/**
 * Created by Andres Giraldo on 28.05.2016.
 */

public class GameSpace extends GLSurfaceView {

    class Worker extends Thread {

        private float fgAlpha;
        private Vector<MotionEvent> ogEvents;
        private Element ogElement;
        private Vector<Line> ogElements;

        Worker() {
            super();

            ogElements = new Vector <> ();
            ogEvents = new Vector <> ();
            fgAlpha = 1f;

        }


        @Override
        public void run() {

            while(true) {
                if(!ogRenderer.isInitiated())
                    continue;

                if (ogElement == null) {
                    ogRenderer.setWorkContext();
                    ogElement = new Element();

                    ogElement.setProgramInx(ogRenderer.PRG_ALPHA_TEXTURE);
                    ogElement.setTypeId(GLES20.GL_TRIANGLE_STRIP);

                    float[] alVertex = {
                            0f, ogRenderer.getUnitY() * 100f, .1f,
                            ogRenderer.getUnitX() * 100f, ogRenderer.getUnitY() * 100f, .1f,
                            ogRenderer.getUnitX() * 100f, 0f, .1f,
                            0f, 0f, .1f
                    };
                    float[] alTexel = {
                            0f, 0f,
                            1f, 0f,
                            1f, 1f,
                            0f, 1f
                    };
                    int[] alIndex = {0, 1, 2, 2, 3, 0};

                    ogElement.setVertex(alVertex);
                    ogElement.setTextel(alTexel);
                    ogElement.setIndex(alIndex);

                    Texture olTex = ogRenderer.loadTexture2d(R.drawable.ic_board_actions, true);
                    ogElement.setTexture(olTex);

                    ogRenderer.freeWorkContext();
                    ogElement.ready();
                    ogRenderer.addElement(ogElement);
                }

                if(fgAlpha >= -1f)
                    fgAlpha -= .0001f;
                else
                    fgAlpha = 1f;

                ogElement.setAlpha(Math.abs(fgAlpha));
                ogElement.ready();

                synchronized(ogEvents) {
                    for(MotionEvent olEvent : ogEvents){
                        /*
                        if (ogElement.isWithin(ogRenderer.toModelX(olEvent.getX()), ogRenderer.toModelY(olEvent.getY())))
                            Log.e(TAG, "S");
                        else
                            Log.e(TAG, "N");
                        */

                        if (olEvent.getAction() == MotionEvent.ACTION_DOWN)
                            ogElements.add(new Arrow(ogRenderer));

                        ogElements.lastElement().trace(olEvent);
                    }

                    ogEvents.clear();
                }
            }
        }

        public void onMotionEvent(MotionEvent opEvent) {
            synchronized(ogEvents) {
                ogEvents.add(MotionEvent.obtain(opEvent));
            }
        }

    }

    final String TAG = "GameSpace";

    private com.quickrugby.gamerenderer.Renderer ogRenderer;
    private Worker ogWorker;

    GameSpace(Context opCtx) {
        super(opCtx);
        init(opCtx);
    }

    GameSpace(Context opCtx, AttributeSet apAttrSet) {
        super(opCtx, apAttrSet);
        init(opCtx);
    }

    private void init(Context opCtx) {
        setEGLContextClientVersion(2);
        ogRenderer = new com.quickrugby.gamerenderer.Renderer(opCtx);
        setRenderer(ogRenderer);

        ogWorker = new Worker();
        ogWorker.start();
    }

    public boolean onTouchEvent(MotionEvent opEvent) {
        super.onTouchEvent(opEvent);
        ogWorker.onMotionEvent(opEvent);

        return true;
    }

}
