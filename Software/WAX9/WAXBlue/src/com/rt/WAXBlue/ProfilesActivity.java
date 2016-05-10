package com.rt.WAXBlue;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.*;

import java.io.*;
import java.util.ArrayList;
import java.util.Collections;

/**
 * Author: Rob Thompson
 * Date: 13/04/2014
 */
public class ProfilesActivity extends Activity {

    private static final String TAG = "Profiles Activity";       //Logging tag
    private static boolean D = true;                             //Logging flag

    public static final String PROFILES = "WAX_Profiles.conf";  //name of file containing details of the profiles stored locally.
    public static final String PROFILE_NAMES = "com.rt.WAXBlue.ProfileNames";
    public static final String PROFILE_LOCATIONS = "com.rt.WAXBlue.ProfileLocations";
    public static final String PROFILE_NAME = "com.rt.WAXBlue.ProfileName";
    public static final String EDIT_MESSAGE = "com.rt.WAXBlue.Edit";

    private ArrayList<Profile> profilesList;                //List of profiles read from profiles file
    private ArrayList<String> profileNames;                 //List of names of profiles currently held on device.
    private ArrayAdapter<String> profilesAdapter;           //Array Adapter for the display of the profile names.
    private ListView profilesListView;                      //List view to display the names of available profiles.

    private ArrayList<String> locationsList;                //List of locations in the currently selected profile.
    private ArrayAdapter<String> locationsAdapter;          //Array Adapter for the display of the locations.
    private ListView locationsListView;                     //List view to display the locations in the profile.

    private int selectedProfileIndex;
    private Profile selectedProfile;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.profiles);

        selectedProfileIndex = -1;
        selectedProfile = null;

        profileNames = new ArrayList<String>();
        profilesList = new ArrayList<Profile>();

        manageConfig(this, profilesList, profileNames);

        locationsList = new ArrayList<String>();
        locationsListView = (ListView) findViewById(R.id.currentProfileLocationListView);
        locationsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, locationsList);
        locationsListView.setAdapter(locationsAdapter);



        profilesListView = (ListView) findViewById(R.id.profileListView);
        profilesAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, profileNames);
        profilesListView.setAdapter(profilesAdapter);
        profilesListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                locationsList.clear();
                if(selectedProfileIndex == i){

                    profilesListView.getChildAt(i).setBackgroundResource(Color.TRANSPARENT);
                    selectedProfileIndex = -1;
                    selectedProfile = null;

                }else{
                    if (selectedProfileIndex != -1) {

                        profilesListView.getChildAt(selectedProfileIndex).setBackgroundResource(Color.TRANSPARENT);

                    }
                    profilesListView.getChildAt(i).setBackgroundResource(R.drawable.grid_background_highlighted);
                    selectedProfileIndex = i;
                    for(Profile p : profilesList){
                        String v = ((TextView)profilesListView.getChildAt(i)).getText().toString();
                        if(v.equals(p.getName())){
                            selectedProfile = p;
                        }
                    }
                    if (selectedProfile != null) {
                        String[] profileLocations = selectedProfile.getLocations();
                        Collections.addAll(locationsList, profileLocations);
                    }
                    locationsAdapter.notifyDataSetChanged();
                }
            }
        });


    }

    public static void manageConfig(Context c, ArrayList<Profile> profilesList, ArrayList<String> profileNames){

        profilesList.clear();
        profileNames.clear();

        FileInputStream fis = null;
        //Either open or create the profiles file
        try {
            fis = c.openFileInput(PROFILES);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "Error loading profiles file for reading: " + e.getMessage());

        }

        ObjectInputStream ois = null;

        //If file was opened parse contents
        if(fis!=null){
            try {
                ois = new ObjectInputStream(fis);
            } catch (IOException e) {
                e.printStackTrace();
            }

        }

        //populate list with available profiles
        while (true) {
            try {
                if (ois != null) {
                    profilesList.add((Profile) ois.readObject());
                } else {
                    break;
                }
            } catch (EOFException e) {
                if (D) Log.d(TAG, "End of File");
                try {
                    ois.close();
                } catch (IOException e1) {
                    Log.e(TAG, e.getMessage());
                }
                break;

            } catch (IOException e) {
                Log.e(TAG, e.getMessage());
            } catch (ClassNotFoundException e) {
                Log.e(TAG, e.getMessage());
            }
        }

        for (Profile p : profilesList) {
            profileNames.add(p.getName());
        }

        try {
            if (fis != null) {
                fis.close();
            }
        } catch (IOException e) {
            Log.e(TAG, "Error closing file: "+e.getMessage());
        }

    }


    /**
     * Launch main activity once profile has been selected
     * @param v view element that was clicked. For OS use only.
     */
    public void launch(View v){
        if(selectedProfileIndex != -1){
            Intent intent = new Intent(this, MainActivity.class);
            intent.putExtra(PROFILE_LOCATIONS, locationsList);
            intent.putExtra(PROFILE_NAME, selectedProfile.getName());
            startActivity(intent);
        }else{
            Toast.makeText(this, "Nothing Selected", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Launch Activity to create a new profile.
     * @param v view element that was clicked. For OS use only.
     */
    public void createProfile(View v){

        Intent intent = new Intent(this, CreateProfileActivity.class);
        intent.putExtra(EDIT_MESSAGE, false);
        startActivity(intent);
    }

    /**
     * Launch Activity to edit an existing profile
     * @param v view element that was clicked. For OS use only.
     */
    public void editProfile(View v){

        if(selectedProfile != null){
            Intent intent = new Intent(this, CreateProfileActivity.class);
            intent.putExtra(EDIT_MESSAGE, true);

            intent.putExtra(PROFILE_NAMES, selectedProfile.getName());
            intent.putStringArrayListExtra(PROFILE_LOCATIONS, locationsList);
            startActivity(intent);
        }else{
            Toast.makeText(this, "Nothing Selected", Toast.LENGTH_SHORT).show();
        }

    }

    public void deleteProfile(View v){

        if(selectedProfileIndex != -1){
            ProfilesActivity.manageConfig(this, profilesList, profileNames);
            Profile old = null;
            for(Profile p : profilesList){
                if(p.getName().equals(selectedProfile.getName())){
                    old = p;
                }
            }
            profilesList.remove(old);
            if (old != null) {
                profileNames.remove(old.getName());
            }
            FileOutputStream fos = null;
            try {
                fos = this.openFileOutput(ProfilesActivity.PROFILES, Context.MODE_PRIVATE);
            } catch (FileNotFoundException e) {
                Log.e(TAG, "Could not find profiles file");
            }
            try {
                ObjectOutputStream os = new ObjectOutputStream(fos);

                for (Profile p : profilesList) {
                    os.writeObject(p);
                }
                os.close();
            } catch (IOException e) {
                Log.e(TAG, e.getMessage());
            }

            profilesListView.getChildAt(selectedProfileIndex).setBackgroundResource(Color.TRANSPARENT);
            selectedProfileIndex = -1;
            selectedProfile = null;

            profilesAdapter.notifyDataSetChanged();
            locationsList.clear();
            locationsAdapter.notifyDataSetChanged();
        }else{
            Toast.makeText(this, "Nothing Selected", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        selectedProfileIndex = -1;
        selectedProfile = null;
        //TODO make this work, should update list on resuming activity from create activity.
        manageConfig(this, profilesList, profileNames);
        locationsList.clear();
        locationsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, locationsList);
        locationsListView.setAdapter(locationsAdapter);
        profilesAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, profileNames);
        profilesListView.setAdapter(profilesAdapter);
    }

}