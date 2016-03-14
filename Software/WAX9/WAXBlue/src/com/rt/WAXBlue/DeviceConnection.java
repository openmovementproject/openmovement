package com.rt.WAXBlue;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.UUID;
import java.util.concurrent.CyclicBarrier;

/**
 * Attempt a new Bluetooth connection on a new thread
 * @author Rob Thompson
 *
 */
public class DeviceConnection{

    private static final String TAG = "Device Connection";   //Logging tag
    private static final boolean D = true;                  //Logging flag

    public static final String UUID_STRING = "00001101-0000-1000-8000-00805f9b34fb";     //UUID of device

    private BluetoothDevice waxDevice;          //The device to connect to
    private BluetoothSocket mSocket;            //The devices Bluetooth socket
    private ConnectedThread connection;         //Runnable implementation for the connection
    private String deviceName;                  //Name of device to which a connection will be established
    private File storageDirectory;              //Directory to store log files
    private String location;                    //Location at which the device will be attached
    private final int rate;                     //Rate at which the device should stream
    private int mode;                           //Mode of streaming for device
    private CyclicBarrier ready;                //Semaphore to indicate that all devices are ready to start streaming
    private Thread mConnected;                  //Thread to execute the connection
    private ArrayList<String> fileList;
    /**
     *
     * @param waxDevice         Device to connect to
     * @param id                Id of connection
     * @param storageDirectory  Directory for the storage of log files
     * @param location          Location at which device will be attached
     * @param rate              Rate at which device will stream
     * @param mode              Mode in which device will stream
     * @param ready             Semaphore to indicate device is ready to stream
     */
    public DeviceConnection(BluetoothDevice waxDevice, final int id, File storageDirectory, String location, int rate,
                            int mode, CyclicBarrier ready, ArrayList<String> fileList) {

        if (D) Log.d(TAG, "Constructing device connection on: " + waxDevice.getName() + " ID: "+id);

        this.waxDevice = waxDevice;
        this.mode = mode;
        this.rate = rate;
        this.ready = ready;
        this.storageDirectory = storageDirectory;
        this.location = location;
        this.deviceName = waxDevice.getName();
        this.fileList = fileList;
        mSocket = null;
    }

    /**
     * Initialise the connection to the device
     * @return true if connection successful
     */
    public boolean init() {
        if (D) Log.d(TAG, "Running Device Connection: " + waxDevice.getName());

        // Open device's Bluetooth socket
        try {
            if (D) Log.d(TAG, "Opening Socket with " + waxDevice.getName());
            mSocket = waxDevice.createRfcommSocketToServiceRecord(UUID.fromString(UUID_STRING));

        } catch (IOException e) {
            if (D) Log.e(TAG, "Error opening Socket: "+e.getMessage());
            return false;
        }

        //Establish connection to socket
        try {
            mSocket.connect();
        } catch (IOException e) {
            //TODO Fail here. Return to some place better
            Log.e(TAG, "Error Connecting to Socket on "+waxDevice.getName() +": "+ e.getMessage());
            try {
                mSocket.close();
            } catch (IOException e2) {
                Log.e(TAG, "Error closing socket: " + e2.getMessage());
                return false;
            }
            return false;
        }

        //Create connection runnable
        connection = new ConnectedThread(mSocket, storageDirectory, location, rate, mode, ready, fileList, waxDevice);
        //Create thread using connection runnable
        mConnected = new Thread(connection);
        return true;

    }

    public void startConnection(){
        mConnected.start();
    }

    /**
     * Stop the device from streaming and close the connection;
     */
    public void stopStream(){

        //Ensure the connection exists
        if(connection!=null){
            //Call stop stream on the connection thread
            connection.stopStream();
        }

        //Close the bluetooth connection.
        try {
            Thread.sleep(1000);
            if(connection!=null){
                connection.socket.close();
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to close Socket: " + e.getMessage());
        } catch (InterruptedException e) {
            Log.e(TAG, "Interrupted Sleep");
        }


    }

    /**
     *
     * @return Connected device's socket
     */
    public BluetoothSocket getmSocket(){
        return mSocket;
    }

    /**
     *
     * @return Name of the connected device
     */
    public String getDeviceName(){
        return deviceName;
    }

}
