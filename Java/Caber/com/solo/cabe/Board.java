package com.solo.cabe;

public class Board {
    public static final int VALUE_NULL = -1;
    
    public static final int PIECE_NULL = 0;
    public static final int PIECE_S = 1;
    public static final int PIECE_C = 2;
    public static final int PIECE_X = 4;
    
    public static final int RISK_NULL = 0;
    public static final int RISK_0 = 1;
    public static final int RISK_1 = 2;
    public static final int RISK_2 = 4;
    public static final int RISK_3 = 8;
    public static final int RISK_4 = 16;
    public static final int RISK_E5 = 32;
    public static final int RISK_E6 = 64;
    public static final int RISK_E7 = 128;
    public static final int RISK_E8 = 256;
    public static final int RISK_E9 = 512;
    
    public static final int DETAIL_LEVEL_1 = 1;
    public static final int DETAIL_LEVEL_2 = 2;
    public static final int DETAIL_LEVEL_3 = 3;

    public static final int DIR_H = 1;
    public static final int DIR_V = 2;
    public static final int DIR_DU = 4;
    public static final int DIR_DD = 8;
    
    public class Cell {
        public int value;
        public int piece;
        public int risk;
        
        public Cell(int pValue, int pPiece, int pRisk) {
            value = pValue;
            piece = pPiece;
            risk = pRisk;
        }
    }
    
    boolean aAutomatic;
    int aWidth, aHeight, aDefaultRisk, aPieceQty, aSPieceQty, aCPieceQty, aXPieceQty, aPieceCnt, aSPieceCnt, aCPieceCnt, aXPieceCnt, aFilter;
    Cell[][] aBoard;
    
    public Board(int pWidth, int pHeight, int pSPieceQty, int pCPieceQty, int pXPieceQty, boolean pAutomatic, int pFilter) {
        if(pWidth < 5 || pWidth > 99)
            throw new RuntimeException("Board.Board: Width cannot be lesser than 5 nor greater than 99: " + pWidth);
        aWidth = pWidth;
        
        if(pHeight < 5 || pHeight > 99)
            throw new RuntimeException("Board.Board: Height cannot be lesser than 5 not greater than 99: " + pHeight);
        aHeight = pHeight;

        if(pFilter < 0 || pFilter > 4)
            throw new RuntimeException("Caber.Caber: Filter must be a Value between 0 and 4.");
        aFilter = pFilter;
        
        if(pSPieceQty < 0 || pCPieceQty < 0 || pXPieceQty < 0)
            throw new RuntimeException("Board.Board: PieceQty must be a Value greater than or equal to 0.");
        
        aPieceQty = pSPieceQty + pCPieceQty + pXPieceQty;
        if(aPieceQty != 0) {
            aSPieceQty = pSPieceQty;
            aCPieceQty = pCPieceQty;
            aXPieceQty = pXPieceQty;

            if(aPieceQty < 2 || aPieceQty > (aWidth <= aHeight? aWidth: aHeight))
                throw new RuntimeException("Board.Board: The Total of Pieces must be a Value between 2 and the Shortest Side of the Board: " + aPieceQty);
        } else {
            aSPieceQty = 99;
            aCPieceQty = 99;
            aXPieceQty = 99;
            aPieceQty = aSPieceQty + aCPieceQty + aXPieceQty;
        }
        
        aAutomatic = pAutomatic;
        aBoard = new Cell[aWidth][aHeight];
        reset();
    }
    
    public void reset() {
        aPieceCnt = 0;
        aSPieceCnt = 0;
        aCPieceCnt = 0;
        aXPieceCnt = 0;
        aDefaultRisk = 0;
        switch(aPieceQty >= 1 && aPieceQty <= 9? aPieceQty: 9) {
            case 9:
                aDefaultRisk |= Board.RISK_E9;
            case 8:
                aDefaultRisk |= Board.RISK_E8;
            case 7:
                aDefaultRisk |= Board.RISK_E7;
            case 6:
                aDefaultRisk |= Board.RISK_E6;
            case 5:
                aDefaultRisk |= Board.RISK_E5;
            case 4:
                aDefaultRisk |= Board.RISK_4;
            case 3:
                aDefaultRisk |= Board.RISK_3;
            case 2:
                aDefaultRisk |= Board.RISK_2;
            case 1:
                aDefaultRisk |= Board.RISK_1;
        }
        for(int i = 0; i < aWidth; i++)
            for(int j = 0; j < aHeight; j++)
                if(aBoard[i][j] != null) {
                    aBoard[i][j].piece = 0;
                    aBoard[i][j].risk = 0;
                } else
                    aBoard[i][j] = new Cell(VALUE_NULL, 0, 0);
    }
    
