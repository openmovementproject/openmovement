package com.rt.WAXBlue;


import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.*;

import java.util.*;

public class MainActivity extends Activity {

    private static final String TAG = "Main Activity";          //Debugging tag
    private static final boolean D = true;                      //Flag to turn on or off debug logging

    private static final int REQUEST_ENABLE_BT = 1;             //Int to allow for BT enabling request
    public static final String ADDED_DEVICE_LIST
            = "com.rt.WAXBlue.addedDeviceList";                 //Name of added devices parcel for connection activity
    public static final String LOCATIONS_LIST
            = "com.rt.WAXBlue.locationsList";                   //Name of locationsList parcel for connection intent

    private GridView locationsGridView;                         //GridView to display the locations at which the devices will be attached
    private ArrayList<String> locationsList;                    //ArrayList of locations to be passed to array adapter
    private ArrayAdapter<String> locationDisplayArrayAdapter;   //Array Adapter for GridView
    private String[] locations;

    private List<String> pairedDevicesList;                     //List of device names paired with phone
    private Set<BluetoothDevice> pairedDevicesSet;              //Set of devices paired with phone
    private ArrayAdapter<String> deviceDisplayArrayAdapter;     //Paired devices array adapter

    private ArrayList<Integer> usedLocations;                   //Array list to hold indices of locations already assigned
    private List<DeviceToBeAdded> addedDevicesList;             //List of devices to be connected to

