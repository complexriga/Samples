package com.afgv.notemanager;

import android.app.Fragment;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.os.Bundle;
import android.widget.EditText;

public class NoteEditFragment extends Fragment implements LocalDialog.DialogFragmentCallback, FragmentInterface {
	
	public final static String FRAGMENT_TAG = "EDIT";
	private final static String DEL_DIALOG_TAG = "DEL_DIALOG";

	private int igNoteId;
	private String sgNoteText;
	private EditText ogEdit;
	
	public interface NoteFragmentCallback {
		
		public void onDeleteNote(int ipNoteId);
		public void onSaveNote(int ipNoteId, String spNoteText);
	}
	
	@Override
	public View onCreateView(LayoutInflater opInflater, ViewGroup opContainer, Bundle opSavedState) {
		View olFragment = opInflater.inflate(R.layout.note_edit, opContainer, false);
		
		ogEdit = (EditText) olFragment.findViewById(R.id.note);
		ogEdit.setText(sgNoteText);
		
		return olFragment;
	}
	
	@Override
	public void onClick(int ipId) {
		LocalDialog olDialog;
		
		InputMethodManager olInputManager = (InputMethodManager) getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
		
		if(getActivity().getCurrentFocus() != null)
			olInputManager.hideSoftInputFromWindow(getActivity().getCurrentFocus().getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
		
		switch(ipId){
			case R.id.save_option:
				((NoteFragmentCallback) getActivity()).onSaveNote(igNoteId, ogEdit.getText().toString());
				break;
			case R.id.del_option:
				olDialog = new LocalDialog();
				olDialog.setMessage(R.string.del_message);
				olDialog.showCancelButton();
				olDialog.setCaller(this);
				olDialog.show(getFragmentManager(), DEL_DIALOG_TAG);
		}
	}
	
	@Override
	public void changeMenu(Menu opMenu) {
		MenuItem olItem;
		
		for(int i = 0; i < opMenu.size(); i++) {
			
			olItem = opMenu.getItem(i); 
			switch(olItem.getItemId()) {
				case R.id.save_option:
					olItem.setVisible(true);
					break;
				case R.id.del_option:
					if(igNoteId != 0)
						olItem.setVisible(true);
					else
						olItem.setVisible(false);
					break;
				default:
					olItem.setVisible(false);						
			}
		}
	}

	@Override
	public void onOkButtonClick(String spTag) {
		((NoteFragmentCallback) getActivity()).onDeleteNote(igNoteId);
	}

	@Override
	public void onCancelButtonClick(String spTag) {
		
	}

	public void setNote(int ipNoteId, String spNoteText) {
		igNoteId = ipNoteId;
		sgNoteText = spNoteText;
	}

}