    public boolean isAutomatic() {
        return aAutomatic;
    }
    
    public void setAutomatic(boolean pIsAutomatic) {
        if(!aAutomatic && pIsAutomatic)
            reset();
        aAutomatic = pIsAutomatic;
    }
    
    public int getWidth() {
        return aWidth;
    }
    
    public int getHeight() {
        return aHeight;
    }
    
    public int getPieceQuantity() {
        return aPieceQty;
    }
    
    public int getSPieceQuantity() {
        return aSPieceQty;
    }
    
    public int getCPieceQuantity() {
        return aCPieceQty;
    }
    
    public int getXPieceQuantity() {
        return aXPieceQty;
    }
    
    public int getSPieceCount() {
        return aSPieceCnt;
    }
    
    public int getCPieceCount() {
        return aCPieceCnt;
    }
    
    public int getXPieceCount() {
        return aXPieceCnt;
    }
    
    public int getFilter() {
        return aFilter;
    }
    
    public int getValue(int pX, int pY) {
        if(pX < 0 || pX >= aWidth)
            throw new RuntimeException("Board.getValue: pX must be a Value between 0 and " + (aWidth - 1) + ": " + pX);
        if(pY < 0 || pY >= aHeight)
            throw new RuntimeException("Board.getValue: pY must be a Value between 0 and " + (aHeight - 1) + ": " + pY);
        return aBoard[pX][pY].value;
    }
    
    public void setValue(int pX, int pY, int pValue) {
        if(pX < 0 || pX >= aWidth)
            throw new RuntimeException("Board.setValue: pX must be a Value between 0 and " + (aWidth - 1) + ": " + pX);
        if(pY < 0 || pY >= aHeight)
            throw new RuntimeException("Board.setValue: pY must be a Value between 0 and " + (aHeight - 1) + ": " + pY);
        if(pValue < VALUE_NULL || pValue > 4)
            throw new RuntimeException("Board.setValue: pValue must be a Value between -1 and 4: " + pValue);

        aBoard[pX][pY].value = pValue;
        aBoard[pX][pY].piece = 0;
        aBoard[pX][pY].risk = 0;
    }
    
    protected int getPiece(int pX, int pY) {
        if(pX < 0 || pX >= aWidth)
            throw new RuntimeException("Board.getPiece: pX must be a Value between 0 and " + (aWidth - 1) + ": " + pX);
        if(pY < 0 || pY >= aHeight)
            throw new RuntimeException("Board.getPiece: pY must be a Value between 0 and " + (aHeight - 1) + ": " + pY);
        return aBoard[pX][pY].piece;
    }
    
