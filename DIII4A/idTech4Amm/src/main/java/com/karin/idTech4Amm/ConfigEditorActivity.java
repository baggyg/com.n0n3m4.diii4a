package com.karin.idTech4Amm;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.EditText;
import android.content.Intent;
import java.io.File;
import android.widget.Toast;
import android.text.TextWatcher;
import android.text.Editable;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.preference.PreferenceManager;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.karin.idTech4Amm.lib.ContextUtility;
import com.karin.idTech4Amm.lib.FileUtility;
import com.karin.idTech4Amm.sys.Constants;

@SuppressLint("NonConstantResourceId")
public class ConfigEditorActivity extends Activity
{
    private ViewHolder V = new ViewHolder();
    private String m_filePath;
    private File m_file;
    private boolean m_edited = false;

    private TextWatcher m_textWatcher = new TextWatcher() {           
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        if(m_edited)
            return;
        m_edited = true; 
        V.titleText.setText(m_filePath + "*");
        V.titleText.setTextColor(Color.RED);
        if(V.saveBtn != null)
            V.saveBtn.setEnabled(true);
        if(V.reloadBtn != null)
            V.reloadBtn.setEnabled(true);
    }           
    public void beforeTextChanged(CharSequence s, int start, int count,int after) {}            
    public void afterTextChanged(Editable s) {}
    };

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        boolean o = PreferenceManager.getDefaultSharedPreferences(this).getBoolean(Constants.PreferenceKey.LAUNCHER_ORIENTATION, false);
        ContextUtility.SetScreenOrientation(this, o ? 0 : 1);

        setContentView(R.layout.editor_page);

        V.SetupUI();

        SetupUI();
    }

    private void SetupUI()
    {
        Intent intent = getIntent();
        String path = intent.getStringExtra("CONST_FILE_PATH");
        if (path != null)
        {
            LoadFile(path);
        }
    }

    private void Reset()
    {
        V.editText.removeTextChangedListener(m_textWatcher);
        m_edited = false;
        m_filePath = null;
        m_file = null;

        if(V.saveBtn != null)
            V.saveBtn.setEnabled(false);
        if(V.reloadBtn != null)
            V.reloadBtn.setEnabled(false);
        V.editText.setText("");
        V.titleText.setText("");
        V.editText.setFocusableInTouchMode(false);
        V.titleText.setTextColor(Color.BLACK);
    }

    private boolean LoadFile(String path)
    {
        Reset();

        File file = new File(path);
        String text = FileUtility.file_get_contents(file);
        if (text != null)
        {
            V.editText.setText(text);
            //V.saveBtn.setClickable(true);
            //V.reloadBtn.setClickable(true);
            V.editText.setFocusableInTouchMode(true);
            V.editText.addTextChangedListener(m_textWatcher);
            V.titleText.setText(path);
            m_file = file;
            m_filePath = path;
            return true;
        }
        else
        {
            Reset();
            return false;
        }
    }

    private boolean SaveFile()
    {
        if (m_file == null)
            return false;
        return FileUtility.file_put_contents(m_file, V.editText.getText().toString());
    }

    private boolean IsValid()
    {
        return m_file != null;
    }

    @Override
    public void onBackPressed()
    {
        if(IsValid() && m_edited)
        {
            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which)
                {
                    switch(which)
                    {
                        case DialogInterface.BUTTON_POSITIVE:
                            SaveFile();
                            dialog.dismiss();
                            finish();
                            break;
                        case DialogInterface.BUTTON_NEGATIVE:
                            dialog.dismiss();
                            finish();
                            break;
                        case DialogInterface.BUTTON_NEUTRAL:
                        default:
                            dialog.dismiss();
                            break;
                    }
                }
            };
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Warning");
            builder.setMessage("The text is modified since open, save changes to file?");
            builder.setPositiveButton("Yes", listener);
            builder.setNegativeButton("No", listener);
            builder.setNeutralButton("Cancel", listener);
            AlertDialog dialog = builder.create();
            dialog.show();
        }
        else
            super.onBackPressed();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.config_editor_menu, menu);
        V.SetupMenu(menu);
        V.saveBtn.setEnabled(false);
        V.reloadBtn.setEnabled(false);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch(item.getItemId())
        {
            case R.id.config_editor_menu_save:
                if (IsValid())
                {
                    if (SaveFile())  
                    {
                        m_edited = false;
                        V.titleText.setText(m_filePath);
                        V.titleText.setTextColor(Color.BLACK);
                        if(V.saveBtn != null)
                            V.saveBtn.setEnabled(false);
                        if(V.reloadBtn != null)
                            V.reloadBtn.setEnabled(false);
                        Toast.makeText(ConfigEditorActivity.this, "Save file successful.", Toast.LENGTH_LONG).show();
                    }
                    else
                        Toast.makeText(ConfigEditorActivity.this, "Save file failed!", Toast.LENGTH_LONG).show(); 

                }
                else
                    Toast.makeText(ConfigEditorActivity.this, "No file!", Toast.LENGTH_LONG).show();
                break;
                case R.id.config_editor_menu_reload:
                if (IsValid())
                    LoadFile(m_filePath);
                else
                    Toast.makeText(ConfigEditorActivity.this, "No file!", Toast.LENGTH_LONG).show();
                    break;
                default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private class ViewHolder
    {
        private TextView titleText;
        private EditText editText;
        private MenuItem saveBtn;
        private MenuItem reloadBtn;

        public void SetupUI()
        {
            titleText = findViewById(R.id.editor_page_title);
            editText = findViewById(R.id.editor_page_editor);
        }

        public void SetupMenu(Menu menu)
        {
            saveBtn = menu.findItem(R.id.config_editor_menu_save);
            reloadBtn = menu.findItem(R.id.config_editor_menu_reload);
        }
    }
}
