package com.rt.WAXBlue;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Bundle;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.widget.*;

import java.io.*;
import java.util.ArrayList;
import java.util.Collections;

/**
 * Author: Rob Thompson
 * Date: 16/04/2014
 *
 * Activity to facilitate the creation of a new profile
 */
public class CreateProfileActivity extends Activity {

    private static final boolean D = true;                              //Logging Flag
    private static final String TAG = "Create Profile";
    private Profile profile;                                            //The profile to be created
    private String locationName;
    private String initialName;                                         //Original name of profile if being editted.

    private ArrayList<String> locations;                                //Array list to hold the names of the locations
    private ArrayAdapter<String> locationsAdapter;                      //Array adapter for the display of the locations
    private ListView locationsListView;                                 //List view to display the locations as they are added/

    private int selectedLocation;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.create_profile);

        //TODO fill in
        if(!this.getIntent().getBooleanExtra(ProfilesActivity.EDIT_MESSAGE, false)){
            if(D) Log.d(TAG, "Creating New Profile");
            initFresh();

        }else{
            if (D) Log.d(TAG, "Editing Profile");
            initEdit();
        }
    }

    /**
     * Initialisation method
     */
    protected void initFresh(){

        this.profile = new Profile();

        selectedLocation = -1;
        initialName = null;

        locations = new ArrayList<String>();

        locationsListView = (ListView) findViewById(R.id.profileLocationsListView);
        locationsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, locations);
        locationsListView.setAdapter(locationsAdapter);

        locationsListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                if(selectedLocation!=-1) {
                    locationsListView.getChildAt(selectedLocation).setBackgroundResource(Color.TRANSPARENT);
                }
                selectedLocation = i;
                locationsListView.getChildAt(i).setBackgroundResource(R.drawable.grid_background_highlighted);
            }
        });

    }

    private void initEdit(){

        String pName = this.getIntent().getStringExtra(ProfilesActivity.PROFILE_NAMES);
        ArrayList<String> pLocations = this.getIntent().getStringArrayListExtra(ProfilesActivity.PROFILE_LOCATIONS);
        String[] pLocationsArray = new String[pLocations.size()];
        int i = 0;
        for(String s : pLocations){
            pLocationsArray[i] = s;
            i++;
        }

        this.profile = new Profile(pName, pLocationsArray);

        selectedLocation = -1;
        initialName = pName;

        ((EditText) findViewById(R.id.nameEntry)).setText(pName);

        locations = pLocations;

        locationsListView = (ListView) findViewById(R.id.profileLocationsListView);
        locationsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, locations);
        locationsListView.setAdapter(locationsAdapter);

        locationsListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                if (selectedLocation != -1) {
                    locationsListView.getChildAt(selectedLocation).setBackgroundResource(Color.TRANSPARENT);
                }
                selectedLocation = i;
                locationsListView.getChildAt(i).setBackgroundResource(R.drawable.grid_background_highlighted);
            }
        });
    }

    /**
     * Add a location to the profile
     * @param v View element that has been clicked. For OS use only
     */
    public void addLocation(View v){

        if(D) Log.d(TAG, "Add Location Clicked");

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Add Location");

        // Set up the input
        final EditText input = new EditText(this);

        // Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
        input.setInputType(InputType.TYPE_CLASS_TEXT);
        builder.setView(input);

        // Set up the buttons
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

                locationName = input.getText().toString();

                //add the input text to the array list of locations
                locations.add(locationName);

                //update the array adapter
                locationsAdapter.notifyDataSetChanged();
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });

        builder.show();

    }

    /**
     * Removes the currently highlighted location
     * @param v View element that has been clicked. For OS use only
     */
    public void deleteLocation(View v){
        if (selectedLocation != -1) {
            //Create confirmation dialogue
            new AlertDialog.Builder(this)
                    .setTitle("Finish")
                    .setMessage("Are you sure you want to delete this Location?")
                    .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            String s;
                            s = (String) ((TextView) locationsListView.getChildAt(selectedLocation)).getText();
                            locations.remove(s);
                            locationsAdapter.notifyDataSetChanged();
                            selectedLocation = -1;                    }
                    })
                    .setNegativeButton("No", null)
                    .show();
        }else {
            Toast.makeText(this, "Nothing selected", Toast.LENGTH_SHORT).show();
        }

    }


    public void editLocation(View v){

        if (selectedLocation != -1) {


            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setTitle("Add Location");

            // Set up the input
            final EditText input = new EditText(this);
            input.setText(((TextView)locationsListView.getChildAt(selectedLocation)).getText());
            // Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
            input.setInputType(InputType.TYPE_CLASS_TEXT);
            builder.setView(input);

            // Set up the buttons
            builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {

                    locationName = input.getText().toString();

                    //add the input text to the array list of locations
                    locations.set(selectedLocation, locationName);

                    //update the array adapter
                    locationsAdapter.notifyDataSetChanged();
                }
            });
            builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    dialog.cancel();
                }
            });

            builder.show();

        }
        else{
            Toast.makeText(this, "Nothing selected", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Save the profile in it's current state.
     */
    public void saveProfile(View v){

        ArrayList<Profile> profilesList = new ArrayList<Profile>();
        ArrayList<String> profileNames = new ArrayList<String>();


        ProfilesActivity.manageConfig(this, profilesList, profileNames);
        //Todo check for duplicates

        String name = ((EditText)(findViewById(R.id.nameEntry))).getText().toString();
        if (!name.equals("")) {
            if(profileNames.contains(name) && !name.equals(initialName)){
                Toast.makeText(this, "Profile already exists with this name", Toast.LENGTH_SHORT).show();
            }else{
                Profile old = null;
                for(Profile p : profilesList){
                    if (p.getName().equals(initialName))
                        old = p;
                }

                profilesList.remove(old);

                profile.setName(name);
                String[] locationsArray = new String[locations.size()];
                int i = 0;
                for(String l : locations){
                    locationsArray[i] = l;
                    i++;
                }
                profile.setLocations(locationsArray);

                profilesList.add(profile);

                FileOutputStream fos = null;
                try {
                    fos = this.openFileOutput(ProfilesActivity.PROFILES, Context.MODE_PRIVATE);
                } catch (FileNotFoundException e) {
                    Log.e(TAG, "Could not find profiles file");
                }
                try {
                    ObjectOutputStream os = new ObjectOutputStream(fos);

                    for(Profile p : profilesList){
                        os.writeObject(p);
                    }
                    os.close();
                } catch (IOException e) {
                    Log.e(TAG, e.getMessage());
                }
                finish();
            }

        }else{
            //prompt name entry
            Toast.makeText(this, "Please Name your Profile", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Cancels profile creation and exits
     * @param v View element that was clicked. For OS use only
     */
    public void cancel(View v){

        //Create confirmation dialogue
        new AlertDialog.Builder(this)
                .setTitle("Finish")
                .setMessage("Are you sure you want to cancel this entry?")
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        //Kill activity
                        finish();
                    }
                })
                .setNegativeButton("No", null)
                .show();

    }

}