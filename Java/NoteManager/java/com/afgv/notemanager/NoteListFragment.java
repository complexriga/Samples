package com.afgv.notemanager;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.SimpleCursorAdapter;
import android.database.sqlite.SQLiteDatabase;
import android.database.Cursor;

public class NoteListFragment extends Fragment implements OnItemClickListener, FragmentInterface {
	
	public final static String FRAGMENT_TAG = "LIST";

	public interface NoteListFragmentCallback {
		public void onAddNote();
		public void onSelectedNote(int ipNoteId);
	}
	
	ListView ogList;
	
	@Override
	public View onCreateView(LayoutInflater opInflater, ViewGroup opContainer, Bundle opSavedState) {
		super.onCreateView(opInflater, opContainer, opSavedState);
		
		View olFragment = opInflater.inflate(R.layout.note_list, opContainer, false);
		
		ogList = (ListView) olFragment.findViewById(R.id.notes_list);
		ogList.setOnItemClickListener(this);
		
		return olFragment;
	}
	
	@Override
	public void onStart() {
		super.onStart();
		populateList();
	}
	
	@Override
	public void onStop() {
		super.onStop();
		((SimpleCursorAdapter) ogList.getAdapter()).getCursor().close();
	}
	
	private void populateList() {
		DatabaseHelper olHelper = new DatabaseHelper(ogList.getContext());
		SQLiteDatabase olDB = olHelper.getReadableDatabase();
		SimpleCursorAdapter olOldAdapter= (SimpleCursorAdapter) ogList.getAdapter();
		
		Cursor olCrs = olDB.rawQuery("SELECT rowid _id, note_id, note_text, note_date FROM notes", null);
		
		SimpleCursorAdapter olAdapter = new SimpleCursorAdapter(
					ogList.getContext(), 
					R.layout.list_element, 
					olCrs, 
					new String[] {"note_id", "note_text", "note_date"}, 
					new int[] {R.id.note_id, R.id.note_text, R.id.note_date},
					0
		);
		
		if(olOldAdapter != null)
			olOldAdapter.getCursor().close();
			
		ogList.setAdapter(olAdapter);
	}
	
	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		((NoteListFragmentCallback) getActivity()).onSelectedNote(((Cursor) arg0.getItemAtPosition(arg2)).getInt(0));
		
	}
	
	@Override
	public void onClick(int ipId) {
		((NoteListFragmentCallback) getActivity()).onAddNote();
	}

	@Override
	public void changeMenu(Menu opMenu) {
		MenuItem olItem;
		
		for(int i = 0; i < opMenu.size(); i++) {
			
			olItem = opMenu.getItem(i); 
			switch(olItem.getItemId()){
				case R.id.add_option:
				case R.id.export_option:
				case R.id.import_option:
					olItem.setVisible(true);
					break;
				default:
					olItem.setVisible(false);						
			}
		}
	}

	public void refreshList() {
		populateList();
	}

}