    protected void setPiece(int pX, int pY, int pPieceType) {
        if(pX < 0 || pX >= aWidth)
            throw new RuntimeException("Board.setPiece: pX must be a Value between 0 and " + (aWidth - 1) + ": " + pX);
        if(pY < 0 || pY >= aHeight)
            throw new RuntimeException("Board.setPiece: pY must be a Value between 0 and " + (aHeight - 1) + ": " + pY);

        if(aBoard[pX][pY].piece == pPieceType)
            return;

        if(aAutomatic) {
            if(aBoard[pX][pY].value != VALUE_NULL)
                throw new RuntimeException("Board.setPiece: A Position with a Hint Value cannot have a Piece.");
            if((pPieceType & PIECE_S) == 0 && (pPieceType & PIECE_C) == 0 && (pPieceType & PIECE_X) == 0 && pPieceType != PIECE_NULL)
                throw new RuntimeException("Board.setPiece: pPieceType has an Invalid Type: " + pPieceType);
            if(((pPieceType & PIECE_S) != 0? 1: 0) + ((pPieceType & PIECE_C) != 0? 1: 0) + ((pPieceType & PIECE_X) != 0? 1: 0) > 1)
                throw new RuntimeException("Board.setPiece: pPieceType cannot specify more than 1 Piece: " + pPieceType);
            if((aBoard[pX][pY].piece & PIECE_S) == 0 && (pPieceType & PIECE_S) != 0) {
                if(aSPieceCnt < aSPieceQty)
                    aSPieceCnt++;
                else
                    throw new RuntimeException("Board.setPiece: All S Pieces placed: " + aSPieceCnt);
            } else if((aBoard[pX][pY].piece & PIECE_S) != 0 && (pPieceType & PIECE_S) == 0) {
                if(aSPieceCnt > 0)
                    aSPieceCnt--;
                else
                    throw new RuntimeException("Board.setPiece: All S Pieces retired: " + aSPieceCnt);
            }
            if((aBoard[pX][pY].piece & PIECE_C) == 0 && (pPieceType & PIECE_C) != 0) {
                if(aCPieceCnt < aCPieceQty)
                    aCPieceCnt++;
                else
                    throw new RuntimeException("Board.setPiece: All C Pieces placed: " + aCPieceCnt);
            } else if((aBoard[pX][pY].piece & PIECE_C) != 0 && (pPieceType & PIECE_C) == 0) {
                if(aCPieceCnt > 0)
                    aCPieceCnt--;
                else
                    throw new RuntimeException("Board.setPiece: All C Pieces retired: " + aCPieceCnt);
            }
            if((aBoard[pX][pY].piece & PIECE_X) == 0 && (pPieceType & PIECE_X) != 0) {
                if(aXPieceCnt < aXPieceQty)
                    aXPieceCnt++;
                else
                    throw new RuntimeException("Board.setPiece: All X Pieces placed: " + aXPieceCnt);
            } else if((aBoard[pX][pY].piece & PIECE_X) != 0 && (pPieceType & PIECE_X) == 0) {
                if(aXPieceCnt > 0)
                    aXPieceCnt--;
                else
                    throw new RuntimeException("Board.setPiece: All X Pieces retired: " + aXPieceCnt);
            }
            
            // Special condition that considers when an X is on one corner of the board.
            if(((pPieceType & PIECE_X) != 0 && (aBoard[pX][pY].piece & PIECE_X) == 0) || ((pPieceType & PIECE_X) == 0 && (aBoard[pX][pY].piece & PIECE_X) != 0)) {
                boolean vAdd = (pPieceType & PIECE_X) != 0 && (aBoard[pX][pY].piece & PIECE_X) == 0;
                int vEX = aWidth - 1;
                int vEY = aHeight - 1;
                if(pX == 0 && pY == 0) {
                    if(vAdd)
                        aBoard[vEX][vEY].piece |= PIECE_X;
                    else
                        aBoard[vEX][vEY].piece &= ~PIECE_X;
                } else if(pX == vEX && pY == 0) {
                    if(vAdd)
                        aBoard[0][vEY].piece |= PIECE_X;
                    else
                        aBoard[0][vEY].piece &= ~PIECE_X;
                } else if(pX == 0 && pY == vEY) {
                    if(vAdd)
                        aBoard[vEX][0].piece |= PIECE_X;
                    else
                        aBoard[vEX][0].piece &= ~PIECE_X;
                } else if(pX == vEX && pY == vEY) {
                    if(vAdd)
                        aBoard[0][0].piece |= PIECE_X;
                    else
                        aBoard[0][0].piece &= ~PIECE_X;
                }
            }
            
            if((pPieceType & PIECE_S) != 0 && (aBoard[pX][pY].piece & PIECE_S) == 0)
                transform(DIR_H | DIR_V | DIR_DU | DIR_DD, pX, pY, true);
            else
                if((aBoard[pX][pY].piece & PIECE_S) != 0)
                    transform(DIR_H | DIR_V | DIR_DU | DIR_DD, pX, pY, false);
            
            if((pPieceType & PIECE_C) != 0 && (aBoard[pX][pY].piece & PIECE_C) == 0)
                transform(DIR_H | DIR_V, pX, pY, true);
            else
                if((aBoard[pX][pY].piece & Board.PIECE_C) != 0)
                    transform(DIR_H | DIR_V, pX, pY, false);
            
            if((pPieceType & PIECE_X) != 0 && (aBoard[pX][pY].piece & PIECE_X) == 0)
                transform(DIR_DU | DIR_DD, pX, pY, true);
            else
                if((aBoard[pX][pY].piece & Board.PIECE_X) != 0)
                    transform(DIR_DU | DIR_DD, pX, pY, false);
        }

        aBoard[pX][pY].piece = pPieceType;
    }
    
