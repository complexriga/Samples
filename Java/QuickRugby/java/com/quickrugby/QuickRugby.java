package com.quickrugby;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

public class QuickRugby extends AppCompatActivity {

    private GameSpace  ogView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
        ogView = new GameSpace(this);
        setContentView(ogView);
    }
}
