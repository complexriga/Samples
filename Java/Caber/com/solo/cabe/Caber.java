package com.solo.cabe;

import java.util.ArrayList;
import java.util.Random;

public class Caber {
    static final int FROM_DOWN = 1;
    static final int FROM_UP = 2;

    class Cross {
        public int du;
        public int dd;
        
        public Cross(int pDU, int pDD) {
            du = pDU;
            dd = pDD;
        }
    }
    
    public class Position {
        public int x;
        public int y;
        
        public Position(int pX, int pY) {
            x = pX;
            y = pY;
        }
    }

    public class Piece {
        public int type;
        public int duCnt; // Counts the intersections of a PIECE_X on a DU diagonal with a diagonal, column or row of othe piece.
        public int ddCnt; // Counts the intersections of a PIECE_X on a DD diagonal with a diagonal, column or row of othe piece.
        public int duLen; // Counts the number of position of a DU diagonal on which is a PIECE_X that are intersected by the diagonals, columns, or rows of other pieces.
        public int ddLen; // Counts the number of position of a DD diagonal on which is a PIECE_X that are intersected by the diagonals, columns, or rows of other pieces.
        public Position pos;
        
        public Piece(int pType, Position pPos) {
            type = pType;
            pos = pPos;
            duCnt = 0;
            ddCnt = 0;
            duLen = 0;
            ddLen = 0;
        }
        
        public boolean equals(Piece vPiece) {
            return type == vPiece.type && pos.x == vPiece.pos.x && pos.y == vPiece.pos.y;
        }
    }
    
    int aTryOutCount;
    int aTotalMoveCount;
    int aXFailedCount;
    int a0Count;
    int a1Count;
    int a2Count;
    int a3Count;
    int a4Count;

    Board aBoard;
    Piece[] aPieces;
    
    ArrayList<Piece> aTryPieces;
    