    protected int getRisk(int pX, int pY) {
        if(pX < 0 || pX >= aWidth)
            throw new RuntimeException("Board.getRisk: pX must be a Value between 0 and " + (aWidth - 1) + ": " + pX);
        if(pY < 0 || pY >= aHeight)
            throw new RuntimeException("Board.getRisk: pY must be a Value between 0 and " + (aHeight - 1) + ": " + pY);
        return aBoard[pX][pY].risk;
    }
    
    protected void setRisk(int pX, int pY, int pRisk) {
        if(aAutomatic)
            throw new RuntimeException("Board.setRisk: Risk cannot be set when the Board is in Automatic Mode.");
        if(pX < 0 || pX >= aWidth)
            throw new RuntimeException("Board.setRisk: pX must be a Value between 0 and " + (aWidth - 1) + ": " + pX);
        if(pY < 0 || pY >= aHeight)
            throw new RuntimeException("Board.setRisk: pY must be a Value between 0 and " + (aHeight - 1) + ": " + pY);
        if(pRisk != RISK_NULL && (pRisk & RISK_0) == 0 && (pRisk & RISK_1) == 0 && (pRisk & RISK_2) == 0 && (pRisk & RISK_3) == 0 && (pRisk & RISK_4) == 0)
            throw new RuntimeException("Board.setRisk: Invalid Risks: (" + pX + ", " + pY + ") = " + pRisk);
        aBoard[pX][pY].risk = pRisk;
    }
    
    // Here RISK_1 is used instead of RISK_0 to solve the problem of testing a solution that has Xs on the corners.
    protected int addLowerRisk(int pRisk, boolean vApplyDefaultRisk) {
        if(pRisk == 0)
            return vApplyDefaultRisk? aDefaultRisk: RISK_1;
        
        int vRisk = 0;
        if((pRisk & RISK_1) != 0) {
            vRisk = pRisk & ~RISK_1;
            return vRisk != 0? vRisk: RISK_2;
        }
        if((pRisk & RISK_2) != 0) {
            vRisk = pRisk & ~RISK_2;
            return vRisk != 0? vRisk: RISK_3;
        }
        if((pRisk & RISK_3) != 0) {
            vRisk = pRisk & ~RISK_3;
            return vRisk != 0? vRisk: RISK_4;
        }
        if((pRisk & RISK_4) != 0) {
            vRisk = pRisk & ~RISK_4;
            return vRisk != 0? vRisk: RISK_E5;
        }
        if((pRisk & RISK_E5) != 0) {
            vRisk = pRisk & ~RISK_E5;
            return vRisk != 0? vRisk: RISK_E6;
        }
        if((pRisk & RISK_E6) != 0) {
            vRisk = pRisk & ~RISK_E6;
            return vRisk != 0? vRisk: RISK_E7;
        }
        if((pRisk & RISK_E7) != 0) {
            vRisk = pRisk & ~RISK_E7;
            return vRisk != 0? vRisk: RISK_E8;
        }
        if((pRisk & RISK_E8) != 0) {
            vRisk = pRisk & ~RISK_E8;
            return vRisk != 0? vRisk: RISK_E9;
        }
        throw new RuntimeException("Board.addLowerRisk: Error Adding at Lower Risk.");
    }

