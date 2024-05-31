package com.afgv.notemanager;

import android.os.Bundle;
import android.os.Environment;
import android.app.Activity;
import android.app.FragmentTransaction;
import android.app.Fragment;
import android.app.ActionBar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.inputmethod.InputMethodManager;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.Cursor;
import android.media.MediaScannerConnection;
import java.io.File;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class NoteActivity extends Activity implements NoteListFragment.NoteListFragmentCallback, NoteEditFragment.NoteFragmentCallback, LocalDialog.DialogFragmentCallback {

	private final static String BACKUP_FILE = "NotesBck.xml";
	private final static String ROOT_NODE = "LIST";
	private final static String ELEMENT_NODE = "NOTE";
	private final static String DATE_NODE = "DATE";
	private final static String TEXT_NODE = "TEXT";
	private final static String CANCEL_TAG = "CANCEL";
	private final static String IMPORT_TAG = "IMPORT";
	private final static String LAST_FRAGMENT = "LAST_FRAGMENT";
	private final static String ABOUT_TAG = "ABOUT";

	private DatabaseHelper ogHelper;
	private SQLiteDatabase ogDB;
	private Menu ogMenu;
	private String sgActualFragment = "";

	@Override
	public void onCreate(Bundle opSavedState) {
		
		super.onCreate(opSavedState);
		
		setContentView(R.layout.note_activity);
		ogHelper = new DatabaseHelper(getApplicationContext());

		if(opSavedState != null) {
			
			String slFragment = opSavedState.getString(LAST_FRAGMENT);
			
			if(NoteListFragment.FRAGMENT_TAG.equals(slFragment)) {
				
				getActionBar().setDisplayHomeAsUpEnabled(false);
				sgActualFragment = NoteListFragment.FRAGMENT_TAG;

			} else {
				
				getActionBar().setDisplayHomeAsUpEnabled(true);
				sgActualFragment = NoteEditFragment.FRAGMENT_TAG;

			}

		} else {
			
			getActionBar().setDisplayHomeAsUpEnabled(false);
			sgActualFragment = NoteListFragment.FRAGMENT_TAG;
			getFragmentManager().beginTransaction().add(R.id.fragment_container, new NoteListFragment(), NoteListFragment.FRAGMENT_TAG).commit();

		}
	
	}
	
	@Override
	public void onStart() {
		super.onStart();
		
		if(ogDB == null || !ogDB.isOpen())
			ogDB = ogHelper.getWritableDatabase();
	}
	
	@Override
	public void onStop() {
		super.onStop();
		ogDB.close();
	}
	
	@Override
	public void onSaveInstanceState(Bundle opSavedState) {
		
		opSavedState.putString(LAST_FRAGMENT, sgActualFragment);
		super.onSaveInstanceState(opSavedState);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu opMenu) {
		getMenuInflater().inflate(R.menu.note_activity, opMenu);
		ogMenu = opMenu;
		
		if(sgActualFragment != null && !sgActualFragment.equals("")) {
			((FragmentInterface) getFragmentManager().findFragmentByTag(sgActualFragment)).changeMenu(ogMenu);
			ogMenu.findItem(R.id.about_option).setVisible(true);
		}
		
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem opItem) {
		LocalDialog olDialog;
		
		InputMethodManager olInputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		
		if(getCurrentFocus() != null)
			olInputManager.hideSoftInputFromWindow(getCurrentFocus().getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);

		switch(opItem.getItemId()) {
			case android.R.id.home:
				if((getActionBar().getDisplayOptions() & ActionBar.DISPLAY_HOME_AS_UP) == 0)
					break;
				olDialog = new LocalDialog();
				olDialog.setMessage(R.string.cancel_message);
				olDialog.showCancelButton();
				olDialog.setTag(CANCEL_TAG);
				olDialog.setCaller(this);
				olDialog.show(getFragmentManager(), CANCEL_TAG);
				break;
			case R.id.import_option:
				olDialog = new LocalDialog();
				olDialog.setMessage(R.string.import_message);
				olDialog.showCancelButton();
				olDialog.setTag(IMPORT_TAG);
				olDialog.setCaller(this);
				olDialog.show(getFragmentManager(), IMPORT_TAG);
				break;
			case R.id.export_option:
				backupNotes();
				break;
			case R.id.save_option:
				((FragmentInterface) getFragmentManager().findFragmentByTag(NoteEditFragment.FRAGMENT_TAG)).onClick(R.id.save_option);
				break;
			case R.id.del_option:
				((FragmentInterface) getFragmentManager().findFragmentByTag(NoteEditFragment.FRAGMENT_TAG)).onClick(R.id.del_option);
				break;
			case R.id.add_option:
				((FragmentInterface) getFragmentManager().findFragmentByTag(NoteListFragment.FRAGMENT_TAG)).onClick(R.id.del_option);
				break;
			case R.id.about_option:
				olDialog = new LocalDialog();
				olDialog.setMessage(R.string.about_message);
				olDialog.setCaller(this);
				olDialog.show(getFragmentManager(), ABOUT_TAG);
		}
		
		return super.onOptionsItemSelected(opItem);
	}
	
	private void backupNotes() {
		DocumentBuilderFactory olDocFactory;
		DocumentBuilder olDocBuilder;
		Document olDocument;
		Element olRootElement;
		Element olNote;
		Element olDate;
		Element olText;
		TransformerFactory olTransformerFactory;
		Transformer olTransformer;
		DOMSource olSource;
		StreamResult olResult;
		Cursor olCrs = null;
		File flFile = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), BACKUP_FILE);
		String slState = Environment.getExternalStorageState();

		if(Environment.isExternalStorageRemovable() && !Environment.MEDIA_MOUNTED.equals(slState))
			return;

		try {
	    	
			olDocFactory = DocumentBuilderFactory.newInstance();
			olDocBuilder = olDocFactory.newDocumentBuilder();
			
			olDocument = olDocBuilder.newDocument();
			olRootElement = olDocument.createElement(ROOT_NODE);
			olDocument.appendChild(olRootElement);
			
			olCrs = ogDB.rawQuery("SELECT note_date, note_text FROM notes ORDER BY note_date DESC", null);
	        if(olCrs.moveToFirst()) {
	        	
        		olNote = olDocument.createElement(ELEMENT_NODE);
    			olRootElement.appendChild(olNote);

    			olDate = olDocument.createElement(DATE_NODE);
    			olDate.appendChild(olDocument.createTextNode(olCrs.getString(0)));
    			olNote.appendChild(olDate);

    			olText = olDocument.createElement(TEXT_NODE);
    			olText.appendChild(olDocument.createTextNode(olCrs.getString(1)));
    			olNote.appendChild(olText);

    			while(olCrs.moveToNext()) {
	    			
	        		olNote = olDocument.createElement(ELEMENT_NODE);
	    			olRootElement.appendChild(olNote);

	    			olDate = olDocument.createElement(DATE_NODE);
	    			olDate.appendChild(olDocument.createTextNode(olCrs.getString(0)));
	    			olNote.appendChild(olDate);

	    			olText = olDocument.createElement(TEXT_NODE);
	    			olText.appendChild(olDocument.createTextNode(olCrs.getString(1)));
	    			olNote.appendChild(olText);

	    		}
	    	
	        } else {
	    		olCrs.close();
	    		return;
	    	}
	    	olCrs.close();
	    	
			olTransformerFactory = TransformerFactory.newInstance();
			olTransformer = olTransformerFactory.newTransformer();
			olSource = new DOMSource(olDocument);
			olResult = new StreamResult(flFile);
			olTransformer.transform(olSource, olResult);
	    	
	        MediaScannerConnection.scanFile(
	        		this,
	                new String[] { flFile.toString() }, 
	                null,
	                null
	        );

		} catch(ParserConfigurationException e) {

		} catch(TransformerException e) {
	    	
	    } catch(Exception e) {
	    	if(olCrs != null) olCrs.close();
	    }

	}
	
	private void restoreNotes() {
		class Note {

			private String sgDate;
			private String sgText;
			
			public Note(String spDate, String spText) {
				sgDate = spDate;
				sgText = spText;
			}
			
			public String getDate() {
				return sgDate;
			}
			
			public String getText() {
				return sgText;
			}

		}
		
		DocumentBuilderFactory olDocFactory;
		DocumentBuilder olDocBuilder;
		Document olDocument;
		NodeList alList;
		Node olNode;
		Element olNote;
		Note[] alNotes;
		NoteListFragment olFragment;
		String slDate, slText;
		NodeList alDate, alText;

		File flFile = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), BACKUP_FILE);
		String slState = Environment.getExternalStorageState();
		
		if(Environment.isExternalStorageRemovable() && !(Environment.MEDIA_MOUNTED.equals(slState) || Environment.MEDIA_MOUNTED_READ_ONLY.equals(slState)))
			return;
		
		if(!flFile.exists() || flFile.length() == 0)
			return;
		
		try {
			
			olDocFactory = DocumentBuilderFactory.newInstance();
			olDocBuilder = olDocFactory.newDocumentBuilder();
			
			olDocument = olDocBuilder.parse(flFile);
			olDocument.getDocumentElement().normalize();
			
			alList = olDocument.getElementsByTagName(ELEMENT_NODE);
			
			if(alList.getLength() != 0) {
				
				alNotes = new Note[alList.getLength()];
				
				for(int i = 0; i < alList.getLength(); i++){
					
					olNode = alList.item(i);
					
					if(olNode.getNodeType() == Element.ELEMENT_NODE) {
						
						olNote = (Element) olNode;
						
						alDate = olNote.getElementsByTagName(DATE_NODE);
						if(alDate != null && alDate.getLength() != 0)
							slDate = alDate.item(0).getTextContent();
						else
							slDate = "";
						
						alText = olNote.getElementsByTagName(TEXT_NODE);
						if(alText != null && alText.getLength() != 0)
							slText = alText.item(0).getTextContent();
						else
							slText = "";
						
						alNotes[i] = new Note(
													slDate,
													slText
											 );
					}
				}
				
				ogDB.beginTransaction();
				
				ogDB.execSQL("DELETE FROM notes");
				for(int i = 0, id = 1; i < alNotes.length; i++)
					if(alNotes[i].getText() != null && alNotes[i].getText().trim().length() != 0) {
						ogDB.execSQL("INSERT INTO notes (note_id, note_text, note_date) VALUES (" + id + ", '" + alNotes[i].getText() + "', " + (alNotes[i].getDate() != null && alNotes[i].getDate().trim().length() != 0 ? "'" + alNotes[i].getDate() + "'": "date('now')") + ")");
						id++;
					}
				
				ogDB.setTransactionSuccessful();
				ogDB.endTransaction();
				
				olFragment = ((NoteListFragment) getFragmentManager().findFragmentByTag(NoteListFragment.FRAGMENT_TAG));
				if(olFragment != null)
					olFragment.refreshList();

			}

		} catch(ParserConfigurationException e) {

	    } catch(Exception e) {

	    }

	}
	
	private void go(Fragment opFragment, String spTag){
		FragmentTransaction olTrx = getFragmentManager().beginTransaction();
		
		sgActualFragment = spTag;
		olTrx.replace(R.id.fragment_container, opFragment, spTag);
		olTrx.commit();
		((FragmentInterface) opFragment).changeMenu(ogMenu);

	}
	
	private void goBack() {
		go(new NoteListFragment(), NoteListFragment.FRAGMENT_TAG);
		getActionBar().setDisplayHomeAsUpEnabled(false);
	}

	private void goForward(int ipNoteId, String spNoteText) {
		NoteEditFragment ogFragment = new NoteEditFragment();
		
		ogFragment.setNote(ipNoteId, spNoteText);
		
		go(ogFragment, NoteEditFragment.FRAGMENT_TAG);
		getActionBar().setDisplayHomeAsUpEnabled(true);
	}

	@Override
	public void onDeleteNote(int ipNoteId) {
		ogDB.beginTransaction();
		ogDB.execSQL("DELETE FROM notes WHERE note_id = " + ipNoteId);
		ogDB.setTransactionSuccessful();
		ogDB.endTransaction();
		
		goBack();
	}

	@Override
	public void onSaveNote(int ipNoteId, String spNoteText) {
		Cursor olCrs;
		int ilId;
		
		if(spNoteText == null || spNoteText.trim().equals(""))
			return;
		
		ogDB.beginTransaction();
		
		if(ipNoteId == 0){

				olCrs = ogDB.rawQuery("SELECT max(note_id) FROM notes", null);
				
			try {
					
				if(olCrs.moveToFirst())
					ilId = olCrs.getInt(0) + 1;
				else
					ilId = 1;
				
				olCrs.close();
				
				ogDB.execSQL("INSERT INTO notes (note_id, note_text, note_date) VALUES (" + ilId + ", '" + spNoteText + "', date('now'))");

			} catch(Exception e) {
				olCrs.close();
			}
			
		} else 
			ogDB.execSQL("UPDATE notes SET note_text = '" + spNoteText + "', note_date = date('now') WHERE note_id = " + ipNoteId);
		
		ogDB.setTransactionSuccessful();
		ogDB.endTransaction();
		
		goBack();
	}

	@Override
	public void onAddNote() {
		goForward(0, "");
	}

	@Override
	public void onSelectedNote(int ipNoteId) {
		Cursor olCrs = ogDB.rawQuery("SELECT note_text FROM notes WHERE note_id = " + ipNoteId, null);

		try {
		
			if(olCrs.moveToFirst()) {
				goForward(ipNoteId, olCrs.getString(0));
				olCrs.close();
			}

		} catch(Exception e) {
			olCrs.close();
		}
		
	}

	@Override
	public void onOkButtonClick(String spTag) {
		
		if(spTag == null)
			return;
		
		if(spTag.equals(CANCEL_TAG))
			goBack();
		
		if(spTag.equals(IMPORT_TAG))
			restoreNotes();

	}

	@Override
	public void onCancelButtonClick(String spTag) {
		// TODO Auto-generated method stub
		
	}
	
}
