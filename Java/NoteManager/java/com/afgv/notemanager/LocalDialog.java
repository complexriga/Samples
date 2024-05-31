package com.afgv.notemanager;

import android.app.DialogFragment;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;

public class LocalDialog extends DialogFragment implements DialogInterface.OnClickListener {
	
	public interface DialogFragmentCallback {
		
		public void onOkButtonClick(String spTag);
		public void onCancelButtonClick(String spTag);
	}
	
	private int igMessage = -1;
	private int igTitle = -1;
	private boolean bgShowCancel = false;
	private String sgTag = "";
	private DialogFragmentCallback ogCaller;
	
	@Override
	public AlertDialog onCreateDialog(Bundle opSavedState) {
		super.onCreateDialog(opSavedState);
		
		AlertDialog.Builder olBuilder = new AlertDialog.Builder(getActivity());
		
		if(igMessage != -1)
			olBuilder.setMessage(igMessage);
		
		if(igTitle != -1)
			olBuilder.setTitle(igTitle);
		
		olBuilder.setPositiveButton(android.R.string.ok, this);
		
		if(bgShowCancel)
			olBuilder.setNegativeButton(android.R.string.cancel, this);
		
		return olBuilder.create();
		
	}

	@Override
	public void onClick(DialogInterface opDialog, int ipButton) {

		switch(ipButton) {
			case DialogInterface.BUTTON_POSITIVE:
				ogCaller.onOkButtonClick(sgTag);
				break;
				
			case DialogInterface.BUTTON_NEGATIVE:
				ogCaller.onCancelButtonClick(sgTag);
		}
		
	}
	
	public void setTitle(int ipTitle) {
		igTitle = ipTitle;
	}
	
	public void setMessage(int ipMessage) {
		igMessage = ipMessage;
	}
	
	public void showCancelButton() {
		bgShowCancel = true;
	}
	
	public void hideCancelButton() {
		bgShowCancel = false;
	}
	
	public void setCaller(DialogFragmentCallback opCaller) {
		ogCaller = opCaller;
	}
	
	public void setTag(String spTag) {
		if(spTag != null)
			sgTag = spTag;
		else
			sgTag = "";
	}
	
}