    private int selectedItem = -1;                              //Int representing which location has been selected
    private boolean locked = false;                             //Flag to indicate status of buttons
    private boolean selected = false;                           //Flag to indicate if any location is currently selected

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ArrayList<String> incomingLocations = this.getIntent().getStringArrayListExtra(ProfilesActivity.PROFILE_LOCATIONS);
        int i = 0;
        locations = new String[incomingLocations.size()];
        for(String il : incomingLocations){
            locations[i] = il;
            i++;
        }
        init();
    }
    //TODO FILL THESE IN
    @Override
    protected void onRestart() {
        super.onRestart();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

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
        addedDevicesList.clear();
        pairedDevicesList.clear();
        usedLocations.clear();
        deviceDisplayArrayAdapter.notifyDataSetChanged();
        locationDisplayArrayAdapter.notifyDataSetChanged();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * Initialise Activity variables
     */
    private void init() {


        ListView pairedDeviceListView = (ListView) this.findViewById(R.id.deviceListView);

        //Initialise Lists
        addedDevicesList = new ArrayList<DeviceToBeAdded>();
        pairedDevicesList = new ArrayList<String>();
        usedLocations = new ArrayList<Integer>();

        //Perform Bluetooth Checks, paired devices list must be initialised first.
        if (!checkBluetooth()) {
            finish();
            //TODO make sure no other options
        }

        //Create display adapter for paired Devices view
        deviceDisplayArrayAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                pairedDevicesList);

        pairedDeviceListView.setAdapter(deviceDisplayArrayAdapter);
        pairedDeviceListView.getBackground().setAlpha(92);

        //Create the click functionality for the devices
        pairedDeviceListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                //Check that a location is currently selected
                if (selectedItem != -1 && !locked && !usedLocations.contains(selectedItem)) {

                    //Initialise the TextView to control the text on the button
                    TextView locView = (TextView) locationsGridView.getChildAt(selectedItem);

                    //Un-highlight the button
                    locView.setBackgroundResource(R.drawable.grid_background_default);

                    //Update the button to show the device associated with that location
                    String s = locationsList.get(selectedItem);
                    locationsList.set(selectedItem, s + "\n" + pairedDevicesList.get(i));
                    locationDisplayArrayAdapter.notifyDataSetChanged();

                    /*Associate the selected device with the location by creating a new
                    DeviceToBeAdded instance, and adding it to addedDevicesList.*/
                    //Flag to indicate when device has been found in list
                    boolean found = false;
                    //loop through paired devices
                    for (BluetoothDevice d : pairedDevicesSet) {
                        if (d.getName().equals(pairedDevicesList.get(i)) && !found) {
                            //add device to added devices list, associate with location
                            addedDevicesList.add(new DeviceToBeAdded(d, locations[selectedItem]));
                            //set found flag
                            found = true;
                        }
                    }
                    if (D)
                        Log.d(TAG, "Location: " + locations[selectedItem] + ", i: " + i + ", Device: " + pairedDevicesList
                                .get(i));

                    //Remove the added device from the list of devices being displayed, device still exists
                    //in pairedDevicesSet
                    pairedDevicesList.remove(i);
                    deviceDisplayArrayAdapter.notifyDataSetChanged();
                    usedLocations.add(selectedItem);
                    //Deselect location
                    View v = locationsGridView.getChildAt(selectedItem);
                    deselectItem(v);
                }
            }
        });

        locationsGridView = (GridView) findViewById(R.id.locationGridView);
        locationsList = new ArrayList<String>();
        Collections.addAll(locationsList, locations);
        locationDisplayArrayAdapter = new ArrayAdapter<String>(this, R.layout.centeredtext, locationsList);
        locationsGridView.setAdapter(locationDisplayArrayAdapter);

        locationsGridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                if (!locked) {
                    if (!selected) {
                        selectItem(view, i);
                    } else if (selectedItem == i) {
                        deselectItem(view);
                    } else {
                        //deselect current item
                        View preView = locationsGridView.getChildAt(selectedItem);
                        deselectItem(preView);
                        //select new item
                        selectItem(view, i);

                    }
                }
            }
        });


    }



    /**
     * Deselects a highlighted location
     * @param view clicked view element
     */
    private void deselectItem(View view) {
        //Unhighlight
        view.setBackgroundResource(R.drawable.grid_background_default);
        //Indicate no location is selected anymore
        selected = false;
        selectedItem = -1;
    }

    /**
     * Highlights clicked location and indicates that it has been selected
     * @param view View element that was selected
     * @param item item that has been selected
     */
    private void selectItem(View view, int item) {
        view.setBackgroundResource(R.drawable.grid_background_highlighted);
        if (!selected) {
            selectedItem = item;
            selected = true;
        }
    }

    /**
     * Checks that bluetooth is supported, enabled and that there are devices paired.
     *
     * @return true if bluetooth connection successful
     */
    private boolean checkBluetooth() {

        //Retrieve adapter
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        //Check that Bluetooth is supported on Android Device
        if (mBluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not supported on this device", 0).show();
            finish();
        }

        // Check if enabled, if not request to enable it.
        if (!(mBluetoothAdapter != null && mBluetoothAdapter.isEnabled())) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        //Retrieve set of paired devices
        assert mBluetoothAdapter != null;
        pairedDevicesSet = mBluetoothAdapter.getBondedDevices();
        populatePairedDevices();

        //TODO if number of paired devices is less than number of locations, open up dialogue for discovery of new items

        if (D) Log.d(TAG, "Connected Successfully to Adapter");
        return true;
    }

    /**
     * Populates just the paired devices list from the set.
     */
    private void populatePairedDevices() {
        //Populate Array Adapter with devices for display in list view

        if (pairedDevicesSet.size() > 0) {
            for (BluetoothDevice d : pairedDevicesSet) {
                pairedDevicesList.add(d.getName());
            }
        }
    }

    /**
     * Clears all bindings and resets all lists.
     * @param v View element that was clicked. Only for OS functionality
     */
    public void clearClick(View v) {

        //Clear old paired devices list
        pairedDevicesList.clear();
        //Repopulate with all paired devices
        populatePairedDevices();
        //Clear list of locations
        locationsList.clear();
        //Repopulate
        Collections.addAll(locationsList, locations);
        //Notify display adapters of changes
        deviceDisplayArrayAdapter.notifyDataSetChanged();
        locationDisplayArrayAdapter.notifyDataSetChanged();
        //Release all locations for use again
        usedLocations.clear();
        //Clear added devices list
        addedDevicesList.clear();
        //Deselect any location that might currently be selected
        if (selectedItem != -1) {
            View selView = locationsGridView.getChildAt(selectedItem);
            deselectItem(selView);
        }
        //TODO more clear code
    }

    /**
     * Clears selected location/device binding and repopulates lists.
     * @param v View element that was clicked. Only for OS functionality
     */
    public void clearSelectedClick(View v) {

        //Check to make sure something has been selected at least
        if (!selected) {
            displayToast("Nothing to Clear");
        }
        //Check to make sure selected item has been previously set
        else if (!usedLocations.contains(selectedItem)) {
            displayToast("Nothing to Clear");
        } else {
            //Get the view to be deselected
            TextView t = (TextView) locationsGridView.getChildAt(selectedItem);
            //Iterate through added devices list to find the device to be cleared
            String deviceName = null;
            DeviceToBeAdded[] itList = addedDevicesList.toArray(new DeviceToBeAdded[0]);
            for (DeviceToBeAdded d : itList) {
                //get the text of the location/device pairing
                String s = (String) t.getText();

                //if the pairing contains the name of the currently inspected device, remove from added devices,
                //add back into paired devices list for reselection
                if (s.contains(d.getDeviceName())) {
                    deviceName = d.getDeviceName();
                    pairedDevicesList.add(d.getDeviceName());
                    addedDevicesList.remove(d);
                }
            }
            //Rename the location to no-longer include the device name
            for (int i = 0; i < locationsList.size(); i++) {
                String s = locationsList.get(i);
                if (s.contains(deviceName) && deviceName != null) {
                    locationsList.set(i, locations[selectedItem]);
                }
            }
            //Release the location for reuse
            usedLocations.remove((Integer) selectedItem);
            //deselect the location
            deselectItem(t);
            //notify the display
            deviceDisplayArrayAdapter.notifyDataSetChanged();
            locationDisplayArrayAdapter.notifyDataSetChanged();
        }
    }

    public void addPreset(View v){

        String[] pl = {"WAX9-5620", "WAX9-5666", "WAX9-A904", "WAX9-573E", "WAX9-5642"};
        ArrayList<String> presetList = new ArrayList<String>();
        Collections.addAll(presetList, pl);

            if(!pairedDevicesList.containsAll(presetList)){
                displayToast("Please pair all devices");
            }

        for(BluetoothDevice d : pairedDevicesSet){
            if(d.getName().equals(pl[0])){
                addedDevicesList.add(new DeviceToBeAdded(d, "Left Fore White"));
                String s = locationsList.get(0);
                locationsList.set(0, s + "\n" + presetList.get(0));
            }
            else if (d.getName().equals(pl[1])) {
                addedDevicesList.add(new DeviceToBeAdded(d, "Left Hind Red"));
                String s = locationsList.get(1);
                locationsList.set(1, s + "\n" + presetList.get(1));
            }
            else if (d.getName().equals(pl[2])) {
                addedDevicesList.add(new DeviceToBeAdded(d, "Noseband Purple"));
                String s = locationsList.get(2);
                locationsList.set(2, s + "\n" + presetList.get(2));
            }
            else if (d.getName().equals(pl[3])) {
                addedDevicesList.add(new DeviceToBeAdded(d, "Right Fore Blue"));
                String s = locationsList.get(3);
                locationsList.set(3, s + "\n" + presetList.get(3));
            }
            else if (d.getName().equals(pl[4])) {
                addedDevicesList.add(new DeviceToBeAdded(d, "Right Hind Green"));
                String s = locationsList.get(4);
                locationsList.set(4, s + "\n" + presetList.get(4));
            }

        }
        Integer[] ul = {0, 1, 2, 3, 4};
        Collections.addAll(usedLocations, ul);
        moveToConnections();

    }

    /**
     * Requests confirmation of location/device bindings and continues to next phase if successful.
     * @param v
     */
    public void finishClick(View v) {
        //Create confirmation dialogue
        new AlertDialog.Builder(this)
                .setTitle("Finish")
                .setMessage("Are you sure you want to accept this pairing?")
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        moveToConnections();
                    }
                })
                .setNegativeButton("No", null)
                .show();
    }


    /**
     * Creates and starts new activity to manage the connections with the devices.
     */
    private void moveToConnections(){
        Intent intent = new Intent(this, ConnectionsActivity.class);
        intent.putParcelableArrayListExtra(ADDED_DEVICE_LIST, (ArrayList<DeviceToBeAdded>) addedDevicesList);
        intent.putStringArrayListExtra(LOCATIONS_LIST, locationsList);
        intent.putExtra(ProfilesActivity.PROFILE_NAME, this.getIntent().getStringExtra(ProfilesActivity.PROFILE_NAME));
        startActivity(intent);
    }


    /**
     * Menu button, currently no functionality.
     * @param menu Menu item for the activity
     * @return true
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.action_exit:
                //Todo make better
                finish();
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
    }

    /**
     * Display string as toast
     *
     * @param s String toast text
     */
    private void displayToast(String s) {
        Toast.makeText(this, s, Toast.LENGTH_SHORT).show();
    }

}