    protected int addUpperRisk(int pRisk, boolean vApplyDefaultRisk) {
        if(pRisk == 0)
            return vApplyDefaultRisk? aDefaultRisk: RISK_1;
        
        if((pRisk & RISK_E9) != 0)
            return pRisk;
        if((pRisk & RISK_E8) != 0)
            return 8 < aPieceQty? pRisk | RISK_E9: pRisk;
        if((pRisk & RISK_E7) != 0)
            return 7 < aPieceQty? pRisk | RISK_E8: pRisk;
        if((pRisk & RISK_E6) != 0)
            return 6 < aPieceQty? pRisk | RISK_E7: pRisk;
        if((pRisk & RISK_E5) != 0)
            return 5 < aPieceQty? pRisk | RISK_E6: pRisk;
        if((pRisk & RISK_4) != 0)
            return 4 < aPieceQty? pRisk | RISK_E5: pRisk;
        if((pRisk & RISK_3) != 0)
            return 3 < aPieceQty? pRisk | RISK_4: pRisk;
        if((pRisk & RISK_2) != 0)
            return 2 < aPieceQty? pRisk | RISK_3: pRisk;
        if((pRisk & RISK_1) != 0)
            return 1 < aPieceQty? pRisk | RISK_2: pRisk;
        throw new RuntimeException("Board.addUpperRisk: Error Adding at Upper Risk.");
    }

    // Here RISK_1 is used instead of RISK_0 to solve the problem of test a solution that has Xs on the corners.
    protected int subtractLowerRisk(int pRisk, boolean vApplyDefaultRisk) {
        if(pRisk == 0)
            throw new RuntimeException("Board.subtractLowerRisk: Error Subtracting at Lower Risk.");
        if(pRisk == aDefaultRisk || pRisk == RISK_1)
            return 0;

        int vRisk = 0;
        if((pRisk & RISK_E9) != 0 && (pRisk & RISK_E8) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~(RISK_E7 + RISK_E6 + RISK_E5 + RISK_4 + RISK_3 + RISK_2 + RISK_1) | (aPieceQty == 9? RISK_E9: 0);
            return vRisk | RISK_E8;
        }
        if((pRisk & RISK_E8) != 0 && (pRisk & RISK_E7) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~(RISK_E6 + RISK_E5 + RISK_4 + RISK_3 + RISK_2 + RISK_1) | (aPieceQty == 8? RISK_E8: 0);
            return vRisk | RISK_E7;
        }
        if((pRisk & RISK_E7) != 0 && (pRisk & RISK_E6) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~(RISK_E5 + RISK_4 + RISK_3 + RISK_2 + RISK_1) | (aPieceQty == 7? RISK_E7: 0);
            return vRisk | RISK_E6;
        }
        if((pRisk & RISK_E6) != 0 && (pRisk & RISK_E5) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~(RISK_4 + RISK_3 + RISK_2 + RISK_1) | (aPieceQty == 6? RISK_E6: 0);
            return vRisk | RISK_E5;
        }
        if((pRisk & RISK_E5) != 0 && (pRisk & RISK_4) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~(RISK_3 + RISK_2 + RISK_1) | (aPieceQty == 5? RISK_E5: 0);
            return vRisk | RISK_4;
        }
        if((pRisk & RISK_4) != 0 && (pRisk & RISK_3) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~(RISK_2 + RISK_1) | (aPieceQty == 4? RISK_4: 0);
            return vRisk | RISK_3;
        }
        if((pRisk & RISK_3) != 0 && (pRisk & RISK_2) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk & ~RISK_1 | (aPieceQty == 3? RISK_3: 0);
            return vRisk | RISK_2;
        }
        if((pRisk & RISK_2) != 0 && (pRisk & RISK_1) == 0) {
            if(vApplyDefaultRisk)
                vRisk = aDefaultRisk | (aPieceQty == 2? RISK_2: 0);
            return vRisk | RISK_1;
        }

        return vApplyDefaultRisk? aDefaultRisk: RISK_1;
    }

