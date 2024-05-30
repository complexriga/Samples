package com.solo.cabe;

import java.util.StringTokenizer;
import java.io.StringWriter;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Engine {
    static Caber aPuzzle;
    
    public static boolean IsInt(String pStr)
    {
        if (pStr == null) {
                return false;
        }
        int vLen = pStr.length();
        if (vLen == 0) {
                return false;
        }
        int i = 0;
        if (pStr.charAt(0) == '-') {
                if (vLen == 1) {
                        return false;
                }
                i = 1;
        }
        for (; i < vLen; i++) {
                char c = pStr.charAt(i);
                if (c <= '/' || c >= ':') {
                        return false;
                }
        }
        return true;
    }

    public static void main(String[] pArgs) {
        int vDefaultRisk = Board.RISK_0 + Board.RISK_1 + Board.RISK_2 + Board.RISK_3 + Board.RISK_4;
        
        try {
            int vWidth = 0;
            int vHeight = 0;
            
            System.out.println("CABER Game Engine v1.0");
            BufferedReader vInput = new BufferedReader(new InputStreamReader(System.in));
            boolean vExternalInd = true;
            while(vExternalInd) {
                System.out.print("> ");
                StringTokenizer vTokens = new StringTokenizer(vInput.readLine());
                boolean vInternalInd = true;
                while(vInternalInd && vTokens.hasMoreTokens()) {
                    String vToken = vTokens.nextToken();
                    switch(vToken) {
                        case "g": //Generate Board.
                            try {
                                int vFilter = 0;
                                int vPieceQty = 0;
                                String vCmdMsg = "Error in Sintax: g Width Height Filter QuantityOfStars QuantityOfCrosses QuantityOfXs IsAutomatic";
                                
                                if(!vTokens.hasMoreTokens())
                                    throw new RuntimeException(vCmdMsg);
                                vToken = vTokens.nextToken();
                                if(!IsInt(vToken))
                                    throw new RuntimeException("Width must be an Integer.");
                                vWidth = Integer.parseInt(vToken);

                                if(!vTokens.hasMoreTokens())
                                    throw new RuntimeException(vCmdMsg);
                                vToken = vTokens.nextToken();
                                if(!IsInt(vToken))
                                    throw new RuntimeException("Height must be an Integer.");
                                vHeight = Integer.parseInt(vToken);

                                if(!vTokens.hasMoreTokens())
                                    throw new RuntimeException(vCmdMsg);
                                vToken = vTokens.nextToken();
                                if(!IsInt(vToken))
                                    throw new RuntimeException("Filter must be an Integer.");
                                vFilter = Integer.parseInt(vToken);

                                int[] vNumberOfPieces = new int[3];
                                for(int i = 0; i < vNumberOfPieces.length; i++) {
                                    if(!vTokens.hasMoreTokens())
                                        throw new RuntimeException(vCmdMsg);
                                    vToken = vTokens.nextToken();
                                    if(!IsInt(vToken))
                                        throw new RuntimeException("NumberOfPieces must be an Integer.");
                                    vNumberOfPieces[i] = Integer.parseInt(vToken);
                                }

                                if(!vTokens.hasMoreTokens())
                                    throw new RuntimeException(vCmdMsg);
                                vToken = vTokens.nextToken();
                                if(!vToken.equals("y") && !vToken.equals("n"))
                                    throw new RuntimeException("IsAutomatic must be y or n.");
                                boolean vIsAutomatic = vToken.equals("y");

                                aPuzzle = new Caber(vWidth, vHeight, vFilter, vNumberOfPieces[0], vNumberOfPieces[1], vNumberOfPieces[2], vIsAutomatic);
                                System.out.println(aPuzzle.getBoard().print(Board.DETAIL_LEVEL_1, true, false));
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "a": //Show/Set Automatic Mode.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");
                                
                                boolean vIsAutomatic = true;
                                if(vTokens.hasMoreTokens()) {
                                    vToken = vTokens.nextToken();
                                    if(!vToken.equals("y") && !vToken.equals("n"))
                                        throw new RuntimeException("IsAutomatic must be y or n.");
                                    aPuzzle.setAutomatic(vToken.equals("y"));
                                } else
                                    System.out.println(aPuzzle.isAutomatic()? "y": "n");
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "m": //Mark Position.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");
                                
                                if(aPuzzle.isAutomatic()) {
                                    int vX = 0;
                                    int vY = 0;
                                    String vOption = null;
                                    String vCmdMsg = "Error in Sintax (Automatic Mode): m X Y [s|c|x|-]";

                                    if(!vTokens.hasMoreTokens())
                                        throw new RuntimeException(vCmdMsg);
                                    vToken = vTokens.nextToken();
                                    if(!IsInt(vToken))
                                        throw new RuntimeException("X must be an Integer.");
                                    vX = Integer.parseInt(vToken);
                                    if(vX < 0 || vX >= vWidth)
                                        throw new RuntimeException("X must be a Value between 0 and " + (vWidth - 1) + ".");

                                    if(!vTokens.hasMoreTokens())
                                        throw new RuntimeException(vCmdMsg);
                                    vToken = vTokens.nextToken();
                                    if(!IsInt(vToken))
                                        throw new RuntimeException("Y must be an Integer.");
                                    vY = Integer.parseInt(vToken);
                                    if(vY < 0 || vY >= vHeight)
                                        throw new RuntimeException("Y must be a Value between 0 and " + (vHeight - 1) + ".");
                                    
                                    int vActualMarks = -1;
                                    if(vTokens.hasMoreTokens()) {
                                        vToken = vTokens.nextToken();
                                        if(vToken.length() != 1)
                                            throw new RuntimeException("Invalid Entry: " + vToken);
                                        
                                        boolean vError = false;
                                        boolean vIsAdd = true;
                                        switch(vToken) {
                                            case "s":
                                                vActualMarks = Board.PIECE_S;
                                                break;
                                            case "c":
                                                vActualMarks = Board.PIECE_C;
                                                break;
                                            case "x":
                                                vActualMarks = Board.PIECE_X;
                                                break;
                                            case "-":
                                                vActualMarks = Board.PIECE_NULL;
                                                break;
                                            default:
                                                throw new RuntimeException("Invalid Piece: " + vToken);
                                        }
                                    } else
                                        throw new RuntimeException(vCmdMsg);
                                    
                                    try {
                                        aPuzzle.setPiece(vX, vY, vActualMarks);
                                    } catch(Exception e) {
                                        System.out.println(e.getMessage().substring(e.getMessage().indexOf(":") + 1).trim());
                                    }
                                } else {
                                    int vX = 0;
                                    int vY = 0;
                                    String vOption = null;
                                    String vCmdMsg = "Error in Sintax (Manual Mode): m X Y [r [[[[+|-][0|1|2|3|4]] ...][-]]][p [[[[+|-][s|c|x]] ...][-]]]";

                                    if(!vTokens.hasMoreTokens())
                                        throw new RuntimeException(vCmdMsg);
                                    vToken = vTokens.nextToken();
                                    if(!IsInt(vToken))
                                        throw new RuntimeException("X must be an Integer.");
                                    vX = Integer.parseInt(vToken);
                                    if(vX < 0 || vX >= vWidth)
                                        throw new RuntimeException("X must be a Value between 0 and " + (vWidth - 1) + ".");

                                    if(!vTokens.hasMoreTokens())
                                        throw new RuntimeException(vCmdMsg);
                                    vToken = vTokens.nextToken();
                                    if(!IsInt(vToken))
                                        throw new RuntimeException("Y must be an Integer.");
                                    vY = Integer.parseInt(vToken);
                                    if(vY < 0 || vY >= vHeight)
                                        throw new RuntimeException("Y must be a Value between 0 and " + (vHeight - 1) + ".");
                                    
                                    if(!vTokens.hasMoreTokens())
                                        throw new RuntimeException(vCmdMsg);
                                    vToken = vTokens.nextToken();
                                    if(!vToken.equals("r") && !vToken.equals("p"))
                                        throw new RuntimeException("Option must be an r or p.");
                                    vOption = vToken;
                                    
                                    boolean vEraseAll = false;
                                    int vCnt = 0;
                                    int vActualMarks = vOption.equals("r")? aPuzzle.getRisk(vX, vY): aPuzzle.getPiece(vX, vY);
                                    while(vTokens.hasMoreTokens()) {
                                        vToken = vTokens.nextToken();
                                        if(vToken.length() > 2)
                                            throw new RuntimeException("Invalid Risk/Piece: " + vToken);
                                        
                                        boolean vError = false;
                                        boolean vIsAdd = true;
                                        String vAction = vToken.substring(0, 1);
                                        switch(vAction) {
                                            case "+":
                                                if(vToken.length() != 2) {
                                                    vError = true;
                                                    break;
                                                }
                                                vIsAdd = true;
                                                break;
                                            case "-":
                                                if(vToken.length() > 2) {
                                                    vError = true;
                                                    break;
                                                }
                                                if(vToken.length() == 2)
                                                    vIsAdd = false;
                                                else {
                                                    vActualMarks = 0;
                                                    vEraseAll = true;
                                                    continue;
                                                }
                                                break;
                                            default:
                                                vError = true;
                                        }
                                        if(vError)
                                            throw new RuntimeException("Invalid Action: " + vAction);
                                        
                                        String vMark = vToken.substring(1);
                                        if(vOption.equals("r")) {
                                            switch(vMark) {
                                                case "0":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.RISK_0;
                                                    else
                                                        vActualMarks &= ~Board.RISK_0;
                                                    break;
                                                case "1":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.RISK_1;
                                                    else
                                                        vActualMarks &= ~Board.RISK_1;
                                                    break;
                                                case "2":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.RISK_2;
                                                    else
                                                        vActualMarks &= ~Board.RISK_2;
                                                    break;
                                                case "3":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.RISK_3;
                                                    else
                                                        vActualMarks &= ~Board.RISK_3;
                                                    break;
                                                case "4":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.RISK_4;
                                                    else
                                                        vActualMarks &= ~Board.RISK_4;
                                                    break;
                                                default:
                                                    throw new RuntimeException("Invalid Risk: " + vMark);
                                            }
                                        } else {
                                            switch(vMark) {
                                                case "s":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.PIECE_S;
                                                    else
                                                        vActualMarks &= ~Board.PIECE_S;
                                                    break;
                                                case "c":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.PIECE_C;
                                                    else
                                                        vActualMarks &= ~Board.PIECE_C;
                                                    break;
                                                case "x":
                                                    if(vIsAdd)
                                                        vActualMarks |= Board.PIECE_X;
                                                    else
                                                        vActualMarks &= ~Board.PIECE_X;
                                                    break;
                                                default:
                                                    throw new RuntimeException("Invalid Piece: " + vMark);
                                            }
                                        }
                                        
                                        if(vEraseAll && vCnt != 0)
                                            throw new RuntimeException("Cannot erase Marks at the Same Time when Marks are given.");
                                        else
                                            vCnt++;
                                    }
                                    try {
                                        if(vOption.equals("r"))
                                            aPuzzle.setRisk(vX, vY, vCnt != 0? vActualMarks: vDefaultRisk);
                                        else
                                            aPuzzle.setPiece(vX, vY, vActualMarks);
                                    } catch(Exception e) {
                                        System.out.println(e.getMessage().substring(15));
                                    }
                                }
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "s": //Show Solution.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");

                                Caber.Piece[] vPieces = aPuzzle.getSolution();
                                if(vPieces.length == 0)
                                    throw new RuntimeException("No Pieces.");
                                
                                for(int k = 0; k < vPieces.length; k++) {
                                    String vType = "";
                                    switch(vPieces[k].type) {
                                        case Board.PIECE_S:
                                            vType = "s";
                                            break;
                                        case Board.PIECE_C:
                                            vType = "c";
                                            break;
                                        case Board.PIECE_X:
                                            vType = "x";
                                            break;
                                        default:
                                            vType = "*";
                                    }
                                    System.out.println("<" + vType + ", " + vPieces[k].pos.x + ", " + vPieces[k].pos.y + ">");
                                }
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "j": //Show Pieces.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");

                                Caber.Piece[] vPieces = aPuzzle.getTryPieces();
                                if(vPieces.length == 0)
                                    throw new RuntimeException("No Pieces.");
                                
                                for(int k = 0; k < vPieces.length; k++) {
                                    String vType = "";
                                    switch(vPieces[k].type) {
                                        case Board.PIECE_S:
                                            vType = "s";
                                            break;
                                        case Board.PIECE_C:
                                            vType = "c";
                                            break;
                                        case Board.PIECE_X:
                                            vType = "x";
                                            break;
                                        default:
                                            vType = "*";
                                    }
                                    System.out.println("<" + vType + ", " + vPieces[k].pos.x + ", " + vPieces[k].pos.y + ">");
                                }
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "c": //Show Clues.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");
                                
                                System.out.println("<S: " + String.format("%02d", aPuzzle.getSPieceQuantity()) + "|" + String.format("%02d", aPuzzle.getSPieceCount()) + ">");
                                System.out.println("<C: " + String.format("%02d", aPuzzle.getCPieceQuantity()) + "|" + String.format("%02d", aPuzzle.getCPieceCount()) + ">");
                                System.out.println("<X: " + String.format("%02d", aPuzzle.getXPieceQuantity()) + "|" + String.format("%02d", aPuzzle.getXPieceCount()) + ">");
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "t": //Test Solution.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");
                                System.out.println(aPuzzle.test()? "t": "f");
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "r": //Reset Board.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");

                                aPuzzle.reset();
                                System.out.println(aPuzzle.getBoard().print(Board.DETAIL_LEVEL_2, true, false));
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "p": //Print Board.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");
                                
                                String vCmdMsg = "Error in Sintax: p [Level [ShowOnlyHints [HideEntries]]]";
                                boolean vShowOnlyHints = true;
                                boolean vHideEntries = false;
                                int vDetailLevel = Board.DETAIL_LEVEL_2;
                                if(vTokens.hasMoreTokens()) {
                                    vToken = vTokens.nextToken();
                                    if(IsInt(vToken)) {
                                        vDetailLevel = Integer.parseInt(vToken);
                                        if(vDetailLevel < 0 || vDetailLevel > Board.DETAIL_LEVEL_3)
                                            throw new RuntimeException("Level must be 1, 2 or 3.");
                                        if(vTokens.hasMoreTokens()) {
                                            vToken = vTokens.nextToken();
                                            if(!vToken.equals("y") && !vToken.equals("n"))
                                                throw new RuntimeException("ShowOnlyHints must be y or n.");
                                            vShowOnlyHints = vToken.equals("y")? true: false;
                                            
                                            if(vTokens.hasMoreTokens()) {
                                                vToken = vTokens.nextToken();
                                                if(!vToken.equals("y") && !vToken.equals("n"))
                                                    throw new RuntimeException("HideEntries must be y or n.");
                                                vHideEntries = vToken.equals("y")? true: false;
                                            }
                                        }
                                    } else
                                        throw new RuntimeException("Level must an Integer.");
                                }
                                System.out.println(aPuzzle.getBoard().print(vDetailLevel, vShowOnlyHints, vHideEntries));
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "d": //Show Dimensions.
                            vInternalInd = false;
                            if(aPuzzle == null) {
                                System.out.println("No CABER.");
                                break;
                            }
                            System.out.println("<W: " + aPuzzle.getBoard().getWidth() + ">");
                            System.out.println("<H: " + aPuzzle.getBoard().getHeight() + ">");
                            break;
                        case "f": //Show Filter.
                            vInternalInd = false;
                            if(aPuzzle == null) {
                                System.out.println("No CABER.");
                                break;
                            }
                            System.out.println(aPuzzle.getFilter());
                            break;
                        case "k": //Show Statistics.
                            vInternalInd = false;
                            if(aPuzzle == null) {
                                System.out.println("No CABER.");
                                break;
                            }
                            System.out.println("Try Outs: " + aPuzzle.getTryOutCount());
                            System.out.println("Moves: " + aPuzzle.getMovementCount());
                            System.out.println("X Failed: " + aPuzzle.getXFailedCount());
                            System.out.println("0 Count: " + aPuzzle.get0Count());
                            System.out.println("1 Count: " + aPuzzle.get1Count());
                            System.out.println("2 Count: " + aPuzzle.get2Count());
                            System.out.println("3 Count: " + aPuzzle.get3Count());
                            System.out.println("4 Count: " + aPuzzle.get4Count());
                            break;
                        case "u": //Set Risk Template.
                            try {
                                if(aPuzzle == null)
                                    throw new RuntimeException("No CABER.");

                                String vCmdMsg = "Error in Sintax: u [[[[+|-][0|1|2|3|4]] ...][-]]";

                                boolean vEraseAll = false;
                                int vCnt = 0;
                                vDefaultRisk = 0;
                                while(vTokens.hasMoreTokens()) {
                                    vToken = vTokens.nextToken();
                                    if(vToken.length() > 2)
                                        throw new RuntimeException("Invalid Risk: " + vToken);
                                    
                                    boolean vError = false;
                                    boolean vIsAdd = true;
                                    String vAction = vToken.substring(0, 1);
                                    switch(vAction) {
                                        case "+":
                                            if(vToken.length() != 2) {
                                                vError = true;
                                                break;
                                            }
                                            vIsAdd = true;
                                            break;
                                        case "-":
                                            if(vToken.length() > 2) {
                                                vError = true;
                                                break;
                                            }
                                            if(vToken.length() == 2)
                                                vIsAdd = false;
                                            else {
                                                vDefaultRisk = 0;
                                                vEraseAll = true;
                                                continue;
                                            }
                                            break;
                                        default:
                                            vError = true;
                                    }
                                    if(vError)
                                        throw new RuntimeException("Invalid Action: " + vAction);
                                    
                                    String vMark = vToken.substring(1);
                                    switch(vMark) {
                                        case "0":
                                            if(vIsAdd)
                                                vDefaultRisk |= Board.RISK_0;
                                            else
                                                vDefaultRisk &= ~Board.RISK_0;
                                            break;
                                        case "1":
                                            if(vIsAdd)
                                                vDefaultRisk |= Board.RISK_1;
                                            else
                                                vDefaultRisk &= ~Board.RISK_1;
                                            break;
                                        case "2":
                                            if(vIsAdd)
                                                vDefaultRisk |= Board.RISK_2;
                                            else
                                                vDefaultRisk &= ~Board.RISK_2;
                                            break;
                                        case "3":
                                            if(vIsAdd)
                                                vDefaultRisk |= Board.RISK_3;
                                            else
                                                vDefaultRisk &= ~Board.RISK_3;
                                            break;
                                        case "4":
                                            if(vIsAdd)
                                                vDefaultRisk |= Board.RISK_4;
                                            else
                                                vDefaultRisk &= ~Board.RISK_4;
                                            break;
                                        default:
                                            throw new RuntimeException("Invalid Risk: " + vMark);
                                    }
                                    
                                    if(vEraseAll && vCnt != 0)
                                        throw new RuntimeException("Cannot erase Risks at the Same Time when Risks are given.");
                                    else
                                        vCnt++;
                                }
                            } catch(Exception e) {
                                System.out.println(e.getMessage());
                            }
                            vInternalInd = false;
                            break;
                        case "h": // Help.
                            {   
                                String vCommand = "";
                                if(vTokens.hasMoreTokens())
                                    vCommand = vTokens.nextToken();
                                boolean vInd = !vCommand.equals("");
                                switch(vCommand) {
                                    case "":
                                    case "g":
                                        System.out.println("Generate Board: g Width Height Filter QuantityOfStars QuantityOfCrosses QuantityOfXs IsAutomatic");
                                        if(vInd)
                                            break;
                                    case "a":
                                        System.out.println("Show/Set Automatic Mode: a [IsAutomatic]");
                                        if(vInd)
                                            break;
                                    case "m":
                                        System.out.println("Mark Position in Automatic Mode: m X Y [s|c|x|-]");
                                        System.out.println("Mark Position in Manual Mode: m X Y [r [[[[+|-][0|1|2|3|4]] ...][-]]][p [[[[+|-][s|c|x]] ...][-]]]");
                                        if(vInd)
                                            break;
                                    case "s":
                                        System.out.println("Show Solution: s");
                                        if(vInd)
                                            break;
                                    case "j":
                                        System.out.println("Show Played Pieces: j");
                                        if(vInd)
                                            break;
                                    case "c":
                                        System.out.println("Show Clues: c");
                                        if(vInd)
                                            break;
                                    case "t":
                                        System.out.println("Test Solution: t");
                                        if(vInd)
                                            break;
                                    case "r":
                                        System.out.println("Reset Board: r");
                                        if(vInd)
                                            break;
                                    case "p":
                                        System.out.println("Print Board: p [Level][OnlyHints]");
                                        if(vInd)
                                            break;
                                    case "d":
                                        System.out.println("Show Board Dimensions: d");
                                        if(vInd)
                                            break;
                                    case "f":
                                        System.out.println("Show Filter: f");
                                        if(vInd)
                                            break;
                                    case "k":
                                        System.out.println("Show Statistics: k");
                                        if(vInd)
                                            break;
                                     case "u":
                                        System.out.println("Set Risk Template: u [[[[+|-][0|1|2|3|4]] ...][-]]");
                                        if(vInd)
                                            break;
                                   case "h":
                                        System.out.println("Help: h [Command]");
                                        if(vInd)
                                            break;
                                    case "q":
                                        System.out.println("Quit: q");
                                        break;
                                    default:
                                        System.out.println("Not Help available for this Command: " + vCommand);
                                }
                            }
                            vInternalInd = false;
                            break;
                        case "q": //Quit.
                            vExternalInd = false;
                            break;
                        default:
                            System.out.println("Command not recognized.");
                            vInternalInd = false;
                    }
                }
                
            }

        } catch(Exception e) {
            StringWriter vWriter = new StringWriter();
            PrintWriter vPrintWriter = new PrintWriter(vWriter);
            e.printStackTrace(vPrintWriter);
            vPrintWriter.flush();
            System.out.println("ERROR: " + e.getMessage() + ":" + vWriter.toString());
        }
        return;
    }
}