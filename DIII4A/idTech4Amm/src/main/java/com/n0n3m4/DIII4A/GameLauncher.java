/*
	Copyright (C) 2012 n0n3m4
	
    This file is part of RTCW4A.

    RTCW4A is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    RTCW4A is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RTCW4A.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.n0n3m4.DIII4A;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Display;
import android.view.DisplayCutout;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowInsets;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;

import com.karin.idTech4Amm.ConfigEditorActivity;
import com.karin.idTech4Amm.OnScreenButtonConfigActivity;
import com.karin.idTech4Amm.R;
import com.karin.idTech4Amm.lib.ContextUtility;
import com.karin.idTech4Amm.lib.FileUtility;
import com.karin.idTech4Amm.lib.Utility;
import com.karin.idTech4Amm.misc.TextHelper;
import com.karin.idTech4Amm.sys.Constants;
import com.karin.idTech4Amm.sys.UncaughtExceptionHandler;
import com.karin.idTech4Amm.ui.DebugDialog;
import com.karin.idTech4Amm.ui.FileBrowserDialog;
import com.karin.idTech4Amm.ui.LauncherSettingsDialog;
import com.n0n3m4.q3e.Q3EInterface;
import com.n0n3m4.q3e.Q3EJNI;
import com.n0n3m4.q3e.Q3EKeyCodes;
import com.n0n3m4.q3e.Q3EMain;
import com.n0n3m4.q3e.Q3EUiConfig;
import com.n0n3m4.q3e.Q3EUtils;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import android.widget.RadioButton;
import java.util.Set;
import android.graphics.drawable.ColorDrawable;
import android.content.res.Resources;
import android.app.ActionBar;
import android.widget.Spinner;
import android.widget.AdapterView;
import java.util.Map;
import com.n0n3m4.q3e.Q3EControlView;
import com.karin.idTech4Amm.lib.D3CommandUtility;
import android.util.Log;
import java.util.LinkedList;
import android.graphics.Point;

import org.json.JSONObject;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

@SuppressLint({"ApplySharedPref", "NonConstantResourceId", "CommitPrefEdits"})
public class GameLauncher extends Activity{		
    private static final int CONST_REQUEST_EXTERNAL_STORAGE_FOR_START_RESULT_CODE = 1;
    private static final int CONST_REQUEST_EXTERNAL_STORAGE_FOR_EDIT_CONFIG_FILE_RESULT_CODE = 2;
    private static final int CONST_REQUEST_EXTERNAL_STORAGE_FOR_CHOOSE_FOLDER_RESULT_CODE = 3;
    private static final int CONST_DEFAULT_ON_SCREEN_BUTTON_OPACITY = 30;
     
	private ViewHolder V = new ViewHolder();
    private boolean m_cmdUpdateLock = false;
	private CompoundButton.OnCheckedChangeListener m_checkboxChangeListener = new CompoundButton.OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				switch(buttonView.getId())
				{
					case R.id.useetc1cache:
						setProp("r_useETC1cache", isChecked);
                        PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                            .putBoolean(Q3EUtils.pref_useetc1cache, isChecked)
                            .commit();
						break;
					case R.id.nolight:
						setProp("r_noLight", isChecked);
                        PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                            .putBoolean(Q3EUtils.pref_nolight, isChecked)
                            .commit();
						break;
					case R.id.useetc1:
						setProp("r_useETC1", isChecked);
                        PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                            .putBoolean(Q3EUtils.pref_useetc1, isChecked)
                            .commit();
						break;
					case R.id.usedxt:
						setProp("r_useDXT", isChecked);
                        PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                            .putBoolean(Q3EUtils.pref_usedxt, isChecked)
                            .commit();
						break;
					case R.id.detectmouse:
						UpdateMouseManualMenu(!isChecked);
						PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
							.putBoolean(Q3EUtils.pref_detectmouse, isChecked)
						.commit();
						break;
					case R.id.hideonscr:
						UpdateMouseMenu(isChecked);
						PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
							.putBoolean(Q3EUtils.pref_hideonscr, isChecked)
						.commit();
						break;
					case R.id.smoothjoy:
						PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
							.putBoolean(Q3EUtils.pref_analog, isChecked)
						.commit();
						break;
					case R.id.mapvol:
						PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
							.putBoolean(Q3EUtils.pref_mapvol, isChecked)
						.commit();
                        UpdateMapVol(isChecked);
						break;
					case R.id.secfinglmb:
						PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
							.putBoolean(Q3EUtils.pref_2fingerlmb, isChecked)
						.commit();
						break;
                    case R.id.fs_game_user:
                        UpdateUserGame(isChecked);
                        PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                            .putBoolean(Q3EUtils.pref_harm_user_mod, isChecked)
                            .commit();
						break;
                    case R.id.launcher_tab2_enable_gyro:
                        PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                            .putBoolean(Q3EUtils.pref_harm_view_motion_control_gyro, isChecked)
                            .commit();
                        UpdateEnableGyro(isChecked);
						break;
					case R.id.auto_quick_load:
						PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
								.putBoolean(Q3EUtils.pref_harm_auto_quick_load, isChecked)
								.commit();
						if(isChecked)
							SetParam_temp("loadGame", "QuickSave");
						else
							RemoveParam_temp("loadGame");
						break;
					default:
						break;
				}
			}
		};
	private RadioGroup.OnCheckedChangeListener m_groupCheckChangeListener = new RadioGroup.OnCheckedChangeListener() {
		@Override
		public void onCheckedChanged(RadioGroup radioGroup, int id)
		{
            int index;
			switch(radioGroup.getId())
			{
				case R.id.rg_scrres:
					GameLauncher.this.UpdateCustomerResulotion(id == R.id.res_custom);
                    index = GetCheckboxIndex(radioGroup, id);
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putInt(Q3EUtils.pref_scrres, index)
                        .commit();
					break;
				case R.id.r_harmclearvertexbuffer:
                    index = GetCheckboxIndex(radioGroup, id);
					SetProp("harm_r_clearVertexBuffer", index);
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                    .putInt(Q3EUtils.pref_harm_r_harmclearvertexbuffer, index)
                    .commit();
					break;
                case R.id.rg_harm_r_lightModel:
                    String value = GetCheckboxIndex(radioGroup, id) == 1 ? "blinn_phong" : "phong";
                    SetProp("harm_r_lightModel", value);
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putString(Q3EUtils.pref_harm_r_lightModel, value)
                    .commit();
					break;
				case R.id.rg_fs_game:
				case R.id.rg_fs_q4game:
					SetGameDLL(id);
					break;
                case R.id.rg_msaa:
                    index = GetCheckboxIndex(radioGroup, id);
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putInt(Q3EUtils.pref_msaa, index)
                        .commit();
                    break;
                case R.id.rg_color_bits:
                    index = GetCheckboxIndex(radioGroup, id) - 1;
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                    .putBoolean(Q3EUtils.pref_32bit, index == -1)
                    .putInt(Q3EUtils.pref_harm_16bit, index)
                        .commit();
                    break;
				case R.id.rg_curpos:
                    index = GetCheckboxIndex(radioGroup, id);
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putInt(Q3EUtils.pref_mousepos, index)
                        .commit();
					break;
				default:
					break;
			}
		}
	};
	private View.OnClickListener m_buttonClickListener = new View.OnClickListener(){
        @Override
		public void onClick(View view)
		{
			switch(view.getId())
			{
				case R.id.launcher_tab1_edit_autoexec:
					EditFile("autoexec.cfg");
					break;
				case R.id.launcher_tab1_edit_doomconfig:
                    EditFile(Q3EUtils.q3ei.config_name);
					break;
				case R.id.launcher_tab1_game_lib_button:
                    OpenGameLibChooser();
					break;
				case R.id.launcher_tab1_game_data_chooser_button:
					OpenFolderChooser();
					break;
				case R.id.onscreen_button_setting:
					OpenOnScreenButtonSetting();
					break;
				case R.id.setup_onscreen_button_opacity:
					OpenOnScreenButtonOpacitySetting();
					break;
				default:
					break;
			}
		}
	};

	private class SavePreferenceTextWatcher implements TextWatcher
	{    
		private String name;
		private String defValue;
		public SavePreferenceTextWatcher(String name, String defValue)
		{
			this.name = name;
			this.defValue = defValue;
		}
		public SavePreferenceTextWatcher(String name)
		{
			this(name, null);
		}
		public void onTextChanged(CharSequence s, int start, int before, int count) {}
		public void beforeTextChanged(CharSequence s, int start, int count,int after) {}            
		public void afterTextChanged(Editable s)
		{
            String value = s.length() == 0 && null != defValue ? defValue : s.toString();
			PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
				.putString(name, value)
				.commit();
		}
	};
    
    private AdapterView.OnItemSelectedListener m_spinnerItemSelectedListener = new AdapterView.OnItemSelectedListener() {
		public void onItemSelected(AdapterView adapter, View view, int position, long id)
		{
			int[] keyCodes;
			switch(adapter.getId())
			{
				case R.id.launcher_tab2_volume_up_map_config_keys:
					keyCodes = getResources().getIntArray(R.array.key_map_codes);
					PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
						.putInt(Constants.PreferenceKey.VOLUME_UP_KEY, keyCodes[position])
						.commit();
					break;
				case R.id.launcher_tab2_volume_down_map_config_keys:
					keyCodes = getResources().getIntArray(R.array.key_map_codes);
					PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
						.putInt(Constants.PreferenceKey.VOLUME_DOWN_KEY, keyCodes[position])
						.commit();
					break;
				default:
					break;
			}
		}
		public void onNothingSelected(AdapterView adapter) {}
    };
	
	final String default_gamedata= Environment.getExternalStorageDirectory() + "/diii4a";
	
	public static final int UI_JOYSTICK=0;
	public static final int UI_SHOOT=1;
	public static final int UI_JUMP=2;
	public static final int UI_CROUCH=3;
	public static final int UI_RELOADBAR=4;	
	public static final int UI_PDA=5;
	public static final int UI_FLASHLIGHT=6;
	public static final int UI_SAVE=7;
	public static final int UI_1=8;
	public static final int UI_2=9;
	public static final int UI_3=10;	
	public static final int UI_KBD=11;
    public static final int UI_CONSOLE = 12;
    public static final int UI_RUN = 13;
    public static final int UI_ZOOM = 14;
    public static final int UI_INTERACT = 15;
    public static final int UI_WEAPON_PANEL = 16;
	public static final int UI_SIZE=UI_WEAPON_PANEL+1;
	
	public void getgl2progs(String destname) {
        try {            
            byte[] buf = new byte[4096];
            ZipInputStream zipinputstream = null;
            InputStream bis;
            ZipEntry zipentry;
                        
            bis=getAssets().open("gl2progs.zip");            
            zipinputstream=new ZipInputStream(bis);
            
            (new File(destname)).mkdirs();                               
            while ((zipentry= zipinputstream.getNextEntry())!=null) 
            {                	            	                
            	String tmpname=zipentry.getName();                	                	
            	
            	String entryName = destname + tmpname;
                entryName = entryName.replace('/', File.separatorChar);
                entryName = entryName.replace('\\', File.separatorChar);
            	                                    
                int n;
                FileOutputStream fileoutputstream;
                File newFile = new File(entryName);
                if (zipentry.isDirectory()) {
                    if (!newFile.mkdirs()) {
                    }                        
                    continue;
                }
                fileoutputstream = new FileOutputStream(entryName);               
                while ((n = zipinputstream.read(buf, 0, 4096)) > -1) {
                    fileoutputstream.write(buf, 0, n);                    
                }                                             
                fileoutputstream.close();                                                     
                zipinputstream.closeEntry();
            }

            zipinputstream.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private void InitUILayout(Q3EInterface q3ei, boolean safety)
    {
        int safeInsetTop = ContextUtility.GetEdgeHeight(this);
		int[] fullSize = ContextUtility.GetFullScreenSize(this);
		int[] size = ContextUtility.GetNormalScreenSize(this);
		int navBarHeight = fullSize[1] - size[1] - safeInsetTop;
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(GameLauncher.this);
		boolean hideNav = preferences.getBoolean(Constants.PreferenceKey.HIDE_NAVIGATION_BAR, true);
		boolean coverEdges = preferences.getBoolean(Constants.PreferenceKey.COVER_EDGES, true);
		int w, h;
		int X = 0;
		if(safety)
		{
			w = fullSize[0];
			h = fullSize[1];
			h -= navBarHeight;
			if(coverEdges)
				X = safeInsetTop;
			else
				h -= safeInsetTop;
		}
		else
		{
			w = fullSize[0];
			h = fullSize[1];
			if(!hideNav)
				h -= navBarHeight;
			if(!coverEdges)
				h -= safeInsetTop;
		}
        int width = Math.max(w, h);
		int height = Math.min(w, h);

        int r=Q3EUtils.dip2px(this,75);
        int rightoffset=r*3/4;
        int sliders_width=Q3EUtils.dip2px(this,125);
        int alpha = CONST_DEFAULT_ON_SCREEN_BUTTON_OPACITY;

        q3ei.defaults_table=new String[UI_SIZE];
        q3ei.defaults_table[UI_JOYSTICK] =(X + r*4/3)+" "+(height-r*4/3)+" "+r+" "+alpha;
        q3ei.defaults_table[UI_SHOOT]    =(width-r/2-rightoffset)+" "+(height-r/2-rightoffset)+" "+r*3/2+" "+alpha;
        q3ei.defaults_table[UI_JUMP]     =(width-r/2)+" "+(height-r-2*rightoffset)+" "+r+" "+alpha;
        q3ei.defaults_table[UI_CROUCH]   =(width-r/2)+" "+(height-r/2)+" "+r+" "+alpha;
        q3ei.defaults_table[UI_RELOADBAR]=(width-sliders_width/2-rightoffset/3)+" "+(sliders_width*3/8)+" "+sliders_width+" "+alpha;
        q3ei.defaults_table[UI_PDA]   =(width-r-2*rightoffset)+" "+(height-r/2)+" "+r+" "+alpha;
        q3ei.defaults_table[UI_FLASHLIGHT]     =(width-r/2-4*rightoffset)+" "+(height-r/2)+" "+r+" "+alpha;
        q3ei.defaults_table[UI_SAVE]     =(X + sliders_width/2)+" "+sliders_width/2+" "+sliders_width+" "+alpha;

        for (int i=UI_SAVE+1;i<UI_SIZE;i++)
            q3ei.defaults_table[i]=(r/2+r*(i-UI_SAVE-1))+" "+(height+r/2)+" "+r+" "+alpha;

        q3ei.defaults_table[UI_WEAPON_PANEL] =(width - sliders_width - r - rightoffset)+" "+(r)+" "+(r / 3)+" "+alpha;

        //k
        final int sr = r / 6 * 5;
        q3ei.defaults_table[UI_1] = String.format("%d %d %d %d", width - sr / 2 - sr * 2, (sliders_width * 5 / 8 + sr / 2), sr, alpha);
        q3ei.defaults_table[UI_2] = String.format("%d %d %d %d", width - sr / 2 - sr, (sliders_width * 5 / 8 + sr / 2), sr, alpha);
        q3ei.defaults_table[UI_3] = String.format("%d %d %d %d", width - sr / 2, (sliders_width * 5 / 8 + sr / 2), sr, alpha);
        q3ei.defaults_table[UI_KBD] = String.format("%d %d %d %d", X + sliders_width + sr / 2, sr / 2, sr, alpha);
        q3ei.defaults_table[UI_CONSOLE] = String.format("%d %d %d %d", X + sliders_width / 2 + sr / 2, sliders_width / 2 + sr / 2, sr, alpha);
    }
	
	public void InitQ3E()
	{			
		Q3EKeyCodes.InitD3Keycodes();
		Q3EInterface q3ei=new Q3EInterface();
		q3ei.isD3=true;
		q3ei.isD3BFG=true;
		q3ei.arg_table=new int[UI_SIZE*4];
		q3ei.type_table=new int[UI_SIZE];	
        q3ei.UI_SIZE=UI_SIZE;
        
        InitUILayout(q3ei, false);
		
		q3ei.type_table[UI_JOYSTICK]=Q3EUtils.TYPE_JOYSTICK;
		q3ei.type_table[UI_SHOOT]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_JUMP]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_CROUCH]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_RELOADBAR]=Q3EUtils.TYPE_SLIDER;		
		q3ei.type_table[UI_PDA]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_FLASHLIGHT]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_SAVE]=Q3EUtils.TYPE_SLIDER;
		q3ei.type_table[UI_1]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_2]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_3]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_KBD]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_CONSOLE]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_RUN]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_ZOOM]=Q3EUtils.TYPE_BUTTON;
		q3ei.type_table[UI_INTERACT]=Q3EUtils.TYPE_BUTTON;
        
        q3ei.type_table[UI_WEAPON_PANEL] = Q3EUtils.TYPE_DISC;
		
		q3ei.arg_table[UI_SHOOT*4]=Q3EKeyCodes.KeyCodes.K_MOUSE1;
		q3ei.arg_table[UI_SHOOT*4+1]=0;
		q3ei.arg_table[UI_SHOOT*4+2]=0;
		q3ei.arg_table[UI_SHOOT*4+3]=0;
		
		
		q3ei.arg_table[UI_JUMP*4]=Q3EKeyCodes.KeyCodes.K_SPACE;
		q3ei.arg_table[UI_JUMP*4+1]=0;
		q3ei.arg_table[UI_JUMP*4+2]=0;
		q3ei.arg_table[UI_JUMP*4+3]=0;
		
		q3ei.arg_table[UI_CROUCH*4]=Q3EKeyCodes.KeyCodesD3.K_C; // BFG
		q3ei.arg_table[UI_CROUCH*4+1]=1;
		q3ei.arg_table[UI_CROUCH*4+2]=1;
		q3ei.arg_table[UI_CROUCH*4+3]=0;
		
		q3ei.arg_table[UI_RELOADBAR*4]=Q3EKeyCodes.KeyCodesD3.K_BRACKET_RIGHT; // 93
		q3ei.arg_table[UI_RELOADBAR*4+1]=Q3EKeyCodes.KeyCodesD3.K_R; // 114
		q3ei.arg_table[UI_RELOADBAR*4+2]=Q3EKeyCodes.KeyCodesD3.K_BRACKET_LEFT; // 91
		q3ei.arg_table[UI_RELOADBAR*4+3]=0;
		
		q3ei.arg_table[UI_PDA*4]=Q3EKeyCodes.KeyCodes.K_TAB;
		q3ei.arg_table[UI_PDA*4+1]=0;
		q3ei.arg_table[UI_PDA*4+2]=0;
		q3ei.arg_table[UI_PDA*4+3]=0;
		
		q3ei.arg_table[UI_FLASHLIGHT*4]=Q3EKeyCodes.KeyCodesD3.K_F; // BFG
		q3ei.arg_table[UI_FLASHLIGHT*4+1]=0;
		q3ei.arg_table[UI_FLASHLIGHT*4+2]=0;
		q3ei.arg_table[UI_FLASHLIGHT*4+3]=0;
		
		q3ei.arg_table[UI_SAVE*4]=Q3EKeyCodes.KeyCodes.K_F5;
		q3ei.arg_table[UI_SAVE*4+1]=Q3EKeyCodes.KeyCodes.K_ESCAPE;
		q3ei.arg_table[UI_SAVE*4+2]=Q3EKeyCodes.KeyCodes.K_F9;
		q3ei.arg_table[UI_SAVE*4+3]=1;
		
		q3ei.arg_table[UI_1*4]=Q3EKeyCodes.KeyCodesD3BFG.K_1;
		q3ei.arg_table[UI_1*4+1]=0;
		q3ei.arg_table[UI_1*4+2]=0;
		q3ei.arg_table[UI_1*4+3]=0;
		
		q3ei.arg_table[UI_2*4]=Q3EKeyCodes.KeyCodesD3BFG.K_2;
		q3ei.arg_table[UI_2*4+1]=0;
		q3ei.arg_table[UI_2*4+2]=0;
		q3ei.arg_table[UI_2*4+3]=0;
		
		q3ei.arg_table[UI_3*4]=Q3EKeyCodes.KeyCodesD3BFG.K_3;
		q3ei.arg_table[UI_3*4+1]=0;
		q3ei.arg_table[UI_3*4+2]=0;
		q3ei.arg_table[UI_3*4+3]=0;
		
		q3ei.arg_table[UI_KBD*4]=Q3EUtils.K_VKBD;
		q3ei.arg_table[UI_KBD*4+1]=0;
		q3ei.arg_table[UI_KBD*4+2]=0;
		q3ei.arg_table[UI_KBD*4+3]=0;

        q3ei.arg_table[UI_CONSOLE*4]=Q3EKeyCodes.KeyCodesD3.K_CONSOLE;
        q3ei.arg_table[UI_CONSOLE*4+1]=0;
        q3ei.arg_table[UI_CONSOLE*4+2]=0;
		q3ei.arg_table[UI_CONSOLE*4+3]=0;

        q3ei.arg_table[UI_RUN*4]=Q3EKeyCodes.KeyCodesD3.K_SHIFT;
        q3ei.arg_table[UI_RUN*4+1]=1;
        q3ei.arg_table[UI_RUN*4+2]=0;
		q3ei.arg_table[UI_RUN*4+3]=0;

        q3ei.arg_table[UI_ZOOM*4]=Q3EKeyCodes.KeyCodesD3.K_Z;
        q3ei.arg_table[UI_ZOOM*4+1]=1;
        q3ei.arg_table[UI_ZOOM*4+2]=0;
		q3ei.arg_table[UI_ZOOM*4+3]=0;

        q3ei.arg_table[UI_INTERACT*4]=Q3EKeyCodes.KeyCodesD3.K_MOUSE2;
        q3ei.arg_table[UI_INTERACT*4+1]=0;
        q3ei.arg_table[UI_INTERACT*4+2]=0;
		q3ei.arg_table[UI_INTERACT*4+3]=0;
		
		q3ei.default_path=default_gamedata;		
        
		q3ei.SetupDOOM3(); //k armv7-a only support neon now
        
		q3ei.texture_table=new String[UI_SIZE];
		q3ei.texture_table[UI_JOYSTICK]="";
		q3ei.texture_table[UI_SHOOT]="btn_sht.png";
		q3ei.texture_table[UI_JUMP]="btn_jump.png";
		q3ei.texture_table[UI_CROUCH]="btn_crouch.png";
		q3ei.texture_table[UI_RELOADBAR]="btn_reload.png";
		q3ei.texture_table[UI_PDA]="btn_pda.png";
		q3ei.texture_table[UI_FLASHLIGHT]="btn_flashlight.png";
		q3ei.texture_table[UI_SAVE]="btn_pause.png";		
		q3ei.texture_table[UI_1]="btn_1.png";
		q3ei.texture_table[UI_2]="btn_2.png";
		q3ei.texture_table[UI_3]="btn_3.png";
		q3ei.texture_table[UI_KBD]="btn_keyboard.png";
		q3ei.texture_table[UI_CONSOLE]="btn_notepad.png";
		q3ei.texture_table[UI_INTERACT]="btn_activate.png";
		q3ei.texture_table[UI_ZOOM]="btn_binocular.png";
		q3ei.texture_table[UI_RUN]="btn_kick.png";

		q3ei.texture_table[UI_WEAPON_PANEL]="";
        
        Constants.DumpDefaultOnScreenConfig(q3ei.arg_table, q3ei.type_table);

        // index:type;23,1,2,0|...... 
        try
        {
            Set<String> configs = PreferenceManager.getDefaultSharedPreferences(this).getStringSet(Constants.PreferenceKey.ONSCREEN_BUTTON, null);
            if(null != configs && !configs.isEmpty())
            {
                for(String str : configs)
                {
                    String[] subArr = str.split(":", 2);
                    int index = Integer.parseInt(subArr[0]);
                    subArr = subArr[1].split(";");
                    q3ei.type_table[index] = Integer.parseInt(subArr[0]);
                    String[] argArr = subArr[1].split(",");
                    q3ei.arg_table[index * 4] = Integer.parseInt(argArr[0]);
                    q3ei.arg_table[index * 4 + 1] = Integer.parseInt(argArr[1]);
                    q3ei.arg_table[index * 4 + 2] = Integer.parseInt(argArr[2]);
                    q3ei.arg_table[index * 4 + 3] = Integer.parseInt(argArr[3]);
                }
            }        
        }
        catch(Exception e)
        {
            //UncaughtExceptionHandler.DumpException(this, Thread.currentThread(), e);
            e.printStackTrace();
            Constants.RestoreDefaultOnScreenConfig(q3ei.arg_table, q3ei.type_table);
        }
		
		Q3EUtils.q3ei=q3ei;
	}

	@Override
	public void onAttachedToWindow() {
		super.onAttachedToWindow();
		InitUILayout(Q3EUtils.q3ei, false);
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		Q3EUtils.LoadAds(this);
		super.onConfigurationChanged(newConfig);
	}
	
	public void support(View vw)
	{
        AlertDialog.Builder bldr=new AlertDialog.Builder(this);
        bldr.setTitle("Do you want to support the developer?");
        bldr.setPositiveButton("Donate by PayPal", new AlertDialog.OnClickListener() {          
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Intent ppIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=kosleb1169%40gmail%2ecom&lc=US&item_name=n0n3m4&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_SM%2egif%3aNonHostedGuest"));
                    ppIntent.addFlags(Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    startActivity(ppIntent);                
                    dialog.dismiss();
                }
            });
        bldr.setNegativeButton("Don't ask", new AlertDialog.OnClickListener() {         
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            });
        bldr.setNeutralButton("More apps by n0n3m4", new AlertDialog.OnClickListener() {            
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Intent marketIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("market://search?q=pub:n0n3m4"));
                    marketIntent.addFlags(Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    startActivity(marketIntent);                
                    dialog.dismiss();
                }
            });
        AlertDialog dl=bldr.create();
        dl.setCancelable(false);
		dl.show();		
	}
	
	public void UpdateMouseMenu(boolean show)
	{
		V.layout_mouseconfig.setVisibility(show?LinearLayout.VISIBLE:LinearLayout.GONE);
	}
	
	public void UpdateMouseManualMenu(boolean show)
	{
		V.layout_manualmouseconfig.setVisibility(show?LinearLayout.VISIBLE:LinearLayout.GONE);
	}
	
	public void SelectCheckbox(RadioGroup rg, int index)
	{
        for(int i = 0, j = 0; i < rg.getChildCount(); i++)
        {
            View item = rg.getChildAt(i);
            if(item instanceof RadioButton)
            {
                if(j == index)
                {
                    rg.check(item.getId());
                    return;
                }
                j++;
            }
        }
		//rg.check(rg.getChildAt(index).getId());
	}


    public int GetCheckboxIndex(RadioGroup rg)
    {
        return GetCheckboxIndex(rg, rg.getCheckedRadioButtonId());
    }
	
	public int GetCheckboxIndex(RadioGroup rg, int id)
	{
        for(int i = 0, j = 0; i < rg.getChildCount(); i++)
        {
            View item = rg.getChildAt(i);
            if(item instanceof RadioButton)
            {
                if(item.getId() == id)
                {
                    return j;
                }
                j++;
            }
        }
        return -1;
		//return rg.indexOfChild(findViewById(rg.getCheckedRadioButtonId()));
	}

    public int GetCheckboxId(RadioGroup rg, int index)
    {
        for(int i = 0, j = 0; i < rg.getChildCount(); i++)
        {
            View item = rg.getChildAt(i);
            if(item instanceof RadioButton)
            {
                if(j == index)
                {
                    return item.getId();
                }
                j++;
            }
        }
        return -1;
        //return rg.getChildAt(index).getId();
	}
	
	public boolean getProp(String name)
	{
        return D3CommandUtility.GetBoolProp(GetCmdText(), name, false);
	}
    
	public void setProp(String name,boolean val)
	{
        SetProp(name, D3CommandUtility.btostr(val));
	}
	
	public void updatehacktings()
	{
        //k
		V.usedxt.setChecked(getProp("r_useDXT", false));
		V.useetc1.setChecked(getProp("r_useETC1", false));
		V.useetc1cache.setChecked(getProp("r_useETC1cache", false));
		V.nolight.setChecked(getProp("r_noLight", false));
        
        //k fill commandline
        if(!IsProp("r_useDXT")) setProp("r_useDXT", false);
        if(!IsProp("r_useETC1")) setProp("r_useETC1", false);
        if(!IsProp("r_useETC1cache")) setProp("r_useETC1cache", false);
        if(!IsProp("r_noLight")) setProp("r_noLight", false);
        
        String str = GetProp("harm_r_clearVertexBuffer");
        int index = 2;
        if(str != null)
        {
            try
            {
                index = Integer.parseInt(str);   
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
            if(index < 0 || index > 2)
                index = 2;
        }
        else
            SetProp("harm_r_clearVertexBuffer", 2);
        SelectCheckbox(V.r_harmclearvertexbuffer, index);
        if(!IsProp("harm_r_clearVertexBuffer")) SetProp("harm_r_clearVertexBuffer", 2);
        
        str = GetProp("harm_r_lightModel");
        index = 0;
        if(str != null)
        {
            if("blinn_phong".equalsIgnoreCase(str))
                index = 1;
        }
        SelectCheckbox(V.rg_harm_r_lightModel, index);
        if(!IsProp("harm_r_lightModel")) SetProp("harm_r_lightModel", "phong");
        str = GetProp("harm_r_specularExponent");
        if(null != str)
            V.edt_harm_r_specularExponent.setText(str);
        if(!IsProp("harm_r_specularExponent")) SetProp("harm_r_specularExponent", "4.0");
        
        index = 0;
        str = GetProp("fs_game");
		if(str != null)
		{
            if(!V.fs_game_user.isChecked())
            {
			    if(Q3EUtils.q3ei.isQ4)
			    {
				    if("q4base".equals(str) || "".equals(str))
					    index = 0;   
                    else
                    {
                        RemoveProp("fs_game");
                        RemoveProp("fs_game_base");
                    }
				    SelectCheckbox(V.rg_fs_q4game, index);
			    }
				else if(Q3EUtils.q3ei.isPrey)
				{
					if("preybase".equals(str) || "".equals(str))
						index = 0;
					else
					{
						RemoveProp("fs_game");
						RemoveProp("fs_game_base");
					}
					SelectCheckbox(V.rg_fs_preygame, index);
				}
			    else
			    {
				    if("".equals(str) || "base".equals(str))
					    index = 0;
				    else if("d3xp".equals(str))
					    index = 1;
				    else if("cdoom".equals(str))
					    index = 2;
				    else if("d3le".equals(str))
					    index = 3;
				    else if("rivensin".equals(str))
					    index = 4;
				    else if("hardcorps".equals(str))
					    index = 5;
                    else
                    {
                        RemoveProp("fs_game");
                        RemoveProp("fs_game_base");
                    }
                    SelectCheckbox(V.rg_fs_game, index);
			    }
            }
            else
            {
                String cur = V.edt_fs_game.getText().toString();
                if(!str.equals(cur))
                    V.edt_fs_game.setText(str);
            }
		}
        else
        {
			SelectCheckbox(GetGameModRadioGroup(), 0);
        }
        GameLauncher.this.UpdateCustomerResulotion(V.rg_scrres.getCheckedRadioButtonId() == R.id.res_custom);
	}	
    
    private LinkedList<String> m_debugTextHistory = null;
    private boolean m_revTextHistory = true;
    private void DebugText(Object format, Object...args)
    {
        String str;
        if(null == format)
            str = "NULL";
        else if(format instanceof String)
            str = String.format((String)format, args);
        else
            str = format.toString();
        Log.e("xxxxx", str);
        Toast.makeText(this, str, Toast.LENGTH_LONG).show();
        if(null == m_debugTextHistory)
            m_debugTextHistory = new LinkedList<>();
        m_debugTextHistory.add(str);
    }

	private CharSequence MakeDebugTextHistoryText(Boolean rev)
	{
		StringBuilder sb = new StringBuilder();
		final String endl = TextHelper.GetDialogMessageEndl();
		for(int i = 0; i < m_debugTextHistory.size(); i++)
		{
			sb.append("[");
			String str;
			if(rev)
			{
				sb.append(m_debugTextHistory.size() - i);
				str = m_debugTextHistory.get(m_debugTextHistory.size() - i - 1);
			}
			else
			{
				sb.append(i + 1);
				str = m_debugTextHistory.get(i);
			}
			sb.append("]: ").append(str).append(endl);
		}
		return TextHelper.GetDialogMessage(sb.toString());
	}

    private void ShowDebugTextHistoryDialog()
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Debug text history")
        .setMessage(MakeDebugTextHistoryText(m_revTextHistory))
        .setPositiveButton("OK", null)
            .setNegativeButton("Clear", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int p)
                {
                    m_debugTextHistory.clear();
                    ((AlertDialog)dialog).setMessage("");
                }
            })
            .setNeutralButton("Rev", null)
            ;
        AlertDialog dialog = builder.create();
        dialog.setOnShowListener(new DialogInterface.OnShowListener() {
                @Override
                public void onShow(final DialogInterface dialog) {
                    ((AlertDialog)dialog).getButton(DialogInterface.BUTTON_NEUTRAL).setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(View v) {
                                m_revTextHistory = !m_revTextHistory;
                                ((AlertDialog)dialog).setMessage(MakeDebugTextHistoryText(m_revTextHistory));
                            }
                        });
                }
            });
        dialog.show();
    }

    private void UpdateUserGame(boolean on)
    {
        SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(GameLauncher.this);
        String game = preference.getString(GetGameModPreferenceKey(), "");

        if(Q3EUtils.q3ei.isQ4)
        {
            int index = -1;
            if("q4base".equals(game))
                game = "";
            if(game.isEmpty() || "q4base".equals(game))
                index = 0;   
            if(index >= 0)
                SelectCheckbox(V.rg_fs_q4game, index);
            if(index > 0 && !game.isEmpty())
                SetProp("fs_game", game);   
            else
                RemoveProp("fs_game");
        }
		else if(Q3EUtils.q3ei.isPrey)
		{
			int index = -1;
			if("preybase".equals(game))
				game = "";
			if(game.isEmpty() || "preybase".equals(game))
				index = 0;
			if(index >= 0)
				SelectCheckbox(V.rg_fs_preygame, index);
			if(index > 0 && !game.isEmpty())
				SetProp("fs_game", game);
			else
				RemoveProp("fs_game");
		}
        else
        {
            int index = -1;
            if("base".equals(game))
                game = "";
            if(game.isEmpty() || "base".equals(game))
                index = 0;
            else if("d3xp".equals(game))
                index = 1;
            else if("cdoom".equals(game))
                index = 2;
            else if("d3le".equals(game))
                index = 3;
            else if("rivensin".equals(game))
                index = 4;
            else if("hardscorps".equals(game))
                index = 5;   
            if(index >= 0)
                SelectCheckbox(V.rg_fs_game, index);
            if(index > 0 && !game.isEmpty())
                SetProp("fs_game", game);   
            else
                RemoveProp("fs_game");
        }
        preference.edit().putString(Q3EUtils.pref_harm_game_lib, "").commit();
        RemoveProp("harm_fs_gameLibPath");
        if(on)
        {
            SetProp("fs_game", game);
            //RemoveProp("fs_game_base");
        }
        else
        {
            //RemoveProp("fs_game_base");
        }
        V.edt_fs_game.setText(game);
        V.rg_fs_game.setEnabled(!on);
        V.rg_fs_q4game.setEnabled(!on);
        V.fs_game_user.setText(on ? "Mod: " : "User mod");
        V.launcher_tab1_game_lib_button.setEnabled(on);
        V.edt_fs_game.setEnabled(on);
        V.launcher_tab1_user_game_layout.setVisibility(on ? View.VISIBLE : View.GONE);
    }	
	
	public void onCreate(Bundle savedInstanceState) 
	{		
		super.onCreate(savedInstanceState);				
        //k
        HandleUnexperted();
        setTitle(R.string.app_title);
		final SharedPreferences mPrefs=PreferenceManager.getDefaultSharedPreferences(this);
        ContextUtility.SetScreenOrientation(this, mPrefs.getBoolean(Constants.PreferenceKey.LAUNCHER_ORIENTATION, false) ? 0 : 1);
        
		setContentView(R.layout.main);
		
        getActionBar().setDisplayHomeAsUpEnabled(true);						
		
		InitQ3E();
		
		TabHost th=(TabHost)findViewById(R.id.tabhost);
		th.setup();					
	    th.addTab(th.newTabSpec("tab1").setIndicator("General").setContent(R.id.launcher_tab1));	    
	    th.addTab(th.newTabSpec("tab2").setIndicator("Controls").setContent(R.id.launcher_tab2));	    
	    th.addTab(th.newTabSpec("tab3").setIndicator("Graphics").setContent(R.id.launcher_tab3));							

		V.Setup();

        V.main_ad_layout.setVisibility(mPrefs.getBoolean(Constants.PreferenceKey.HIDE_AD_BAR, false) ? View.GONE : View.VISIBLE);

		SetGame(mPrefs.getString(Q3EUtils.pref_harm_game, Q3EInterface.GAME_DOOM3));
		
		V.edt_cmdline.setText(mPrefs.getString(Q3EUtils.pref_params, "game.arm"));
		V.edt_mouse.setText(mPrefs.getString(Q3EUtils.pref_eventdev, "/dev/input/event???"));
		V.edt_path.setText(mPrefs.getString(Q3EUtils.pref_datapath, default_gamedata));
		V.hideonscr.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.hideonscr.setChecked(mPrefs.getBoolean(Q3EUtils.pref_hideonscr, false));
		
		UpdateMouseMenu(V.hideonscr.isChecked());
				
		V.mapvol.setChecked(mPrefs.getBoolean(Q3EUtils.pref_mapvol, false));
		V.secfinglmb.setChecked(mPrefs.getBoolean(Q3EUtils.pref_2fingerlmb, false));
		V.smoothjoy.setChecked(mPrefs.getBoolean(Q3EUtils.pref_analog, true));
		V.detectmouse.setOnCheckedChangeListener(m_checkboxChangeListener);						
		V.detectmouse.setChecked(mPrefs.getBoolean(Q3EUtils.pref_detectmouse, true));
		
		UpdateMouseManualMenu(!V.detectmouse.isChecked());
		
		SelectCheckbox(V.rg_curpos,mPrefs.getInt(Q3EUtils.pref_mousepos, 3));
        V.rg_scrres.setOnCheckedChangeListener(m_groupCheckChangeListener);
		SelectCheckbox(V.rg_scrres,mPrefs.getInt(Q3EUtils.pref_scrres, 0));
		SelectCheckbox(V.rg_msaa,mPrefs.getInt(Q3EUtils.pref_msaa, 0));
        V.rg_msaa.setOnCheckedChangeListener(m_groupCheckChangeListener);
        //k
        V.usedxt.setChecked(mPrefs.getBoolean(Q3EUtils.pref_usedxt, false));
        V.useetc1.setChecked(mPrefs.getBoolean(Q3EUtils.pref_useetc1, false));
        V.useetc1cache.setChecked(mPrefs.getBoolean(Q3EUtils.pref_useetc1cache, false));
        V.nolight.setChecked(mPrefs.getBoolean(Q3EUtils.pref_nolight, false));
		SelectCheckbox(V.rg_color_bits, mPrefs.getInt(Q3EUtils.pref_harm_16bit, -1) + 1);
        V.rg_color_bits.setOnCheckedChangeListener(m_groupCheckChangeListener);
		SelectCheckbox(V.r_harmclearvertexbuffer, mPrefs.getInt(Q3EUtils.pref_harm_r_harmclearvertexbuffer, 2));
        V.r_harmclearvertexbuffer.setOnCheckedChangeListener(m_groupCheckChangeListener);
		SelectCheckbox(V.rg_harm_r_lightModel, "blinn_phong".equalsIgnoreCase(mPrefs.getString(Q3EUtils.pref_harm_r_lightModel, "phong")) ? 1 : 0);
        V.rg_harm_r_lightModel.setOnCheckedChangeListener(m_groupCheckChangeListener);
        V.launcher_tab2_enable_gyro.setChecked(mPrefs.getBoolean(Q3EUtils.pref_harm_view_motion_control_gyro, false));
        boolean autoQuickLoad = mPrefs.getBoolean(Q3EUtils.pref_harm_auto_quick_load, false);
		V.auto_quick_load.setChecked(autoQuickLoad);
		if(autoQuickLoad)
			SetParam_temp("loadGame", "QuickSave");
		V.edt_cmdline.setOnEditorActionListener(new TextView.OnEditorActionListener(){
           public boolean onEditorAction(TextView view, int id, KeyEvent ev)
           {
               if(ev.getKeyCode() == KeyEvent.KEYCODE_ENTER)
               {
                   if(ev.getAction() == KeyEvent.ACTION_UP)
                   {
                       V.edt_path.requestFocus();
                   }
                   return true;
               }
               return false;
           }
        });
        V.launcher_tab1_edit_autoexec.setOnClickListener(m_buttonClickListener);
        V.launcher_tab1_edit_doomconfig.setOnClickListener(m_buttonClickListener);

        boolean userMod = mPrefs.getBoolean(Q3EUtils.pref_harm_user_mod, false);
        V.fs_game_user.setChecked(userMod);
        int index = 0;
        // if(game != null)
		if(Q3EUtils.q3ei.isQ4)
		{
			String game = mPrefs.getString(Q3EUtils.pref_harm_q4_fs_game, "");
            if(game.isEmpty() || "q4base".equals(game))
                index = 0;   
            SelectCheckbox(V.rg_fs_q4game, index);
		}
		else if(Q3EUtils.q3ei.isPrey)
		{
			String game = mPrefs.getString(Q3EUtils.pref_harm_prey_fs_game, "");
			if(game.isEmpty() || "preybase".equals(game))
				index = 0;
			SelectCheckbox(V.rg_fs_preygame, index);
		}
		else
        {
			String game = mPrefs.getString(Q3EUtils.pref_harm_fs_game, "");
            if(game.isEmpty() || "base".equals(game))
                index = 0;
            else if("d3xp".equals(game))
                index = 1;
            else if("cdoom".equals(game))
                index = 2;
            else if("d3le".equals(game))
                index = 3;
            else if("rivensin".equals(game))
                index = 4;
            else if("hardscorps".equals(game))
                index = 5;
            SelectCheckbox(V.rg_fs_game, index);
        }
        UpdateUserGame(userMod);
        V.fs_game_user.setOnCheckedChangeListener(m_checkboxChangeListener);
        V.rg_fs_game.setOnCheckedChangeListener(m_groupCheckChangeListener);
        V.edt_fs_game.addTextChangedListener(new TextWatcher() {           
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    if(V.fs_game_user.isChecked())
                        SetProp("fs_game", s);
                }           
                public void beforeTextChanged(CharSequence s, int start, int count,int after) {}            
                public void afterTextChanged(Editable s)
                {
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putString(GetGameModPreferenceKey(), s.toString())
                        .commit();
                }
            });
        V.launcher_tab1_game_lib_button.setOnClickListener(m_buttonClickListener);
		V.edt_harm_r_specularExponent.setText(Float.toString(mPrefs.getFloat(Q3EUtils.pref_harm_r_specularExponent, 4.0f)));
		
		V.res_x.setText(mPrefs.getString(Q3EUtils.pref_resx, "640"));
		V.res_y.setText(mPrefs.getString(Q3EUtils.pref_resy, "480"));
        V.res_x.addTextChangedListener(new SavePreferenceTextWatcher(Q3EUtils.pref_resx, "640"));
        V.res_y.addTextChangedListener(new SavePreferenceTextWatcher(Q3EUtils.pref_resy, "480"));
        V.launcher_tab1_game_data_chooser_button.setOnClickListener(m_buttonClickListener);
        V.onscreen_button_setting.setOnClickListener(m_buttonClickListener);
		V.setup_onscreen_button_opacity.setOnClickListener(m_buttonClickListener);
		
		//DIII4A-specific					
		V.edt_cmdline.addTextChangedListener(new SavePreferenceTextWatcher(Q3EUtils.pref_params, "game.arm") {			
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                boolean cond = V.edt_cmdline.isInputMethodTarget() && !IsCmdUpdateLocked();
                if(cond)
				    updatehacktings();
			}					
            });
        V.edt_harm_r_specularExponent.addTextChangedListener(new TextWatcher() {           
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    SetProp("harm_r_specularExponent", s);
                }           
                public void beforeTextChanged(CharSequence s, int start, int count,int after) {}            
                public void afterTextChanged(Editable s) {
                    String value = s.length() == 0 ? "4.0" : s.toString();
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putFloat(Q3EUtils.pref_harm_r_specularExponent, Utility.parseFloat_s(value, 4.0f))
                        .commit();
                }
            });
		V.usedxt.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.useetc1.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.useetc1cache.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.nolight.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.smoothjoy.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.edt_path.addTextChangedListener(new SavePreferenceTextWatcher(Q3EUtils.pref_datapath, default_gamedata));
		V.edt_mouse.addTextChangedListener(new SavePreferenceTextWatcher(Q3EUtils.pref_eventdev, "/dev/input/event???"));
        V.rg_curpos.setOnCheckedChangeListener(m_groupCheckChangeListener);
		V.secfinglmb.setOnCheckedChangeListener(m_checkboxChangeListener);
		V.mapvol.setOnCheckedChangeListener(m_checkboxChangeListener);
        UpdateMapVol(V.mapvol.isChecked());
        V.launcher_tab2_volume_up_map_config_keys.setOnItemSelectedListener(m_spinnerItemSelectedListener);
        V.launcher_tab2_volume_down_map_config_keys.setOnItemSelectedListener(m_spinnerItemSelectedListener);
        V.launcher_tab2_enable_gyro.setOnCheckedChangeListener(m_checkboxChangeListener);
        V.launcher_tab2_gyro_x_axis_sens.setText("" + mPrefs.getFloat(Q3EUtils.pref_harm_view_motion_gyro_x_axis_sens, Q3EControlView.GYROSCOPE_X_AXIS_SENS));
        V.launcher_tab2_gyro_y_axis_sens.setText("" + mPrefs.getFloat(Q3EUtils.pref_harm_view_motion_gyro_y_axis_sens, Q3EControlView.GYROSCOPE_Y_AXIS_SENS));
        UpdateEnableGyro(V.launcher_tab2_enable_gyro.isChecked());
        V.launcher_tab2_gyro_x_axis_sens.addTextChangedListener(new TextWatcher() {           
                public void onTextChanged(CharSequence s, int start, int before, int count) {}           
                public void beforeTextChanged(CharSequence s, int start, int count,int after) {}            
                public void afterTextChanged(Editable s) {
                    String value = s.length() == 0 ? "" + Q3EControlView.GYROSCOPE_X_AXIS_SENS : s.toString();
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putFloat(Q3EUtils.pref_harm_view_motion_gyro_x_axis_sens, Utility.parseFloat_s(value, Q3EControlView.GYROSCOPE_Y_AXIS_SENS))
                        .commit();
                }
            });
        V.launcher_tab2_gyro_y_axis_sens.addTextChangedListener(new TextWatcher() {           
                public void onTextChanged(CharSequence s, int start, int before, int count) {}           
                public void beforeTextChanged(CharSequence s, int start, int count,int after) {}            
                public void afterTextChanged(Editable s) {
                    String value = s.length() == 0 ? "" + Q3EControlView.GYROSCOPE_Y_AXIS_SENS : s.toString();
                    PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit()
                        .putFloat(Q3EUtils.pref_harm_view_motion_gyro_y_axis_sens, Utility.parseFloat_s(value, Q3EControlView.GYROSCOPE_Y_AXIS_SENS))
                        .commit();
                }
            });
		V.auto_quick_load.setOnCheckedChangeListener(m_checkboxChangeListener);

		updatehacktings();
		
		Q3EUtils.LoadAds(this);
        
        OpenUpdate();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		StopCheckForUpdate();
	}
	
	public void start(View vw)
	{
		//k
        WritePreferences();
        /*
		String dir=V.edt_path.getText().toString();
		if ((new File(dir+"/base").exists())&&(!new File(dir+"/base/gl2progs").exists()))
		getgl2progs(dir+"/base/");
        */
        
        //k check external storage permission
        int res = ContextUtility.CheckFilePermission(this, CONST_REQUEST_EXTERNAL_STORAGE_FOR_START_RESULT_CODE);
        if(res == ContextUtility.CHECK_PERMISSION_RESULT_REJECT)
            Toast.makeText(this, "Can't start game!\nRead/Write external storage permission is not granted!", Toast.LENGTH_LONG).show();
        if(res != ContextUtility.CHECK_PERMISSION_RESULT_GRANTED)
            return;
		
		if(Q3EUtils.q3ei.isQ4 && PreferenceManager.getDefaultSharedPreferences(this).getBoolean(Constants.PreferenceKey.OPEN_QUAKE4_HELPER, true))
		{
			OpenQuake4LevelDialog();
			return;
		}
		finish();
		startActivity(new Intent(this,Q3EMain.class));
	}

	public void ResetControlsLayout(boolean safety)
	{
		InitUILayout(Q3EUtils.q3ei, safety);
		SharedPreferences.Editor mEdtr=PreferenceManager.getDefaultSharedPreferences(GameLauncher.this).edit();
		for (int i=0;i<UI_SIZE;i++)
			mEdtr.putString(Q3EUtils.pref_controlprefix+i, safety ? Q3EUtils.q3ei.defaults_table[i] : null);
		mEdtr.commit();
		Toast.makeText(GameLauncher.this, "On-screen controls has reset.", Toast.LENGTH_SHORT).show();
	}

	public void resetcontrols(View vw)
	{
        ContextUtility.Confirm(this, "Warning", "Reset on-screen controls?", new Runnable() {
            public void run() {
				ResetControlsLayout(false);
            }
            }, null, "OK(friendly)", new Runnable() {
                public void run() {
					ResetControlsLayout(true);
            }
        });
	}
	
	public void controls(View vw)
	{		
		startActivity(new Intent(this,Q3EUiConfig.class));
	}

    //k
    public boolean getProp(String name, boolean defaultValueIfNotExists)
    {
        String val = GetProp(name);
        if(val != null && !val.trim().isEmpty())
            return "1".equals(val);
        return defaultValueIfNotExists;
	}
    
    private void SetProp(String name, Object val)
    {
        boolean lock = LockCmdUpdate();
        SetCmdText(D3CommandUtility.SetProp(GetCmdText(), name, val));
        if(lock) UnlockCmdUpdate();
	}

    private String GetProp(String name)
    {
		return D3CommandUtility.GetProp(GetCmdText(), name);
	}

    private boolean RemoveProp(String name)
    {
        boolean lock = LockCmdUpdate();
		boolean[] res = { false };
		String str = D3CommandUtility.RemoveProp(GetCmdText(), name, res);
		if(res[0])
			SetCmdText(str);
        if(lock) UnlockCmdUpdate();
        return res[0];
	}

    private boolean IsProp(String name)
    {
        return D3CommandUtility.IsProp(GetCmdText(), name);
	}
    
    private void EditFile(String file)
    {
        int res = ContextUtility.CheckFilePermission(this, CONST_REQUEST_EXTERNAL_STORAGE_FOR_EDIT_CONFIG_FILE_RESULT_CODE);
        if(res == ContextUtility.CHECK_PERMISSION_RESULT_REJECT)
            Toast.makeText(this, "Can't access file!\nRead/Write external storage permission is not granted!", Toast.LENGTH_LONG).show();
        if(res != ContextUtility.CHECK_PERMISSION_RESULT_GRANTED)
            return;
        
        String gamePath = V.edt_path.getText().toString();
        String game = GetProp("fs_game");
        if(game == null || game.isEmpty())
            game = Q3EUtils.q3ei.game_base;
        String basePath = gamePath + File.separator + game + File.separator + file;
        File f = new File(basePath);
        if(!f.isFile() || !f.canWrite() || !f.canRead())
        {
            Toast.makeText(this, "File can not access(" + basePath + ")!", Toast.LENGTH_LONG).show();
            return;
        }
        
        Intent intent = new Intent(this, ConfigEditorActivity.class);
        intent.putExtra("CONST_FILE_PATH", basePath);
        startActivity(intent);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.activity_main, menu);

        if(ContextUtility.BuildIsDebug(this))
        {
            menu.findItem(R.id.main_menu_dev_menu).setVisible(true);
        }
        V.main_menu_game = menu.findItem(R.id.main_menu_game);
        V.main_menu_game.setTitle(Q3EUtils.q3ei.game_name);
        return super.onCreateOptionsMenu(menu);
    }

	@Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch(item.getItemId()) {
			case R.id.main_menu_support_developer:
				support(null);
				return true;
			case R.id.main_menu_save_settings:
				WritePreferences();
				Toast.makeText(this, "Preferences settings saved!", Toast.LENGTH_LONG).show();
				return true;
			case R.id.main_menu_changes:
				OpenChanges();
				return true;
			case R.id.main_menu_source:
				OpenAbout();
				return true;
			case R.id.main_menu_help:
				OpenHelp();
				return true;
			case R.id.main_menu_settings:
				OpenSettings();
				return true;
			case R.id.main_menu_runtime_log:
				OpenRuntimeLog();
				return true;
			case R.id.main_menu_cvar_list:
				OpenCvarListDetail();
				return true;
			case R.id.main_menu_extract_quake4_resource:
				OpenQuake4ResourceDialog();
				return true;

			case R.id.main_menu_debug:
				OpenDebugDialog();
				return true;
			case R.id.main_menu_test:
				Test();
				return true;
			case R.id.main_menu_show_preference:
				ShowPreferenceDialog();
				return true;
            case R.id.main_menu_debug_text_history:
                ShowDebugTextHistoryDialog();
				return true;
			case R.id.main_menu_check_for_update:
				OpenCheckForUpdateDialog();
				return true;

/*			case R.id.main_menu_game:
				ChangeGame();
				return true;*/
			case R.id.main_menu_game_doom3:
				ChangeGame(Q3EInterface.GAME_DOOM3);
				return true;
			case R.id.main_menu_game_quake4:
				ChangeGame(Q3EInterface.GAME_QUAKE4);
				return true;
			case R.id.main_menu_game_prey:
				ChangeGame(Q3EInterface.GAME_PREY);
				return true;

			case android.R.id.home:
				ChangeGame();
				return true;
			default:
				return super.onOptionsItemSelected(item);
        }
    }
    
    private void OpenChanges()
    {
        ContextUtility.OpenMessageDialog(this, "Changes", TextHelper.GetChangesText());
    }

    private void OpenAbout()
    {
        ContextUtility.OpenMessageDialog(this, "About " + Constants.CONST_APP_NAME + "(" + Constants.CONST_CODE + ")", TextHelper.GetAboutText(this));
    }
  
    private void HandleUnexperted()
    {
        Thread.setDefaultUncaughtExceptionHandler(new UncaughtExceptionHandler(this));
    }
    
    private void OpenRuntimeLog()
    {
        String path = V.edt_path.getText().toString() + File.separatorChar + "stdout.txt";
        String text = FileUtility.file_get_contents(path);
        if (text != null)
        {
            ContextUtility.OpenMessageDialog(this, "Last runtime log", text);
        }
        else
        {
            Toast.makeText(this, "Log file can not access(" + path + ")", Toast.LENGTH_LONG).show();
        }
    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        List<String> list = null;
        for(int i = 0; i < permissions.length; i++)
        {
            if(grantResults[i] != PackageManager.PERMISSION_GRANTED)
            {
                if(list == null)
                    list = new ArrayList<String>();
                list.add(permissions[i]);
            }
        }

		HandleGrantPermissionResult(requestCode, list);
	}
    
    private void OpenFolderChooser()
    {
        int res = ContextUtility.CheckFilePermission(this, CONST_REQUEST_EXTERNAL_STORAGE_FOR_CHOOSE_FOLDER_RESULT_CODE);
        if(res == ContextUtility.CHECK_PERMISSION_RESULT_REJECT)
            Toast.makeText(this, "Can't choose folder!\nRead/Write external storage permission is not granted!", Toast.LENGTH_LONG).show();
        if(res != ContextUtility.CHECK_PERMISSION_RESULT_GRANTED)
            return;

        String defaultPath = Environment.getExternalStorageDirectory().getAbsolutePath(); //System.getProperty("user.home");
        String gamePath = V.edt_path.getText().toString();
        if(null == gamePath || gamePath.isEmpty())
            gamePath = defaultPath;
        File f = new File(gamePath);
        if(!f.exists())
        {
            gamePath = defaultPath;
            f = new File(gamePath);
        }
        if(!f.isDirectory())
        {
            gamePath = f.getParent();
            f = f.getParentFile();
        }
        if(!f.canRead())
        {
            gamePath = defaultPath;
            f = new File(gamePath);
        }
        
        FileBrowserDialog dialog = new FileBrowserDialog(this);
        dialog.SetupUI("Chooser data folder", gamePath);
        dialog.setButton(AlertDialog.BUTTON_NEGATIVE, "Cancel", new AlertDialog.OnClickListener() {          
                @Override
                public void onClick(DialogInterface dialog, int which) { 
                    dialog.dismiss();
                }
            });
        dialog.setButton(AlertDialog.BUTTON_POSITIVE, "Choose current directory", new AlertDialog.OnClickListener() {          
                @Override
                public void onClick(DialogInterface dialog, int which) { 
					V.edt_path.setText(((FileBrowserDialog)dialog).Path());
                    dialog.dismiss();
                }
            });
            
        dialog.show();
    }
    
    private void WritePreferences()
    {
        SharedPreferences.Editor mEdtr=PreferenceManager.getDefaultSharedPreferences(this).edit();
        mEdtr.putString(Q3EUtils.pref_params, GetCmdText());
        mEdtr.putString(Q3EUtils.pref_eventdev, V.edt_mouse.getText().toString());
        mEdtr.putString(Q3EUtils.pref_datapath, V.edt_path.getText().toString());
        mEdtr.putBoolean(Q3EUtils.pref_hideonscr, V.hideonscr.isChecked());
        //k mEdtr.putBoolean(Q3EUtils.pref_32bit, true);
        int index = GetCheckboxIndex(V.rg_color_bits) - 1;
        mEdtr.putBoolean(Q3EUtils.pref_32bit, index == -1);
        mEdtr.putInt(Q3EUtils.pref_harm_16bit, index);
        mEdtr.putInt(Q3EUtils.pref_harm_r_harmclearvertexbuffer, GetCheckboxIndex(V.r_harmclearvertexbuffer));
        mEdtr.putString(Q3EUtils.pref_harm_r_lightModel, GetCheckboxIndex(V.rg_harm_r_lightModel) == 1 ? "blinn_phong" : "phong");
		mEdtr.putFloat(Q3EUtils.pref_harm_r_specularExponent, Utility.parseFloat_s(V.edt_harm_r_specularExponent.getText().toString(), 4.0f));

        mEdtr.putBoolean(Q3EUtils.pref_mapvol, V.mapvol.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_analog, V.smoothjoy.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_2fingerlmb, V.secfinglmb.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_detectmouse, V.detectmouse.isChecked());
        mEdtr.putInt(Q3EUtils.pref_mousepos, GetCheckboxIndex(V.rg_curpos));
        mEdtr.putInt(Q3EUtils.pref_scrres, GetCheckboxIndex(V.rg_scrres));
        mEdtr.putInt(Q3EUtils.pref_msaa, GetCheckboxIndex(V.rg_msaa));
        mEdtr.putString(Q3EUtils.pref_resx, V.res_x.getText().toString());
        mEdtr.putString(Q3EUtils.pref_resy, V.res_y.getText().toString());
        mEdtr.putBoolean(Q3EUtils.pref_useetc1cache, V.useetc1cache.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_useetc1, V.useetc1.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_usedxt, V.usedxt.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_nolight, V.nolight.isChecked());
        mEdtr.putBoolean(Q3EUtils.pref_harm_user_mod, V.fs_game_user.isChecked());
        mEdtr.putString(Q3EUtils.pref_harm_game, Q3EUtils.q3ei.game);
        mEdtr.putBoolean(Q3EUtils.pref_harm_view_motion_control_gyro, V.launcher_tab2_enable_gyro.isChecked());
		mEdtr.putFloat(Q3EUtils.pref_harm_view_motion_gyro_x_axis_sens, Utility.parseFloat_s(V.launcher_tab2_gyro_x_axis_sens.getText().toString(), Q3EControlView.GYROSCOPE_X_AXIS_SENS));
		mEdtr.putFloat(Q3EUtils.pref_harm_view_motion_gyro_y_axis_sens, Utility.parseFloat_s(V.launcher_tab2_gyro_y_axis_sens.getText().toString(), Q3EControlView.GYROSCOPE_Y_AXIS_SENS));
		mEdtr.putBoolean(Q3EUtils.pref_harm_auto_quick_load, V.auto_quick_load.isChecked());
		mEdtr.commit();
    }

    private void OpenHelp()
    {
        ContextUtility.OpenMessageDialog(this, "Help", TextHelper.GetHelpText());
    }
    
    private void OpenUpdate()
    {       
        if(IsUpdateRelease())
            ContextUtility.OpenMessageDialog(this, "Update: " + Constants.CONST_APP_NAME + "(" + Constants.CONST_CODE + ")", TextHelper.GetUpdateText(this));
    }

    private boolean IsUpdateRelease()
    {
        final String UPDATE_RELEASE = "UPDATE_RELEASE";
        SharedPreferences pref =  PreferenceManager.getDefaultSharedPreferences(this);
        int r = pref.getInt(UPDATE_RELEASE, 0);
        if(r == Constants.CONST_UPDATE_RELEASE)
            return false;
        pref.edit().putInt(UPDATE_RELEASE, Constants.CONST_UPDATE_RELEASE).commit();
        return true;
    }
    
    private void SetCmdText(String text)
    {
        EditText edit = V.edt_cmdline;
        if(edit.getText().toString().equals(text))
            return;
        int pos = edit.getSelectionStart();
        edit.setText(text);
        if(text != null && !text.isEmpty())
        {
            pos = Math.max(0, Math.min(pos, text.length()));
            try
            {
                edit.setSelection(pos);      
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
        }
    }

    private String GetCmdText()
    {
        return V.edt_cmdline.getText().toString();
    }
    
    private void OpenGameLibChooser()
    {
		final SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(this);
        final String libPath = ContextUtility.NativeLibDir(this) + "/";
		final String[] Libs = Q3EUtils.q3ei.libs;
		final String PreferenceKey = GetGameModLibPreferenceKey();
        final String[] items = new String[Libs.length];
        String lib = preference.getString(PreferenceKey, "");
        int selected = -1;
        for(int i = 0; i < Libs.length; i++)
        {
            items[i] = "lib" + Libs[i] + ".so";
            if((libPath + items[i]).equals(lib))
            {
                selected = i;
            }
        }
        
        StringBuilder sb = new StringBuilder();
        if(Q3EJNI.IS_64)
            sb.append("armv8-a 64");
        else
            sb.append("armv7-a neon");
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(Q3EUtils.q3ei.game_name + " game library(" + sb.toString() + ")");
        builder.setSingleChoiceItems(items, selected, new DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int p)
            {
                String lib = libPath + items[p];
                preference.edit().putString(PreferenceKey, lib).commit();
                SetProp("harm_fs_gameLibPath", lib);
                dialog.dismiss();
            }
        });
        builder.setNeutralButton("Unset", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int p)
            {
                preference.edit().putString(PreferenceKey, "").commit();
                RemoveProp("harm_fs_gameLibPath");
                dialog.dismiss();
            }
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }
    
    private void Test()
    {
        //((Object)null).toString();
    }

    private void OpenSettings()
    {
        LauncherSettingsDialog dialog = LauncherSettingsDialog.newInstance();
        dialog.show(getFragmentManager(), "LauncherSettingsDialog");
    }
    
    private void OpenDebugDialog()
    {
        DebugDialog dialog = DebugDialog.newInstance();
        dialog.show(getFragmentManager(), "DebugDialog");
    }
    
    private void UpdateCustomerResulotion(boolean enabled)
    {
        LinearLayout layout = V.res_customlayout;
        final int count = layout.getChildCount();
        for(int i = 0; i < count; i++)
        {
            layout.getChildAt(i).setEnabled(enabled);
        }
        
        layout.setEnabled(enabled);
    }

	private void SetGameDLL(int val)
	{
        SharedPreferences preference =  PreferenceManager.getDefaultSharedPreferences(this);
        boolean userMod = V.fs_game_user.isChecked(); //preference.getBoolean(Q3EUtils.pref_harm_user_mod, false);
        String game = "";
		switch(val)
		{
			case R.id.fs_game_base:
                if(!userMod)
                {
                    RemoveProp("fs_game");
                    RemoveProp("fs_game_base");
                    RemoveProp("harm_fs_gameLibPath");
                }
				break;
			case R.id.fs_game_d3xp:
                if(!userMod)
                {
				    SetProp("fs_game", "d3xp");
				    RemoveProp("fs_game_base");
				    RemoveProp("harm_fs_gameLibPath");
                }
                game = "d3xp";
				break;
			case R.id.fs_game_cdoom:
                if(!userMod)
                {
				    SetProp("fs_game", "cdoom");
				    RemoveProp("fs_game_base");
				    RemoveProp("harm_fs_gameLibPath");
                }
                game = "cdoom";
				break;
			case R.id.fs_game_lost_mission:
                if(!userMod)
                {
				    SetProp("fs_game", "d3le");
				    SetProp("fs_game_base", "d3xp"); // must load d3xp pak
				    RemoveProp("harm_fs_gameLibPath");
                }
                game = "d3le";
				break;
			case R.id.fs_game_rivensin:
                if(!userMod)
                {
				    SetProp("fs_game", "rivensin");
				    RemoveProp("fs_game_base");
				    RemoveProp("harm_fs_gameLibPath");
                }
                game = "rivensin";
				break;
            case R.id.fs_game_hardcorps:
                if(!userMod)
                {
                    SetProp("fs_game", "hardcorps");
                    RemoveProp("fs_game_base");
                    RemoveProp("harm_fs_gameLibPath");
                }
                game = "hardcorps";
				break;
                
            case R.id.fs_game_quake4:
                if(!userMod)
                {
                    RemoveProp("fs_game");
                    RemoveProp("fs_game_base");
                    RemoveProp("harm_fs_gameLibPath");
                }
				break;

			case R.id.fs_game_prey:
				if(!userMod)
				{
					RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
				}
				break;
			default:
				break;
		}
        if(userMod)
        {
            SetProp("fs_game", game);
            RemoveProp("fs_game_base");
            RemoveProp("harm_fs_gameLibPath");
            V.edt_fs_game.setText(game);
        }
        preference.edit().putString(GetGameModPreferenceKey(), game).commit();
	}

    private boolean LockCmdUpdate()
    {
        boolean res = m_cmdUpdateLock;
        m_cmdUpdateLock = true;
        return !res;
    }

    private void UnlockCmdUpdate()
    {
        m_cmdUpdateLock = false;
    }

    private boolean IsCmdUpdateLocked()
    {
        return m_cmdUpdateLock;
    }

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		// Android SDK > 28
/*		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) // Android 11 FS permission
		{
			if(requestCode == CONST_REQUEST_EXTERNAL_STORAGE_FOR_START_RESULT_CODE
			|| requestCode == CONST_REQUEST_EXTERNAL_STORAGE_FOR_CHOOSE_FOLDER_RESULT_CODE
			|| requestCode == CONST_REQUEST_EXTERNAL_STORAGE_FOR_EDIT_CONFIG_FILE_RESULT_CODE)
			if (!Environment.isExternalStorageManager()) // reject
			{
				List<String> list = Collections.singletonList(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
				HandleGrantPermissionResult(requestCode, list);
			}
		}*/
	}

	private void HandleGrantPermissionResult(int requestCode, List<String> list)
	{
		if(null == list || list.isEmpty())
			return;
		String opt;
		switch(requestCode)
		{
			case CONST_REQUEST_EXTERNAL_STORAGE_FOR_START_RESULT_CODE:
				opt = "Start game";
				break;
			case CONST_REQUEST_EXTERNAL_STORAGE_FOR_EDIT_CONFIG_FILE_RESULT_CODE:
				opt = "Edit config file";
				break;
			case CONST_REQUEST_EXTERNAL_STORAGE_FOR_CHOOSE_FOLDER_RESULT_CODE:
				opt = "Choose game folder";
				break;
			default:
				opt = "Operation";
				break;
		}
		StringBuilder sb = new StringBuilder();
		String endl = TextHelper.GetDialogMessageEndl();
		for(String str : list)
		{
			if(str != null)
				sb.append("  * " + str);
			sb.append(endl);
		}
		AlertDialog.Builder builder = ContextUtility.CreateMessageDialogBuilder(this, opt + " request necessary permissions", TextHelper.GetDialogMessage(sb.toString()));
		builder.setNeutralButton("Grant", new AlertDialog.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				ContextUtility.OpenAppSetting(GameLauncher.this);
				dialog.dismiss();
			}
		});
		builder.create().show();
	}
    
    private void OpenOnScreenButtonSetting()
    {
        Intent intent = new Intent(this, OnScreenButtonConfigActivity.class);
        startActivity(intent);
    }

    private void OpenCvarListDetail()
    {
        ContextUtility.OpenMessageDialog(this, "Special Cvar List", TextHelper.GetCvarText());
    }
    
    private void SetGame(String game)
    {
		Q3EUtils.q3ei.SetupGame(game); //k armv7-a only support neon now
        V.launcher_tab1_edit_doomconfig.setText("Edit " + Q3EUtils.q3ei.config_name);
        if(null != V.main_menu_game)
            V.main_menu_game.setTitle(Q3EUtils.q3ei.game_name);
        ActionBar actionBar = getActionBar();
		Resources res = getResources();
		int colorId;
		int iconId;
		boolean d3Visible = false;
		boolean q4Visible = false;
		boolean preyVisible = false;
		if(Q3EUtils.q3ei.isPrey)
		{
			colorId = R.color.theme_prey_main_color;
			iconId = R.drawable.prey_icon;
			preyVisible = true;
		}
		else if(Q3EUtils.q3ei.isQ4)
		{
			colorId = R.color.theme_quake4_main_color;
			iconId = R.drawable.q4_icon;
			q4Visible = true;
		}
		else
		{
			colorId = R.color.theme_doom3_main_color;
			iconId = R.drawable.d3_icon;
			d3Visible = true;
		}
        actionBar.setBackgroundDrawable(new ColorDrawable(res.getColor(colorId)));
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
			actionBar.setHomeAsUpIndicator(iconId);
		}
		V.rg_fs_game.setVisibility(d3Visible ? View.VISIBLE : View.GONE);
        V.rg_fs_q4game.setVisibility(q4Visible ? View.VISIBLE : View.GONE);
		V.rg_fs_preygame.setVisibility(preyVisible ? View.VISIBLE : View.GONE);
    }
    
    private void ChangeGame(String...games)
    {
    	String newGame = games.length > 0 ? games[0] : null;
    	if(null == newGame || newGame.isEmpty())
		{
			final String[] Games = {
					Q3EInterface.GAME_DOOM3,
					Q3EInterface.GAME_QUAKE4,
					Q3EInterface.GAME_PREY,
			};
			int i;
			for(i = 0; i < Games.length; i++)
			{
				if(Games[i].equalsIgnoreCase(Q3EUtils.q3ei.game))
					break;
			}
			if(i >= Games.length)
				i = Games.length - 1;
			newGame = Games[(i + 1) % 3];
		}
        SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(this);
        preference.edit().putString(Q3EUtils.pref_harm_game, newGame).commit();
        SetGame(newGame);
        preference.edit().putString(Q3EUtils.pref_harm_game_lib, "");

        String game = preference.getString(GetGameModPreferenceKey(), "");
        V.edt_fs_game.setText(game);
        boolean userMod = preference.getBoolean(Q3EUtils.pref_harm_user_mod, false);
		if(Q3EUtils.q3ei.isQ4)
		{
			int index = 0;
			switch(game)
			{
				case "q4base":
					// SetProp("fs_game", "q4base");
					RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 0;
					break;
				default:
					if(userMod && !game.isEmpty())
						SetProp("fs_game", game);
					else
						RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					break;
			}
			SelectCheckbox(V.rg_fs_q4game, index);
		}
		else if(Q3EUtils.q3ei.isPrey)
		{
			int index = 0;
			switch(game)
			{
				case "q4base":
					// SetProp("fs_game", "preybase");
					RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 0;
					break;
				default:
					if(userMod && !game.isEmpty())
						SetProp("fs_game", game);
					else
						RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					break;
			}
			SelectCheckbox(V.rg_fs_preygame, index);
		}
		else
		{
			int index = 0;
			switch(game)
			{
				case "base":
				case "":
					RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 0;
					break;
				case "d3xp":
					SetProp("fs_game", "d3xp");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 1;
					break;
				case "cdoom":
					SetProp("fs_game", "cdoom");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 2;
					break;
				case "d3le":
					SetProp("fs_game", "d3le");
					SetProp("fs_game_base", "d3xp"); // must load d3xp pak
					RemoveProp("harm_fs_gameLibPath");
					index = 3;
					break;
				case "rivensin":
					SetProp("fs_game", "rivensin");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 4;
					break;
				case "hardcorps":
					SetProp("fs_game", "hardcorps");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					index = 5;
					break;
				default:
					if(userMod && !game.isEmpty())
						SetProp("fs_game", game);
					else
						RemoveProp("fs_game");
					RemoveProp("fs_game_base");
					RemoveProp("harm_fs_gameLibPath");
					break;
			}
			SelectCheckbox(V.rg_fs_game, index);
		}
    }
    
    private void OpenQuake4LevelDialog()
    {
        final int[] Acts = {
          5, 7, 6, 11, 2,
        };
        final String[] Act_Names = {
            "I", "II", "III", "IV", "V",
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        String[] levels = new String[Q3EInterface.QUAKE4_LEVELS.length];
        int m = 0;
        int n = 0;
        for(int i = 0; i < Q3EInterface.QUAKE4_LEVELS.length; i++)
        {
            if(n >= Acts[m])
            {
                n = 0;
                m++;
            }
            n++;
            levels[i] = String.format("%s%d.Act %s - %s(%s)", (i < 9 ? " " : ""), (i + 1), Act_Names[m], Q3EInterface.QUAKE4_LEVELS[i], Q3EInterface.QUAKE4_MAPS[i]);
        }
		final AlertDialog dialog = builder.setTitle("Quake 4 Level")
				.setItems(levels, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int p) {
						GameLauncher.this.RemoveParam_temp("loadGame");
						GameLauncher.this.SetParam_temp("map", "game/" + Q3EInterface.QUAKE4_MAPS[p]);
						finish();
						startActivity(new Intent(GameLauncher.this, Q3EMain.class));
					}
				})
				.setPositiveButton("Start", new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int p) {
						finish();
						startActivity(new Intent(GameLauncher.this, Q3EMain.class));
					}
				})
				.setNegativeButton("Cancel", null)
				.setNeutralButton("Extract resource", null)
				.create();
		dialog.setOnShowListener(new DialogInterface.OnShowListener() {
			@Override
			public void onShow(DialogInterface d) {
				dialog.getButton(DialogInterface.BUTTON_NEUTRAL).setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						OpenQuake4ResourceDialog();
					}
				});
			}
		});
		dialog.show();
    }

    //private static final int Q4_RESOURCE_FONT = 1;
    private static final int Q4_RESOURCE_BOT = 1 << 1;
    private static final int Q4_RESOURCE_ALL = ~(1 << 31);
    
    private void OpenQuake4ResourceDialog()
    {
    	// D3-format fonts don't need on longer
        final int[] Types = {
            //Q4_RESOURCE_FONT,
            Q4_RESOURCE_BOT,
        };
        final String[] Names = {
            // "Font(D3 format)",
            "Bot(Q3 support in MP game)",
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Quake 4 extra patch resource")
            .setItems(Names, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int p)
                {
					ExtractQuake4PatchResource(Types[p]);
                }
            })
            .setNegativeButton("Cancel", null)
            .setPositiveButton("Extract all", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int p)
                {
					ExtractQuake4PatchResource(Q4_RESOURCE_ALL);
                }
            })
            .create()
            .show()
            ;
    }
    
    private boolean ExtractQuake4PatchResource(int mask)
    {
    	if(0 == mask)
    		return false;

		int res = ContextUtility.CheckFilePermission(this, CONST_REQUEST_EXTERNAL_STORAGE_FOR_EDIT_CONFIG_FILE_RESULT_CODE);
		if(res == ContextUtility.CHECK_PERMISSION_RESULT_REJECT)
			Toast.makeText(this, "Can't access file!\nRead/Write external storage permission is not granted!", Toast.LENGTH_LONG).show();
		if(res != ContextUtility.CHECK_PERMISSION_RESULT_GRANTED)
			return false;

		String gamePath = V.edt_path.getText().toString();
		final String BasePath = gamePath + File.separator;
    	StringBuilder sb = new StringBuilder();
    	boolean r = true;
/*        if(Utility.MASK(mask, Q4_RESOURCE_FONT))
		{
			String fileName = "q4base/q4_fonts_idtech4amm.pk4";
			String outPath = BasePath + fileName;
			boolean ok = ContextUtility.ExtractAsset(this, "pak/q4base/fonts_d3format.pk4", outPath);
			sb.append("Extract Quake 4 fonts(DOOM3 format) patch file to ").append(fileName).append(" ");
			sb.append(ok ? "success" : "fail");
			r = r && ok;
		}*/
        if(Utility.MASK(mask, Q4_RESOURCE_BOT))
		{
			String fileName = "q4base/q4_botfiles_idtech4amm.pk4";
			String outPath = BasePath + fileName;
			boolean ok = ContextUtility.ExtractAsset(this, "pak/q4base/botfiles_q3.pk4", outPath);
			if(sb.length() > 0)
				sb.append("\n");
			sb.append("Extract Quake 4 bot files(Quake3) patch file to ").append(fileName).append(" ");
			sb.append(ok ? "success" : "fail");
			r = r && ok;
		}
        Toast.makeText(this, sb.toString(), Toast.LENGTH_SHORT).show();
        return r;
    }

    private boolean RemoveParam(String name)
    {
        boolean lock = LockCmdUpdate();
		boolean[] res = { false };
		String str = D3CommandUtility.RemoveParam(GetCmdText(), name, res);
		if(res[0])
			SetCmdText(str);
        if(lock) UnlockCmdUpdate();
        return res[0];
	}

    private void SetParam(String name, Object val)
    {
        boolean lock = LockCmdUpdate();
        SetCmdText(D3CommandUtility.SetParam(GetCmdText(), name, val));
        if(lock) UnlockCmdUpdate();
	}

    private String GetParam(String name)
    {
		return D3CommandUtility.GetParam(GetCmdText(), name);
	}

	private boolean RemoveParam_temp(String name)
	{
		boolean[] res = { false };
		String str = D3CommandUtility.RemoveParam(Q3EUtils.q3ei.start_temporary_extra_command, name, res);
		if(res[0])
			Q3EUtils.q3ei.start_temporary_extra_command = str;
		return res[0];
	}

	private void SetParam_temp(String name, Object val)
	{
		Q3EUtils.q3ei.start_temporary_extra_command = (D3CommandUtility.SetParam(Q3EUtils.q3ei.start_temporary_extra_command, name, val));
	}
    
    private void ShowPreferenceDialog()
    {
        StringBuilder sb = new StringBuilder();
        SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(this);
        Map<String, ?> map = preference.getAll();
        final String endl = TextHelper.GetDialogMessageEndl();
        int i = 0;
        for(Map.Entry<String, ?>  entry : map.entrySet())
        {
            sb.append(i++).append(". ").append(entry.getKey());
            sb.append(": ").append(entry.getValue());
            sb.append(endl);
        }
        ContextUtility.OpenMessageDialog(this, "Shared preferences", TextHelper.GetDialogMessage(sb.toString()));
    }
    
    private void UpdateMapVol(boolean on)
    {
        SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(this);
        int[] keyCodes = getResources().getIntArray(R.array.key_map_codes);
        V.launcher_tab2_volume_map_config_layout.setVisibility(on ? View.VISIBLE : View.GONE);
        int key = preference.getInt(Constants.PreferenceKey.VOLUME_UP_KEY, Q3EKeyCodes.KeyCodes.K_F3);
        V.launcher_tab2_volume_up_map_config_keys.setSelection(Utility.ArrayIndexOf(keyCodes, key));
        key = preference.getInt(Constants.PreferenceKey.VOLUME_DOWN_KEY, Q3EKeyCodes.KeyCodes.K_F2);
        V.launcher_tab2_volume_down_map_config_keys.setSelection(Utility.ArrayIndexOf(keyCodes, key));
    }

    private void UpdateEnableGyro(boolean on)
    {
        V.launcher_tab2_enable_gyro_layout.setVisibility(on ? View.VISIBLE : View.GONE);
        SharedPreferences preference = PreferenceManager.getDefaultSharedPreferences(this);
        float f = preference.getFloat(Q3EUtils.pref_harm_view_motion_gyro_x_axis_sens, Q3EControlView.GYROSCOPE_X_AXIS_SENS);
        V.launcher_tab2_gyro_x_axis_sens.setText("" + f);
        f = preference.getFloat(Q3EUtils.pref_harm_view_motion_gyro_y_axis_sens, Q3EControlView.GYROSCOPE_Y_AXIS_SENS);
        V.launcher_tab2_gyro_y_axis_sens.setText("" + f);
    }

    private void SetupOnScreenButtonOpacity(int alpha)
	{
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(GameLauncher.this);
		SharedPreferences.Editor mEdtr = preferences.edit();
		for (int i = 0; i < UI_SIZE; i++)
		{
			String str = Q3EUtils.q3ei.defaults_table[i];
			int index = str.lastIndexOf(' ');
			str = str.substring(0, index) + ' ' + alpha;
			Q3EUtils.q3ei.defaults_table[i] = str;

			String key = Q3EUtils.pref_controlprefix + i;
			if(!preferences.contains(key))
				continue;
			str = preferences.getString(key, Q3EUtils.q3ei.defaults_table[i]);
			index = str.lastIndexOf(' ');
			str = str.substring(0, index) + ' ' + alpha;
			mEdtr.putString(key, str);
		}
		mEdtr.commit();
		Toast.makeText(GameLauncher.this, "Setup on-screen buttons opacity done.", Toast.LENGTH_SHORT).show();
	}

    private void OpenOnScreenButtonOpacitySetting()
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle("Setup on-screen button opacity");
		View seekBarWidget = getLayoutInflater().inflate(R.layout.seek_bar_dialog_preference, null, false);
		builder.setView(seekBarWidget);
		final SeekBar seekBar = seekBarWidget.findViewById(R.id.seek_bar_dialog_preference_layout_seekbar);
		final TextView min = seekBarWidget.findViewById(R.id.seek_bar_dialog_preference_layout_min);
		final TextView max = seekBarWidget.findViewById(R.id.seek_bar_dialog_preference_layout_max);
		final TextView progress = seekBarWidget.findViewById(R.id.seek_bar_dialog_preference_layout_progress);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			seekBar.setMin(0);
		}
		seekBar.setMax(100);
		seekBar.setProgress(CONST_DEFAULT_ON_SCREEN_BUTTON_OPACITY);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
			min.setText("" + seekBar.getMin());
		else
			min.setText("0");
		max.setText("" + seekBar.getMax());
		progress.setText("" + seekBar.getProgress());
		seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
			public void onProgressChanged(SeekBar seekBar, int p, boolean fromUser)
			{
				progress.setText("" + p);
			}
			public void onStartTrackingTouch(SeekBar seekBar)
			{
				progress.setTextColor(Color.RED);
			}
			public void onStopTrackingTouch(SeekBar seekBar)
			{
				progress.setTextColor(Color.BLACK);
			}
		});
		builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				SetupOnScreenButtonOpacity(seekBar.getProgress());
			}
		})
				.setNeutralButton("Reset", new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						SetupOnScreenButtonOpacity(CONST_DEFAULT_ON_SCREEN_BUTTON_OPACITY);
					}
				})
				.setNegativeButton("Cancel", null);
		AlertDialog dialog = builder.create();
		dialog.show();
	}

	private ProgressDialog m_progressDialog = null;
    private HandlerThread m_handlerThread = null;
    private Handler m_handler = null;

    private void OpenCheckForUpdateDialog()
	{
		if(null != m_handlerThread || null != m_handler || null != m_progressDialog)
			return;
		ProgressDialog dialog = new ProgressDialog(this);
		dialog.setTitle("Check for update");
		dialog.setMessage("Network for GitHub......");
		dialog.setCancelable(false);
		dialog.setButton(DialogInterface.BUTTON_NEGATIVE, "Cancel", new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				dialog.cancel();
			}
		});
		dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
			@Override
			public void onDismiss(DialogInterface dialog) {
				m_progressDialog = null;
				StopCheckForUpdate();
			}
		});
		dialog.show();
		m_progressDialog = dialog;
		m_handlerThread = new HandlerThread("CHECK_FOR_UPDATE");
		m_handlerThread.start();
		m_handler = new Handler(m_handlerThread.getLooper());
		m_handler.post(new Runnable() {
			@Override
			public void run() {
				CheckForUpdate();
			}
		});
	}

	// non-main thread
	private void CheckForUpdate()
	{
		final int TimeOut = 60000;
		HttpsURLConnection conn = null;
		InputStream inputStream = null;
		OutputStream outputStream = null;
		final String[] error = { "Unknown error" };

		try
		{
			URL url = new URL(Constants.CONST_CHECK_FOR_UPDATE_URL);
			conn = (HttpsURLConnection)url.openConnection();
			conn.setRequestMethod("GET");
			conn.setConnectTimeout(TimeOut);
			conn.setInstanceFollowRedirects(true);
			SSLContext sc = SSLContext.getInstance("TLS");
			sc.init(null, new TrustManager[]{
					new X509TrustManager() {
						@Override
						public X509Certificate[] getAcceptedIssuers() {
							return new X509Certificate[]{};
						}
						@Override
						public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException { }
						@Override
						public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException { }
					}
				}, new java.security.SecureRandom());
			SSLSocketFactory newFactory = sc.getSocketFactory();
			conn.setSSLSocketFactory(newFactory);
			conn.setHostnameVerifier(new HostnameVerifier() {
				@Override
				public boolean verify(String hostname, SSLSession session) {
					return true;
				}
			});
			conn.setDoInput(true); // 总是读取结果
			conn.setUseCaches(false);
			conn.connect();

			int respCode = conn.getResponseCode();
			if(respCode == HttpsURLConnection.HTTP_OK)
			{
				inputStream = conn.getInputStream();
				byte[] data = FileUtility.readStream(inputStream);
				if(null != data && data.length > 0)
				{
					String text = new String(data);
					JSONObject json = new JSONObject(text);
					final int release = json.getInt("release");
					final String update = json.getString("update");
					final String version = json.getString("version");
					final String apk_url = json.getString("apk_url");
					final String changes = json.getString("changes");
					runOnUiThread(new Runnable() {
						@Override
						public void run() {
							OpenCheckForUpdateResult(release, version, update, apk_url, changes);
						}
					});
					return;
				}
				error[0] = "Empty response data";
			}
			else
				error[0] = "Network unexpected response: " + respCode;
		}
		catch(Exception e)
		{
			e.printStackTrace();
			error[0] = e.getMessage();
		}
		finally {
			FileUtility.CloseStream(inputStream);
			FileUtility.CloseStream(outputStream);
		}

		runOnUiThread(new Runnable() {
			@Override
			public void run() {
				OpenCheckForUpdateResult(-1, null, null, null, error[0]);
			}
		});
	}

	private void OpenCheckForUpdateResult(int release, String version, String update, final String apk_url, String changes)
	{
		StopCheckForUpdate();
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		if(release <= 0)
		{
			builder.setTitle("Error")
					.setMessage(changes)
			.setNegativeButton("Close", null)
			;
		}
		else if(release > Constants.CONST_UPDATE_RELEASE)
		{
			StringBuilder sb = new StringBuilder();
			final String endl = TextHelper.GetDialogMessageEndl();
			sb.append("Version: ").append(version).append(endl);
			sb.append("Update: ").append(update).append(endl);
			sb.append("Changes: ").append(endl);
			if(null != changes && !changes.isEmpty())
			{
				String[] changesArr = changes.split("\n");
				for(String str : changesArr)
				{
					if(null != str)
						sb.append(str);
					sb.append(endl);
				}
			}
			CharSequence msg = TextHelper.GetDialogMessage(sb.toString());
			builder.setTitle("New update release(" + release + ")")
					.setMessage(msg)
			.setPositiveButton("Download", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					ContextUtility.OpenUrlExternally(GameLauncher.this, apk_url);
				}
			})
			.setNeutralButton("View", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					ContextUtility.OpenUrlExternally(GameLauncher.this, Constants.CONST_MAIN_PAGE);
				}
			})
			.setNegativeButton("Cancel", null)
			;
		}
		else
		{
			builder.setTitle("No update release")
					.setMessage("Current version is newest.")
					.setPositiveButton("OK", null)
			;
		}
		AlertDialog dialog = builder.create();
		dialog.show();
	}

	private void StopCheckForUpdate()
	{
		if(null != m_handler)
		{
			m_handler = null;
		}
		if(null != m_handlerThread)
		{
			m_handlerThread.quit();
			m_handlerThread = null;
		}
		if(null != m_progressDialog)
		{
			m_progressDialog.dismiss();
			m_progressDialog = null;
		}
	}

	private String GetGameModPreferenceKey()
	{
		return Q3EUtils.q3ei.isPrey ? Q3EUtils.pref_harm_prey_fs_game
				: (Q3EUtils.q3ei.isQ4 ? Q3EUtils.pref_harm_q4_fs_game
				: Q3EUtils.pref_harm_fs_game);
	}

	private String GetGameModLibPreferenceKey()
	{
		return Q3EUtils.q3ei.isPrey ? Q3EUtils.pref_harm_prey_game_lib
				: (Q3EUtils.q3ei.isQ4 ? Q3EUtils.pref_harm_q4_game_lib
				: Q3EUtils.pref_harm_game_lib);
	}

	private RadioGroup GetGameModRadioGroup()
	{
		return Q3EUtils.q3ei.isPrey ? V.rg_fs_preygame
				: (Q3EUtils.q3ei.isQ4 ? V.rg_fs_q4game
				: V.rg_fs_game);
	}


	private class ViewHolder
	{
        public MenuItem main_menu_game;
		public EditText edt_cmdline;
		public LinearLayout res_customlayout;
		public CheckBox nolight;
		public CheckBox useetc1cache;
		public CheckBox useetc1;
        public CheckBox usedxt;
		public RadioGroup r_harmclearvertexbuffer;
		public RadioGroup rg_scrres;
		public RadioGroup rg_msaa;
		public RadioGroup rg_color_bits;
		public RadioGroup rg_fs_game;
		public EditText edt_fs_game;
		public EditText edt_mouse;
		public EditText edt_path;
		public CheckBox hideonscr;
		public CheckBox mapvol;
		public CheckBox secfinglmb;
		public CheckBox smoothjoy;
		public CheckBox detectmouse;
		public LinearLayout layout_mouseconfig;
		public LinearLayout layout_manualmouseconfig;
		public Button launcher_tab1_game_lib_button;
		public EditText res_x;
		public EditText res_y;
		public Button launcher_tab1_edit_autoexec;
		public Button launcher_tab1_edit_doomconfig;
		public Button launcher_tab1_game_data_chooser_button;
		public RadioGroup rg_curpos;
		public EditText edt_harm_r_specularExponent;
		public RadioGroup rg_harm_r_lightModel;
		public Button onscreen_button_setting;
		public LinearLayout launcher_tab1_user_game_layout;
		public RadioGroup rg_fs_q4game;
		public CheckBox fs_game_user;
        public LinearLayout launcher_tab2_volume_map_config_layout;
        public Spinner launcher_tab2_volume_up_map_config_keys;
        public Spinner launcher_tab2_volume_down_map_config_keys;
        public View main_ad_layout;
        public CheckBox launcher_tab2_enable_gyro;
        public LinearLayout launcher_tab2_enable_gyro_layout;
        public EditText launcher_tab2_gyro_x_axis_sens;
        public EditText launcher_tab2_gyro_y_axis_sens;
		public CheckBox auto_quick_load;
		public Button setup_onscreen_button_opacity;
		public RadioGroup rg_fs_preygame;

		public void Setup()
		{
			edt_cmdline = findViewById(R.id.edt_cmdline);
			res_customlayout = findViewById(R.id.res_customlayout);
			nolight = findViewById(R.id.nolight);
			useetc1cache = findViewById(R.id.useetc1cache);
			useetc1 = findViewById(R.id.useetc1);
            usedxt = findViewById(R.id.usedxt);
			r_harmclearvertexbuffer = findViewById(R.id.r_harmclearvertexbuffer);
			rg_scrres = findViewById(R.id.rg_scrres);
			rg_msaa = findViewById(R.id.rg_msaa);
			rg_color_bits = findViewById(R.id.rg_color_bits);
			rg_fs_game = findViewById(R.id.rg_fs_game);
			edt_fs_game = findViewById(R.id.edt_fs_game);
			edt_mouse = findViewById(R.id.edt_mouse);
			edt_path = findViewById(R.id.edt_path);
			hideonscr = findViewById(R.id.hideonscr);
			mapvol = findViewById(R.id.mapvol);
			secfinglmb = findViewById(R.id.secfinglmb);
			smoothjoy = findViewById(R.id.smoothjoy);
			detectmouse = findViewById(R.id.detectmouse);
			layout_mouseconfig = findViewById(R.id.layout_mouseconfig);
			layout_manualmouseconfig = findViewById(R.id.layout_manualmouseconfig);
			launcher_tab1_game_lib_button = findViewById(R.id.launcher_tab1_game_lib_button);
			res_x = findViewById(R.id.res_x);
			res_y = findViewById(R.id.res_y);
			launcher_tab1_edit_autoexec = findViewById(R.id.launcher_tab1_edit_autoexec);
			launcher_tab1_edit_doomconfig = findViewById(R.id.launcher_tab1_edit_doomconfig);
			launcher_tab1_game_data_chooser_button = findViewById(R.id.launcher_tab1_game_data_chooser_button);
			rg_curpos = findViewById(R.id.rg_curpos);
			edt_harm_r_specularExponent = findViewById(R.id.edt_harm_r_specularExponent);
		    rg_harm_r_lightModel = findViewById(R.id.rg_harm_r_lightModel);
			onscreen_button_setting = findViewById(R.id.onscreen_button_setting);
			launcher_tab1_user_game_layout = findViewById(R.id.launcher_tab1_user_game_layout);
			rg_fs_q4game = findViewById(R.id.rg_fs_q4game);
			fs_game_user = findViewById(R.id.fs_game_user);
            launcher_tab2_volume_map_config_layout = findViewById(R.id.launcher_tab2_volume_map_config_layout);
            launcher_tab2_volume_up_map_config_keys = findViewById(R.id.launcher_tab2_volume_up_map_config_keys);
            launcher_tab2_volume_down_map_config_keys = findViewById(R.id.launcher_tab2_volume_down_map_config_keys);
            main_ad_layout = findViewById(R.id.main_ad_layout);
            launcher_tab2_enable_gyro = findViewById(R.id.launcher_tab2_enable_gyro);
            launcher_tab2_enable_gyro_layout = findViewById(R.id.launcher_tab2_enable_gyro_layout);
            launcher_tab2_gyro_x_axis_sens = findViewById(R.id.launcher_tab2_gyro_x_axis_sens);
            launcher_tab2_gyro_y_axis_sens = findViewById(R.id.launcher_tab2_gyro_y_axis_sens);
			auto_quick_load = findViewById(R.id.auto_quick_load);
			setup_onscreen_button_opacity = findViewById(R.id.setup_onscreen_button_opacity);
			rg_fs_preygame = findViewById(R.id.rg_fs_preygame);
		}
	}
}