    protected int subtractUpperRisk(int pRisk, boolean vApplyDefaultRisk) {
        if(pRisk == 0)
            throw new RuntimeException("Board.subtractUpperRisk: Error Subtracting at Upper Risk.");
        if(aPieceCnt < aPieceQty - 9)
            return pRisk;
        
        if((pRisk & RISK_E9) != 0)
            return pRisk != RISK_E9? pRisk & ~RISK_E9: pRisk;
        if((pRisk & RISK_E8) != 0)
            return pRisk != RISK_E8? pRisk & ~RISK_E8: pRisk;
        if((pRisk & RISK_E7) != 0)
            return pRisk != RISK_E7? pRisk & ~RISK_E7: pRisk;
        if((pRisk & RISK_E6) != 0)
            return pRisk != RISK_E6? pRisk & ~RISK_E6: pRisk;
        if((pRisk & RISK_E5) != 0)
            return pRisk != RISK_E5? pRisk & ~RISK_E5: pRisk;
        if((pRisk & RISK_4) != 0)
            return pRisk != RISK_4? pRisk & ~RISK_4: pRisk;
        if((pRisk & RISK_3) != 0)
            return pRisk != RISK_3? pRisk & ~RISK_3: pRisk;
        if((pRisk & RISK_2) != 0)
            return pRisk != RISK_2? pRisk & ~RISK_2: pRisk;
        if((pRisk & RISK_1) != 0)
            return pRisk != RISK_1? pRisk & ~RISK_1: pRisk;

        return vApplyDefaultRisk? aDefaultRisk: RISK_1;
    }

    protected void transform(int pDir, int pCenterX, int pCenterY, boolean pApply) {
        int vCenterRisk = aBoard[pCenterX][pCenterY].risk;
        int vTmpRisk = 0;

        for(int i = 0; i < aWidth; i++)
            for(int j = 0; j < aHeight; j++)
                if(aBoard[i][j].risk != 0)
                    aBoard[i][j].risk = pApply? subtractUpperRisk(aBoard[i][j].risk, true): addUpperRisk(aBoard[i][j].risk, true);
        if(aPieceCnt != 0)
            aDefaultRisk = pApply? subtractUpperRisk(aDefaultRisk, true): addUpperRisk(aDefaultRisk, true);
        aPieceCnt += pApply? 1: -1;
        
        if((pDir & DIR_H) != 0)
            for(int i = 0; i < aWidth; i++)
                if(i != pCenterX)
                    if(pApply) {
                        vTmpRisk = addLowerRisk(aBoard[i][pCenterY].risk, true);
                        if(aPieceCnt != aPieceQty && aBoard[i][pCenterY].risk != 0)
                            vTmpRisk = addUpperRisk(vTmpRisk, true);
                        aBoard[i][pCenterY].risk = vTmpRisk;
                    } else
                        aBoard[i][pCenterY].risk = subtractLowerRisk(aBoard[i][pCenterY].risk, true);
        
        if((pDir & DIR_V) != 0)
            for(int j = 0; j < aHeight; j++)
                if(j != pCenterY)
                    if(pApply) {
                        vTmpRisk = addLowerRisk(aBoard[pCenterX][j].risk, true);
                        if(aPieceCnt != aPieceQty && aBoard[pCenterX][j].risk != 0)
                            vTmpRisk = addUpperRisk(vTmpRisk, true);
                        aBoard[pCenterX][j].risk = vTmpRisk;
                    } else
                        aBoard[pCenterX][j].risk = subtractLowerRisk(aBoard[pCenterX][j].risk, true);
        
        if((pDir & DIR_DU) != 0) {
            int vTmpX = -1;
            int vTmpY = -1;
            if(pCenterX >= pCenterY) {
                vTmpX = pCenterX - pCenterY;
                vTmpY = 0;
            } else {
                vTmpX = 0;
                vTmpY = pCenterY - pCenterX;
            }
            for(int i = vTmpX, j = vTmpY; i < aWidth && j < aHeight; i++, j++)
                if(i != pCenterX && j != pCenterY)
                    if(pApply) {
                        vTmpRisk = addLowerRisk(aBoard[i][j].risk, true);
                        if(aPieceCnt != aPieceQty && aBoard[i][j].risk != 0)
                            vTmpRisk = addUpperRisk(vTmpRisk, true);
                        aBoard[i][j].risk = vTmpRisk;
                    } else
                        aBoard[i][j].risk = subtractLowerRisk(aBoard[i][j].risk, true);
        }
        
        if((pDir & DIR_DD) != 0) {
            int vTmpX = pCenterX - aHeight + pCenterY + 1;
            int vTmpY = -1;
            if(vTmpX < 0) {
                vTmpX = 0;
                vTmpY = pCenterY + pCenterX;
            } else
                vTmpY = aHeight - 1;
            for(int i = vTmpX, j = vTmpY; i < aWidth && j >= 0; i++, j--)
                if(i != pCenterX && j != pCenterY)
                    if(pApply) {
                        vTmpRisk = addLowerRisk(aBoard[i][j].risk, true);
                        if(aPieceCnt != aPieceQty && aBoard[i][j].risk != 0)
                            vTmpRisk = addUpperRisk(vTmpRisk, true);
                        aBoard[i][j].risk = vTmpRisk;
                    } else
                        aBoard[i][j].risk = subtractLowerRisk(aBoard[i][j].risk, true);
        }
        
        if(pApply) {
            vTmpRisk = addLowerRisk(aBoard[pCenterX][pCenterY].risk, true);
            if(aPieceCnt != aPieceQty && aBoard[pCenterX][pCenterY].risk != 0)
                vTmpRisk = addUpperRisk(vTmpRisk, true);
            aBoard[pCenterX][pCenterY].risk = vTmpRisk;
        } else
            aBoard[pCenterX][pCenterY].risk = subtractLowerRisk(aBoard[pCenterX][pCenterY].risk, true);
    }
    