    public Caber(int pWidth, int pHeight, int pFilter, int pSPieceQty, int pCPieceQty, int pXPieceQty, boolean pAutomatic) {
            if(pWidth < 5 || pWidth > 25)
                throw new RuntimeException("Caber.Caber: Width must be a Value between 5 and 25.");

            if(pHeight < 5 || pHeight > 25)
                throw new RuntimeException("Caber.Caber: Height must be a Value between 5 and 25.");

            int vPieceTotal = pSPieceQty + pCPieceQty + pXPieceQty;
            aPieces = new Piece[vPieceTotal];
            aTryPieces = new ArrayList<Piece>();
            
            if(vPieceTotal == 0) {
                try {
                    aBoard = new Board(pWidth, pHeight, 0, 0, 0, pAutomatic, pFilter);
                } catch(Exception e) {
                    throw new RuntimeException("Caber.Caber: " + e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
                }
                return;
            }
            
            try {
                aBoard = new Board(pWidth, pHeight, pSPieceQty, pCPieceQty, pXPieceQty, pAutomatic, pFilter);
            } catch(Exception e) {
                throw new RuntimeException("Caber.Caber: " + e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
            }
            
            // Builds the matrix of diagonals to facilitate the search of conflicts and initializes the piece access arrays.
            int[][] vRisks = new int[pWidth][pHeight];
            ArrayList<ArrayList<Piece>> vPiecesByColumn = new ArrayList<ArrayList<Piece>>(pWidth);
            Cross[][] vCrosses = new Cross[pWidth][pHeight];
            int vDUCnt = pHeight - 1;
            int vDDCnt = 0;
            for(int i = 0; i < pWidth; i++) {
                for(int j = 0; j < pHeight; j++) {
                    vCrosses[i][j] = new Cross(vDUCnt--, vDDCnt++);
                    vRisks[i][j] = 0;
                }
                vDUCnt += pHeight + 1;
                vDDCnt -= pHeight - 1;
                vPiecesByColumn.add(new ArrayList<Piece>());
            }
            
            // If several pieces can attack the same position by the same vector then It counts that vector only once using these lock arrays.
            int[] vDULock = new int[pWidth + pHeight];
            int[] vDDLock = new int[pWidth + pHeight];
            int[] vXLock = new int[pHeight];
            int[] vYLock = new int[pHeight];
            
            boolean vHasFailed = true;
            aTryOutCount = 0;
            aTotalMoveCount = 0;
            aXFailedCount = 0;
            while(aTryOutCount < 1000 && vHasFailed) {
                // Initialize the positions of the pieces.
                int vSPieceCnt = pSPieceQty;
                int vCPieceCnt = pCPieceQty;
                int vXPieceCnt = pXPieceQty;
                Random vRand = new Random();
                ArrayList<Position> vPositionList = new ArrayList<Position>();
                for(int k = 0; k < vPiecesByColumn.size(); k++)
                    vPiecesByColumn.get(k).clear();
                for(int p = 0; p < aPieces.length; p++) {
                    // Reckons the risks.
                    for(int i = 0; i < pWidth; i++) {
                        for(int t = 0; t < pHeight; t++) {
                            vRisks[i][t] = 0;
                            vXLock[t] = 0;
                            vYLock[t] = 0;
                        }
                        for(int t = 0; t < vDULock.length; t++) {
                            vDULock[t] = 0;
                            vDDLock[t] = 0;
                        }

                        for(int k = 0; k < pWidth && p != 0; k++) {
                            for(int j = 0; j < pHeight; j++)
                                for(int t = 0; t < vPiecesByColumn.get(k).size(); t++)
                                    if(k != i) {
                                        if((vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_C) && vYLock[j] == 0 && vPiecesByColumn.get(k).get(t).pos.y == j) {
                                            vRisks[i][j]++;
                                            vYLock[j] = 1;
                                        }
                                        if((vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_X) && vDULock[vCrosses[i][j].du] == 0 && vCrosses[k][vPiecesByColumn.get(k).get(t).pos.y].du == vCrosses[i][j].du) {
                                            vRisks[i][j]++;
                                            vDULock[vCrosses[i][j].du] = 1;
                                        }
                                        if((vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_X) && vDDLock[vCrosses[i][j].dd] == 0 && vCrosses[k][vPiecesByColumn.get(k).get(t).pos.y].dd == vCrosses[i][j].dd) {
                                            vRisks[i][j]++;
                                            vDDLock[vCrosses[i][j].dd] = 1;
                                        }
                                    } else {
                                        if(vPiecesByColumn.get(k).get(t).pos.y != j && (vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_C)) {
                                            if((vXLock[j] & FROM_DOWN) == 0 && vPiecesByColumn.get(k).get(t).pos.y < j) {
                                                vRisks[i][j]++;
                                                vXLock[j] |= FROM_DOWN;
                                            } else if((vXLock[j] & FROM_UP) == 0 && vPiecesByColumn.get(k).get(t).pos.y > j) {
                                                vRisks[i][j]++;
                                                vXLock[j] |= FROM_UP;
                                                
                                            }
                                        } else
                                            if(vPiecesByColumn.get(k).get(t).pos.y == j)
                                                vRisks[i][j]++;
                                    }
                            if(k == i) {
                                for(int t = 0; t < pHeight; t++) {
                                    vXLock[t] = 0;
                                    vYLock[t] = 0;
                                }
                                for(int t = 0; t < vDULock.length; t++) {
                                    vDULock[t] = 0;
                                    vDULock[t] = 0;
                                }
                            }
                        }
                    }

                    int vRiskMin = 9999;
                    for(int i = 0; i < pWidth; i++)
                        for(int j = 0; j < pHeight; j++) {
                            if(vRisks[i][j] < vRiskMin) {
                                vRiskMin = vRisks[i][j];
                                vPositionList.clear();
                            }
                            if(vRisks[i][j] == vRiskMin)
                                vPositionList.add(new Position(i, j));
                        }
                    
                    int vType = -1;
                    if(vSPieceCnt != 0) {
                        vType = Board.PIECE_S;
                        vSPieceCnt--;
                    } else if(vCPieceCnt != 0) {
                        vType = Board.PIECE_C;
                        vCPieceCnt--;
                    } else if(vXPieceCnt != 0) {
                        vType = Board.PIECE_X;
                        vXPieceCnt--;
                    } else
                        throw new RuntimeException("Caber.Caber: No Piece Types selected.");
                    aPieces[p] = new Piece(vType, vPositionList.get(vRand.nextInt(vPositionList.size())));
                    vPiecesByColumn.get(aPieces[p].pos.x).add(aPieces[p]);
                }

                // Searches for the positions that have not conflicts.
                int vMoveCount = 0;
                while(vMoveCount < 50) {
                    // Reckons the risks.
                    for(int i = 0; i < pWidth; i++) {
                        for(int t = 0; t < pHeight; t++) {
                            vRisks[i][t] = 0;
                            vXLock[t] = 0;
                            vYLock[t] = 0;
                        }
                        for(int t = 0; t < vDULock.length; t++) {
                            vDULock[t] = 0;
                            vDDLock[t] = 0;
                        }

                        for(int k = 0; k < pWidth; k++) {
                            for(int j = 0; j < pHeight; j++)
                                for(int t = 0; t < vPiecesByColumn.get(k).size(); t++)
                                    if(k != i) {
                                        if((vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_C) && vYLock[j] == 0 && vPiecesByColumn.get(k).get(t).pos.y == j) {
                                            vRisks[i][j]++;
                                            vYLock[j] = 1;
                                        }
                                        if(vCrosses[k][vPiecesByColumn.get(k).get(t).pos.y].du == vCrosses[i][j].du) {
                                            vPiecesByColumn.get(k).get(t).duLen++;
                                            if((vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_X) && vDULock[vCrosses[i][j].du] == 0) {
                                                vRisks[i][j]++;
                                                vDULock[vCrosses[i][j].du] = 1;
                                                vPiecesByColumn.get(k).get(t).duCnt++;
                                            }
                                        }
                                        if(vCrosses[k][vPiecesByColumn.get(k).get(t).pos.y].dd == vCrosses[i][j].dd) {
                                            vPiecesByColumn.get(k).get(t).ddLen++;
                                            if((vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_X) && vDDLock[vCrosses[i][j].dd] == 0) {
                                                vRisks[i][j]++;
                                                vDDLock[vCrosses[i][j].dd] = 1;
                                                vPiecesByColumn.get(k).get(t).ddCnt++;
                                            }
                                        }
                                    } else {
                                        if(vPiecesByColumn.get(k).get(t).pos.y != j && (vPiecesByColumn.get(k).get(t).type == Board.PIECE_S || vPiecesByColumn.get(k).get(t).type == Board.PIECE_C)) {
                                            if((vXLock[j] & FROM_DOWN) == 0 && vPiecesByColumn.get(k).get(t).pos.y < j) {
                                                vRisks[i][j]++;
                                                vXLock[j] |= FROM_DOWN;
                                            } else if((vXLock[j] & FROM_UP) == 0 && vPiecesByColumn.get(k).get(t).pos.y > j) {
                                                vRisks[i][j]++;
                                                vXLock[j] |= FROM_UP;
                                                
                                            }
                                        } else  // This option is left to solve the problem of testing a solutions that has Xs on the corners.
                                            if(vPiecesByColumn.get(k).get(t).pos.y == j)
                                                vRisks[i][j]++;
                                    }
                            if(k == i) {
                                for(int t = 0; t < pHeight; t++) {
                                    vXLock[t] = 0;
                                    vYLock[t] = 0;
                                }
                                for(int t = 0; t < vDULock.length; t++) {
                                    vDULock[t] = 0;
                                    vDULock[t] = 0;
                                }
                            }
                        }
                    }
                    
                    boolean vInd = false;
                    int vRiskMax = -1;
                    ArrayList<Piece> vPieceList = new ArrayList<Piece>();
                    for(int p = 0; p < aPieces.length; p++) {
                        if(vRisks[aPieces[p].pos.x][aPieces[p].pos.y] > 1 && vRisks[aPieces[p].pos.x][aPieces[p].pos.y] > vRiskMax) {
                            vRiskMax = vRisks[aPieces[p].pos.x][aPieces[p].pos.y];
                            vPieceList.clear();
                        }
                        if(vRisks[aPieces[p].pos.x][aPieces[p].pos.y] == vRiskMax)
                            vPieceList.add(aPieces[p]);
                        if(aPieces[p].type == Board.PIECE_X && ((aPieces[p].duCnt == 0 || aPieces[p].ddCnt == 0) && (aPieces[p].duCnt + 1 != aPieces[p].duLen) && (aPieces[p].ddCnt + 1 != aPieces[p].ddLen)))
                            vInd = true;
                        aPieces[p].duCnt = 0;
                        aPieces[p].ddCnt = 0;
                        aPieces[p].duLen = 0;
                        aPieces[p].ddLen = 0;
                    }
                    if(vPieceList.isEmpty()) {
                        if(vInd)
                            aXFailedCount++;
                        vHasFailed = vInd;
                        break;
                    }
                    
                    int vRiskMin = 9999;
                    for(int i = 0; i < pWidth; i++)
                        for(int j = 0; j < pHeight; j++) {
                            if(vRisks[i][j] < vRiskMin) {
                                vRiskMin = vRisks[i][j];
                                vPositionList.clear();
                            }
                            if(vRisks[i][j] == vRiskMin)
                                vPositionList.add(new Position(i, j));
                        }
                    Piece vTmpPiece = vPieceList.get(vRand.nextInt(vPieceList.size()));
                    vPiecesByColumn.get(vTmpPiece.pos.x).remove(vTmpPiece);
                    vTmpPiece.pos = vPositionList.get(vRand.nextInt(vPositionList.size()));
                    vPiecesByColumn.get(vTmpPiece.pos.x).add(vTmpPiece);
                    
                    vMoveCount++;
                }
                aTotalMoveCount += vMoveCount;
                aTryOutCount++;
            }
            
            if(vHasFailed)
                throw new RuntimeException("Caber.Caber: No Valid Boards found!!!");
            
            for(int i = 0; i < pWidth; i ++)
                for(int j = 0; j < pHeight; j ++) {
                    switch(vRisks[i][j]) {
                        case 0:
                            a0Count++;
                            break;
                        case 1:
                            a1Count++;
                            break;
                        case 2:
                            a2Count++;
                            break;
                        case 3:
                            a3Count++;
                            break;
                        case 4:
                            a4Count++;
                            break;
                        default:
                            throw new RuntimeException("Caber.Caber: Invalid Risk: " + vRisks[i][j]);
                    }
                    aBoard.setValue(i, j, vRisks[i][j]);
                }
    }
    
    public boolean test() {
        if(aTryPieces.size() != aPieces.length)
            return false;
        for(int i = 0; i < aPieces.length; i++) {
            boolean vInd = true;
            for(int j = 0; j < aTryPieces.size(); j++)
                if(aPieces[i].pos.x == aTryPieces.get(j).pos.x && aPieces[i].pos.y == aTryPieces.get(j).pos.y && aPieces[i].type == aTryPieces.get(j).type) {
                    vInd = false;
                    break;
                }
            if(vInd)
                return false;
        }
        return true;
    }
    
    public void reset() {
        aTryPieces.clear();
        aBoard.reset();
    }
    
    public boolean isAutomatic() {
        return aBoard.isAutomatic();
    }
    
    public void setAutomatic(boolean pIsAutomatic) {
        aBoard.setAutomatic(pIsAutomatic);
    }
    
    public int getFilter() {
        return aBoard.getFilter();
    }
    
    public Board getBoard() {
        return aBoard;
    }
    
    public int getSPieceQuantity() {
        return aBoard.getSPieceQuantity();
    }
    
    public int getCPieceQuantity() {
        return aBoard.getCPieceQuantity();
    }
    
    public int getXPieceQuantity() {
        return aBoard.getXPieceQuantity();
    }
    
    public int getSPieceCount() {
        return aBoard.getSPieceCount();
    }
    
    public int getCPieceCount() {
        return aBoard.getCPieceCount();
    }
    
    public int getXPieceCount() {
        return aBoard.getXPieceCount();
    }
    
    public int getTryOutCount() {
        return aTryOutCount;
    }
    
    public int getMovementCount() {
        return aTotalMoveCount;
    }
    
    public int getXFailedCount() {
        return aXFailedCount;
    }
    
    public int get0Count() {
        return a0Count;
    }
    
    public int get1Count() {
        return a1Count;
    }
    
    public int get2Count() {
        return a2Count;
    }
    
    public int get3Count() {
        return a3Count;
    }
    
    public int get4Count() {
        return a4Count;
    }
    
    public int getPiece(int pX, int pY) {
        try {
            return aBoard.getPiece(pX, pY);
        } catch(Exception e) {
            throw new RuntimeException("Caber.getPiece: " + e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
        }
    }
    
    public void setPiece(int pX, int pY, int pPieceType) {
        try {
            aBoard.setPiece(pX, pY, pPieceType);
        } catch(Exception e) {
            throw new RuntimeException("Caber.setPiece: " + e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
        }
        
        int vPieceType = pPieceType;
        for(int i = 0, cnt = 0; i < aTryPieces.size() && cnt < 3; i++) {
            Piece vPiece = aTryPieces.get(i);
            if(vPiece.pos.x == pX && vPiece.pos.y == pY)
                switch(vPiece.type) {
                    case Board.PIECE_S:
                        if((vPieceType & Board.PIECE_S) != 0)
                            vPieceType &= ~Board.PIECE_S;
                        else
                            aTryPieces.remove(i--);
                        cnt++;
                        break;
                    case Board.PIECE_C:
                        if((vPieceType & Board.PIECE_C) != 0)
                            vPieceType &= ~Board.PIECE_C;
                        else
                            aTryPieces.remove(i--);
                        cnt++;
                        break;
                    case Board.PIECE_X:
                        if((vPieceType & Board.PIECE_X) != 0)
                            vPieceType &= ~Board.PIECE_X;
                        else
                            aTryPieces.remove(i--);
                        cnt++;
                        break;
                    default:
                        throw new RuntimeException("Caber.setPiece: Invalid Piece Type of Played Piece.");
                }
        }
        if((vPieceType & Board.PIECE_S) != 0)
            aTryPieces.add(new Piece(Board.PIECE_S, new Position(pX, pY)));
        if((vPieceType & Board.PIECE_C) != 0)
            aTryPieces.add(new Piece(Board.PIECE_C, new Position(pX, pY)));
        if((vPieceType & Board.PIECE_X) != 0)
            aTryPieces.add(new Piece(Board.PIECE_X, new Position(pX, pY)));
    }
    
    public int getRisk(int pX, int pY) {
        try {
            return aBoard.getRisk(pX, pY);
        } catch(Exception e) {
            throw new RuntimeException("Caber.getRisk: " + e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
        }
    }
    
    public void setRisk(int pX, int pY, int pRisk) {
        try {
            aBoard.setRisk(pX, pY, pRisk);
        } catch(Exception e) {
            throw new RuntimeException("Caber.setRisk: " + e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
        }
    }
    
    public Piece[] getTryPieces() {
        Piece[] vTmpArray = new Piece[aTryPieces.size()];
        return aTryPieces.toArray(vTmpArray);
    }
    
    public Piece[] getSolution() {
        return aPieces;
    }
}