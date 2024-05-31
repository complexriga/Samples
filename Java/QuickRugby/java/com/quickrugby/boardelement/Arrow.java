package com.quickrugby.boardelement;

import android.opengl.GLES20;
import android.view.MotionEvent;
import com.quickrugby.gamerenderer.Renderer;
import com.quickrugby.gamerenderer.Element;

/**
 * Created by Andres Giraldo on 03.06.2016.
 */

public class Arrow extends Line {

    private Element ogHead;

    public Arrow(Renderer opRenderer) {
        super(opRenderer);

        ogHead = new Element();
        ogHead.setTAG("Arrow Head");
        ogHead.setTypeId(GLES20.GL_LINE_STRIP);
        ogHead.setProgramInx(ogRenderer.PRG_SOLID_COLOR);
        ogHead.setLineWidth(ogLine.getLineWidth());
        ogHead.setColor(ogLine.getColor());

        float[] alVertex = new float[9];
        int[] alIndex = new int[3];

        alVertex[0] = ogRenderer.getUnitX() * 35f;
        alVertex[1] = 0f;
        alVertex[2] = 0f;
        alVertex[3] = 0f;
        alVertex[4] = 0f;
        alVertex[5] = 0f;
        alVertex[6] = 0f;
        alVertex[7] = ogRenderer.getUnitY() * 35f;
        alVertex[8] = 0f;

        alIndex[0] = 0;
        alIndex[1] = 1;
        alIndex[2] = 2;

        ogHead.setVertex(alVertex);
        ogHead.setIndex(alIndex);
        ogHead.hide();
        ogHead.ready();

        ogRenderer.addElement(ogHead);
    }

    @Override
    public void setColor(float[] apColor) {
        super.setColor(apColor);
        ogHead.setColor(getColor());
    }

    @Override
    public void setWidth(float fpWidth) {
        super.setWidth(fpWidth);
        ogHead.setLineWidth(getWidth());
    }

    @Override
    public void trace(MotionEvent opEvent) {
        super.trace(opEvent);

        if(opEvent.getAction() == MotionEvent.ACTION_UP) {
            ogHead.rotate(agPoints.lastElement().s - 225f);
            ogHead.translate(agPoints.lastElement().x, agPoints.lastElement().y, 0f);
            ogHead.show();
            ogHead.ready();
        }
    }

    @Override
    public void delete() {
        super.delete();
        ogHead.inactive();
    }
}
