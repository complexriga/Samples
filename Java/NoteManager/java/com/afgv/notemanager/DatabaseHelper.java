package com.afgv.notemanager;

import android.content.Context;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase;

public class DatabaseHelper extends SQLiteOpenHelper {
	
	private static final int DATABASE_VERSION = 2;
	private static final String DATABASE_NAME = "Notes.db";
	private static final String NOTES_TABLE = "notes";
	private static final String CREATION_STATEMENT = "CREATE TABLE " + NOTES_TABLE + " (note_id INTEGER, note_text TEXT, note_date TEXT)";
		
	public DatabaseHelper(Context opCtx){
		super(opCtx, DATABASE_NAME, null, DATABASE_VERSION);
	}
	
	@Override
	public void onCreate(SQLiteDatabase opDB) {
		opDB.execSQL(CREATION_STATEMENT);
	}

	@Override
	public void onUpgrade(SQLiteDatabase opDB, int ipOldVer, int ipNewVer) {
		// Esta Base de Datos no cambia cuando se actualiza la Version de Base de Datos. 
	}
}
