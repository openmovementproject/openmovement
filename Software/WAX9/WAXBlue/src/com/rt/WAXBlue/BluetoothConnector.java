package com.rt.WAXBlue;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CyclicBarrier;

/**
 * Class to handle and coordinate multiple device connections
 */
public class BluetoothConnector{

    private static final String TAG = "Bluetooth Connector";   //Logging tag
    private static final boolean D = true;                     //Logging flag

    private DeviceConnection[] connections;                    //Array of all connections to be made

    /**
     *
     * @param devices List of devices to be connect to
     * @param storageDirectory Path to the storage directory
     * @param rate Sampling rate (Hz)
     * @param mode Output format
     */
    public BluetoothConnector(List<DeviceToBeAdded> devices, File storageDirectory, int rate, int mode, ArrayList<String> fileList) {

        if(D) Log.d(TAG, "Devices: " + devices.toString());

        //Initialise ready semaphore
        CyclicBarrier ready = new CyclicBarrier(devices.size());

        //Initialise connections array
        connections = new DeviceConnection[devices.size()];

        //Counter for Connection IDs
        int counter = 0;

        //Loop through all devices to be connected to
        for(DeviceToBeAdded d : devices){

            BluetoothDevice device = d.getDevice();

            if (D) Log.d(TAG, "Attempting to create new Device Connection with " + device.getName() + " on " + d.getLocation());

            //Create a new connection and add it to the array.
            connections[counter] = new DeviceConnection(device, counter, storageDirectory, d.getLocation(), rate, mode, ready, fileList);

            if (D) Log.d(TAG, "New Device Connections Created Successfully");

            counter++;
        }


    }

    /**
     * Initialise all connections
     */
    public int runThreads(){

        if(D) Log.d(TAG, "Initialising connections");
        boolean success = false;
        int failID = -1;
        for (int i = 0; i < connections.length; i++) {
            //Check each connection is successfully started
            success = connections[i].init();
            Log.d(TAG, "Connection for "+connections[i].getDeviceName()+" "+ (success ? "succeeded" : "failed"));
            if(!success){
                failID = i;
                break;
            }
        }
        //If one of the connections fails
        if(!success){

            //todo stop file creation.
            //Wait a second to allow everything to catch up (time could be reduced)
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted whilst asleep");
            }
            //Send the stop stream command to all devices to ensure they don't get going.
            stopThreads();
            //Wait again to give catch up time
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted whilst asleep");
            }
            //tear everything down.
            killConnections();
            //Todo Fail case in here
        }else{
            //If all connections succeed - go go go!
            for(DeviceConnection connection : connections){
                connection.startConnection();
            }
        }
        return failID;
    }

    /**
     * Stop streams on all connections    TODO change to semamphore!  (Or maybe don't touch a thing since it's working)
     */
    public void stopThreads(){

        for (DeviceConnection connection : connections) {
            connection.stopStream();
        }
    }

    public void killConnections(){
        for(DeviceConnection connection : connections){
            if(connection.getmSocket() != null){
                try {
                    connection.getmSocket().close();

                } catch (IOException e) {
                    Log.e(TAG, "Failed to Close Socket for "+connection.getDeviceName());
                }
            }
        }
    }
}
