package com.rt.WAXBlue;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.*;

import java.io.*;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

import static java.lang.Integer.parseInt;

/**
 * Author: Rob Thompson
 * Date: 08/04/2014
 */
public class ConnectionsActivity extends Activity {

    private static final String TAG = "Connections Activity";   //Debugging tag
    private static final boolean D = true;                      //Flag to turn on or off debug logging
    public static final String STORAGE_DIRECTORY = "com.rt.WAXBlue.Storage_Directory";

    private int mode = 1;                                       //Output mode
    private int rate = 100;                                     //Sampling Rate (Hard Coded :( )
    private BluetoothConnector bluetoothConnector;              //Connector to set up and manage threads for BT devices
    private File storageDirectory;                              //Directory to store output files
    private ArrayList<String> locationsList;
    private ArrayList<DeviceToBeAdded> addedDevicesList;
    private GridView locationsGridView;
    private ArrayList<String> fileList;

    private boolean hasRun = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.connections);

        Intent intent = getIntent();
        addedDevicesList= intent.getParcelableArrayListExtra(MainActivity.ADDED_DEVICE_LIST);
        locationsList = intent.getStringArrayListExtra(MainActivity.LOCATIONS_LIST);
        init();
    }

    //TODO FILL THESE IN

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();

    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        bluetoothConnector.killConnections();

    }

    private void init(){

        fileList = new ArrayList<String>();
        locationsGridView = (GridView) findViewById(R.id.connectionLocationGridView);
        ArrayAdapter<String> locationDisplayArrayAdapter = new ArrayAdapter<String>(this, R.layout.centeredtext, locationsList);
        locationsGridView.setAdapter(locationDisplayArrayAdapter);
        Log.d(TAG, locationsGridView.getChildCount()+ "");
        if (!isExternalStorageWritable()) {
            displayToast("Cannot Write to External Storage :(");
            finish();
        } else {
            if (!createDirectoryForStorage()) {
                displayToast("Cannot Log Data");
                finish();
            }
        }
    }

    /**
     * If directory does not exist, make it
     *
     * @return true if successful
     */
    private boolean createDirectoryForStorage() {
        //Default storage location is a directory called WAXBlue Data in downloads. Documents kept screwing around so wasn't used.
        storageDirectory = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).toString() + "/WAXBlue_Data/");
        return storageDirectory.exists() || storageDirectory.mkdirs();
    }



    /**
     * Initiate Bluetooth connection
     *
     * @param v View element that was clicked
     */
    public void connectClick(View v) {

        //ensure mode has been set
        if (mode != -1) {
            //Get number of devices and initialise connection
            bluetoothConnector = new BluetoothConnector(addedDevicesList, storageDirectory, rate, mode, fileList);
        } else {
            displayToast("Please select an output mode");
        }
        //Set the streams running!
        if (D) Log.d(TAG, "Starting Stream");
        //Run threads. Method returns -1 if success else index of failed connection
        int id = bluetoothConnector.runThreads();
        if(id == -1){
            for (int i = 0; i < locationsGridView.getChildCount(); i++) {
                TextView locationBox = (TextView) locationsGridView.getChildAt(i);
                locationBox.setTextColor(Color.WHITE);
                locationBox.setBackgroundResource(R.drawable.grid_background_locked);
            }
            findViewById(R.id.progress).setVisibility(View.VISIBLE);
            hasRun = true;
            findViewById(R.id.stopButton).setEnabled(true);
            findViewById(R.id.connectButton).setEnabled(false);

        }else{
            TextView locationBox = (TextView) locationsGridView.getChildAt(id);
            locationBox.setTextColor(Color.RED);
            locationBox.setBackgroundResource(R.drawable.grid_background_default);
        }
    }


    /**
     * Stop the streams!!
     *
     * @param v Button that was clicked. Only used for OS functionality
     */
    public void stopClick(View v) {
        //TODO worry about this.
        if(bluetoothConnector!=null){
            bluetoothConnector.stopThreads();
        }
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Log.e(TAG, "Interrupted Sleep: " + e.getMessage());
        }
        findViewById(R.id.stopButton).setEnabled(false);
        //if has run
        if(hasRun){
            zipItUp();
        }
        findViewById(R.id.progress).setVisibility(View.INVISIBLE);

    }

    /**
     * Quits the active connection activity
     * @param v
     */
    public void quitClick(View v){

        new AlertDialog.Builder(this)
                .setTitle("Finish")
                .setMessage("Are you sure you want to quit this session?")
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                })
                .setNegativeButton("No", null)
                .show();
    }

    private void zipItUp(){
        File[] fullFileList = storageDirectory.listFiles();
        String profileName = this.getIntent().getStringExtra(ProfilesActivity.PROFILE_NAME);
        profileName = profileName.replaceAll("\\s+", "");

        Calendar c = Calendar.getInstance();
        FileOutputStream fos = null;
        //Month zero indexed in calendar class, increment for readability.
        int month = c.get(Calendar.MONTH);
        month++;
        try {
            fos = new FileOutputStream(storageDirectory.getPath() + "/log_" + profileName + "_" + c.get(Calendar.DATE) + "_" + month +
                    "_" + c.get(Calendar.YEAR) + "_" + c.get(Calendar.HOUR_OF_DAY) + "_" + c.get(Calendar.MINUTE) + ".zip");
        } catch (FileNotFoundException e) {
            Log.e(TAG, e.getMessage());
        }
        ZipOutputStream zos = new ZipOutputStream(fos);

        Toast.makeText(this, "Zipping Files", Toast.LENGTH_LONG).show();
        for (File f : fullFileList) {
            if (fileList.contains(f.getPath())) {
                try {
                    addToZip(f, zos);
                    f.delete();
                } catch (IOException e) {
                    Log.e(TAG, e.getMessage());
                }
            }
        }
        try {
            zos.close();
        } catch (IOException e) {
            Log.e(TAG, "Error closing zip stream");
        }
        Toast.makeText(this, "Finished Zipping Files", Toast.LENGTH_LONG).show();


    }

    public static void addToZip(File file, ZipOutputStream zos) throws IOException {

        FileInputStream fis = new FileInputStream(file);

        // we want the zipEntry's path to be a relative path that is relative
        // to the directory being zipped, so chop off the rest of the path

        ZipEntry zipEntry = new ZipEntry(file.getName());
        zos.putNextEntry(zipEntry);

        byte[] bytes = new byte[1024];
        int length;
        while ((length = fis.read(bytes)) >= 0) {
            zos.write(bytes, 0, length);
        }

        zos.closeEntry();
        fis.close();
    }


    /**
     * Display string as toast
     *
     * @param s String toast text
     */
    private void displayToast(String s) {
        Toast.makeText(this, s, Toast.LENGTH_SHORT).show();
    }

    /**
     * Checks if external storage is available for read and write
     */
    private boolean isExternalStorageWritable() {
        String state = Environment.getExternalStorageState();
        return Environment.MEDIA_MOUNTED.equals(state);
    }

}