    protected String print(int pDetailLevel, boolean vShowOnlyHints, boolean vHideEntries) {
        int vEX = aWidth - 1;
        int vEY = aHeight - 1;
        
        String vInxXFormat = null;
        switch(pDetailLevel) {
            case DETAIL_LEVEL_1:
                vInxXFormat = " %02d ";
                break;
            case DETAIL_LEVEL_2:
                vInxXFormat = "         %02d ";
                break;
            default:
                vInxXFormat = "                  %02d ";
        }
        String vOutput = "   ";
        for(int i = 0; i < aWidth; i++)
            vOutput += String.format(vInxXFormat, i);
        
        vOutput += "\n";
        for(int j = 0; j < aHeight; j++) {
            vOutput += String.format("%02d ", j);
            for(int i = 0; i < aWidth; i++) {
                String vRisks = "";
                String vPiece = "";
                String vValue = "";
                String vUnifiedMarks = "";
                if(pDetailLevel != DETAIL_LEVEL_1) {
                    if(!vHideEntries) {
                        int vRisk = 0;
                        if(pDetailLevel == DETAIL_LEVEL_3 || (aBoard[i][j].risk != 0 && aBoard[i][j].piece == 0 && aBoard[i][j].value < aFilter)) {
                            vRisk = aBoard[i][j].risk;
                            if((vRisk & RISK_0) != 0)
                                vRisks += "0";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_1) != 0)
                                vRisks += "1";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_2) != 0)
                                vRisks += "2";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_3) != 0)
                                vRisks += "3";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_4) != 0)
                                vRisks += "4";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_E5) != 0)
                                vRisks += "5";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_E6) != 0)
                                vRisks += "6";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_E7) != 0)
                                vRisks += "7";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_E8) != 0)
                                vRisks += "8";
                            else
                                vRisks += "-";
                            if((vRisk & RISK_E9) != 0)
                                vRisks += "9";
                            else
                                vRisks += "-";
                        }
                        if(pDetailLevel == DETAIL_LEVEL_2 && !vRisks.equals(""))
                            vUnifiedMarks = vRisks;
                        
                        if(pDetailLevel == DETAIL_LEVEL_3 || (aBoard[i][j].piece != 0 && aBoard[i][j].value < aFilter)) {
                            if((aBoard[i][j].piece & PIECE_S) != 0)
                                vPiece += "s";
                            else
                                vPiece += "-";
                            if((aBoard[i][j].piece & PIECE_C) != 0)
                                vPiece += "c";
                            else
                                vPiece += "-";
                            if((aBoard[i][j].piece & PIECE_X) != 0)
                                vPiece += (i == 0 && j == 0) || (i == vEX && j == vEY) || (i == 0 && j == vEY) || (i == vEX && j == 0)? "X": "x"; // Includes this special condition when there is an X on one corner of the board.
                            else
                                vPiece += "-";
                        }
                        if(pDetailLevel == DETAIL_LEVEL_2 && !vPiece.equals(""))
                            vUnifiedMarks = vPiece + "-------";
                    } else {
                        vRisks = "---------";
                        vPiece = "---";
                        vUnifiedMarks = "---------";
                    }
                    
                    if(pDetailLevel == DETAIL_LEVEL_3)
                        vValue = aBoard[i][j].value >= aFilter || !vShowOnlyHints? String.valueOf(aBoard[i][j].value): "-";
                    else {
                        if((aBoard[i][j].value >= aFilter || !vShowOnlyHints) && aBoard[i][j].value == 0)
                            vUnifiedMarks += "0";
                        else
                            vUnifiedMarks += "-";
                        if((aBoard[i][j].value >= aFilter || !vShowOnlyHints) && aBoard[i][j].value == 1)
                            vUnifiedMarks += "1";
                        else
                            vUnifiedMarks += "-";
                        if((aBoard[i][j].value >= aFilter || !vShowOnlyHints) && aBoard[i][j].value == 2)
                            vUnifiedMarks += "2";
                        else
                            vUnifiedMarks += "-";
                        if((aBoard[i][j].value >= aFilter || !vShowOnlyHints) && aBoard[i][j].value == 3)
                            vUnifiedMarks += "3";
                        else
                            vUnifiedMarks += "-";
                        if((aBoard[i][j].value >= aFilter || !vShowOnlyHints) && aBoard[i][j].value == 4)
                            vUnifiedMarks += "4";
                        else
                            vUnifiedMarks += "-";
                        vUnifiedMarks += "-----";
                    }
                    if(pDetailLevel == DETAIL_LEVEL_2 && vUnifiedMarks.equals(""))
                        vUnifiedMarks = "----------";
                } else {
                    if(!vHideEntries) {
                        if(aBoard[i][j].risk != 0 && aBoard[i][j].piece == 0 && aBoard[i][j].value < aFilter) {
                            int vRisk = aBoard[i][j].risk;
                            if((vRisk & RISK_0) != 0)
                                vValue = "0";
                            else if((vRisk & RISK_1) != 0)
                                vValue = "1";
                            else if((vRisk & RISK_2) != 0)
                                vValue = "2";
                            else if((vRisk & RISK_3) != 0)
                                vValue = "3";
                            else if((vRisk & RISK_4) != 0)
                                vValue = "4";
                            else if((vRisk & RISK_E5) != 0)
                                vValue = "5";
                            else if((vRisk & RISK_E6) != 0)
                                vValue = "6";
                            else if((vRisk & RISK_E7) != 0)
                                vValue = "7";
                            else if((vRisk & RISK_E8) != 0)
                                vValue = "8";
                            else
                                vValue = "9";
                        }
                        
                        if(aBoard[i][j].piece != 0 && aBoard[i][j].value < aFilter)
                            switch(aBoard[i][j].piece) {
                                case PIECE_S:
                                    vValue = "s";
                                    break;
                                case PIECE_C:
                                    vValue = "c";
                                    break;
                                case PIECE_X:
                                    vValue = (i == 0 && j == 0) || (i == vEX && j == vEY) || (i == 0 && j == vEY) || (i == vEX && j == 0)? "X": "x"; // Includes this special condition when there is an X on one corner of the board.
                                    break;
                                default:
                                    vValue = "*";
                            }
                    } else
                        vValue = "-";
                    
                    if(aBoard[i][j].value >= aFilter || !vShowOnlyHints) {
                        switch(aBoard[i][j].value) {
                            case 0:
                                vValue = "0";
                                break;
                            case 1:
                                vValue = "1";
                                break;
                            case 2:
                                vValue = "2";
                                break;
                            case 3:
                                vValue = "3";
                                break;
                            default:
                                vValue = "4";
                        }
                    }
                    vValue += (vValue.equals("")? "-": "") + " ";
                }
                
                switch(pDetailLevel) {
                    case DETAIL_LEVEL_3:
                        vOutput += "[" + vValue + ": " + vPiece + " | " + vRisks + "]";
                        break;
                    case DETAIL_LEVEL_2:
                        vOutput += "[" + vUnifiedMarks + "]";
                        break;
                    default:
                        vOutput += "[" + vValue + "]";
                }
            }
            vOutput += "\n";
        }
        return vOutput;
    }
    
    public String toString() {
        return print(DETAIL_LEVEL_3, true, false);
    }
}