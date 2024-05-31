package com.quickrugby.boardelement;

import android.opengl.GLES20;
import android.view.MotionEvent;

import java.util.Vector;
import com.quickrugby.gamerenderer.Element;
import com.quickrugby.gamerenderer.Renderer;

/**
 * Created by Andres Giraldo on 03.06.2016.
 */

public class Line {

    protected class Coordinate {

        public float x;
        public float y;
        public float s;

        public Coordinate(float ipX, float ipY, float fpS) {
            x = ipX;
            y = ipY;
            s = fpS;
        }

    }

    protected Renderer ogRenderer;
    protected Element ogLine;
    protected Vector<Coordinate> agPoints;

    public Line(Renderer opRenderer) {
        ogRenderer = opRenderer;

        agPoints = new Vector <> ();
        float[] alColor = new float[4];
        alColor[0] = 1f;
        alColor[1] = 1f;
        alColor[2] = 1f;
        alColor[3] = 1f;

        ogLine = new Element();
        ogLine.setTAG("Arrow Body");
        ogLine.setTypeId(GLES20.GL_LINE_STRIP);
        ogLine.setProgramInx(ogRenderer.PRG_SOLID_COLOR);
        ogLine.setLineWidth(10f);
        ogLine.setColor(alColor);
        ogRenderer.addElement(ogLine);
    }

    public float getWidth() {
        return ogLine.getLineWidth();
    }

    public void setWidth(float ipWidth) {
        ogLine.setLineWidth(ipWidth);
    }

    public float[] getColor() {
        return ogLine.getColor();
    }

    public void setColor(float[] apColor) {
        ogLine.setColor(apColor);
    }

    public void trace(MotionEvent opEvent) {
        float ilX = ogRenderer.toModelX(opEvent.getX());
        float ilY = ogRenderer.toModelY(opEvent.getY());

        if(opEvent.getAction() == MotionEvent.ACTION_DOWN) {
            agPoints.clear();
            agPoints.add(new Coordinate(ilX, ilY, 0));
        } else if(opEvent.getAction() == MotionEvent.ACTION_MOVE || opEvent.getAction() == MotionEvent.ACTION_UP) {
            if(agPoints.size() == 1) {
                agPoints.add(new Coordinate(ilX, ilY, 0));

                agPoints.get(1).s = (float) Math.toDegrees(Math.atan((agPoints.get(1).y - agPoints.get(0).y) / (agPoints.get(1).x - agPoints.get(0).x - ((agPoints.get(1).x - agPoints.get(0).x) == 0? 0.1e23f: 0))));
                if((agPoints.get(1).y > agPoints.get(0).y && agPoints.get(1).x <= agPoints.get(0).x) || (agPoints.get(1).y <= agPoints.get(0).y && agPoints.get(1).x < agPoints.get(0).x))
                    agPoints.get(1).s += 180f;
            } else {
                float flSlope = (float) Math.toDegrees(Math.atan((ilY - agPoints.lastElement().y) / (ilX - agPoints.lastElement().x - ((ilX - agPoints.lastElement().x) == 0? 0.1e23f: 0))));
                if((ilY >= agPoints.lastElement().y && ilX < agPoints.lastElement().x) || (ilY < agPoints.lastElement().y && ilX <= agPoints.lastElement().x))
                    flSlope += 180f;

                if(agPoints.lastElement().s != flSlope) {
                    agPoints.add(new Coordinate(ilX, ilY, flSlope));
                } else {
                    agPoints.lastElement().x = ilX;
                    agPoints.lastElement().y = ilY;
                }
            }
        }

        if(ogLine != null) {
            synchronized (ogLine) {
                float[] alVertex = new float[agPoints.size() * 3];
                int[] alIndex = new int[agPoints.size()];

                for (int i = 0, j = 0; i < agPoints.size(); i++) {
                    alVertex[j++] = agPoints.get(i).x;
                    alVertex[j++] = agPoints.get(i).y;
                    alVertex[j++] = 0f;
                    alIndex[i] = i;
                }
                ogLine.setVertex(alVertex);
                ogLine.setIndex(alIndex);
                ogLine.ready();
            }
        }
    }

    public void delete() {
        ogLine.inactive();
    }

}
