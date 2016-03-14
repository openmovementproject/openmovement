package com.rt.WAXBlue;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.LinkedList;
import java.util.UUID;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.Semaphore;

public class ConnectedThread implements Runnable {

    private static final String TAG = "Connected Thread";                     //Logging Tag
    private static final boolean D = true;                                    //Logging Flag
    private String location;
    private File storageDirectory;
    public ArrayList<String> fileList;
    private Writer writerThread;                                              //Thread for writing concurrently
    private OutputStream outStream;                                           //Output stream for writing to device
    private BufferedInputStream inStream;                                     //Input stream for reading from device
    private BluetoothDevice waxDevice;
    public BluetoothSocket socket;                                            //Device socket
    private int mode;                                                         //Recording mode (Binary or ASCII)
    private int rate;                                                         //Sample rate
    private int reconnections;
    private volatile LinkedList<byte[]> bigBuffer = new LinkedList<byte[]>(); //Buffer to contain byte[] with data
    private volatile LinkedList<Integer> sizes = new LinkedList<Integer>();   //Buffer to contain sizes of byte[]s
    private volatile boolean running = true;                                  //Flag for run loop
    private boolean quitting = false;
    private CyclicBarrier ready;                                              //Semaphore to synchronize streaming
    private final Semaphore writerDone;                                       //Semaphore to signal that writer thread has finished
    /**
     *
     * @param socket            Bluetooth socket to device
     * @param storageDirectory  Directory to save device output
     * @param location          Location at which device is mounted
     * @param rate              Sample rate to record at from device
     * @param mode              Mode of operation of device
     * @param ready             Semaphore to synchronize starting of streams
     */
    public ConnectedThread(BluetoothSocket socket, File storageDirectory, String location, int rate, int mode,
                           CyclicBarrier ready, ArrayList<String> fileList, BluetoothDevice waxDevice) {

        if (D) Log.d(TAG, "Creating ConnectedThread");

        this.socket = socket;                           //Bluetooth socket
        this.mode = mode;                               //Streaming mode
        this.rate = rate;                               //Sampling rate
        this.ready = ready;                             //ready semaphore
        this.waxDevice = waxDevice;
        this.writerDone = new Semaphore(1);
        this.location = location;
        this.fileList = fileList;
        this.storageDirectory = storageDirectory;
        reconnections = 0;

        //Create file to write to
        File file = createOutputFile(location, storageDirectory, fileList);

        //Instantiate thread to write to file concurrently
        writerThread = new Writer(file, bigBuffer, sizes);

        //Instantiate input and output streams.
        try {
            inStream = new BufferedInputStream(socket.getInputStream(), 2096);
            outStream = socket.getOutputStream();

        } catch (IOException e) {
            Log.e(TAG, "Couldn't construct thread: " + e.getMessage());
        }

    }

    /**
     * Sends the set rate command to the device
     * @param rate Rate of operation for device (Hz)
     */
    public void setRate(int rate) {

        if(D) Log.d(TAG, "Setting rate to: " + rate);

        try {
            outStream.write(("rate x " + rate + "\r\n\r\n").getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Error writing to device: " + e.getMessage());
        }
    }

    private File createOutputFile(String location, File storageDirectory, ArrayList<String> fileList){
        Calendar c = Calendar.getInstance();            //New calendar instance to create date/time

        //Remove spaces from location name for file naming.
        location = location.replaceAll("\\s+", "");

        //Set file extension depending on mode
        String fType;
        if (mode == 0 || mode == 128) {
            fType = ".csv";
        } else {
            fType = "";

        }

        //Month zero indexed in calendar class, increment for readability.
        int month = c.get(Calendar.MONTH);
        month++;

        //Create file to be written to for logging device data. Filename format: "log_LOCATION_DATE_TIME"
        File file = new File(storageDirectory + "/log_" + location + "_" + c.get(Calendar.DATE) + "_" + month +
                "_" + c.get(Calendar.YEAR) + "_" + c.get(Calendar.HOUR_OF_DAY) + "_" + c.get(Calendar.MINUTE) +(reconnections>0?"_P"+reconnections:"")+ fType);
        synchronized (fileList) {
            fileList.add(file.getPath());
        }

        return file;

    }
    /**
     * Sends the set mode command to the device
     * @param mode Mode of operation for device: 0/128 = ASCII, 1/129 = Binary
     */
    public void setDataMode(int mode) {

        if(D) Log.d(TAG, "Setting mode to: " + mode);

        //If mode is not one acceptable, set to 1
        if (!(mode == 0 || mode == 128 || mode == 1 || mode == 129)) {
            mode = 1;
        }

        try {
            outStream.write(("datamode " + mode + "\r\n\r\n").getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Error writing to device: " + e.getMessage());
        }
    }

    /**
     * Sends the stream command to the device.
     */
    public void startStream() {

        if(D) Log.d(TAG, "Starting stream");

        try {
            outStream.write("STREAM=1\r\n".getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Error writing to device: " + e.getMessage());
        }

    }

    /**
     * Sends the stop stream command to the device.
     */
    public void stopStream() {

        if(D) Log.d(TAG, "Stopping stream");
        //TODO make sure time to finish

        try {
            long time = System.currentTimeMillis();
            outStream.write(("\r\n" + time).getBytes());
        } catch (IOException e) {
            Log.e(TAG, "Error writing to device: " + e.getMessage());
        }

        //TODO Semaphore for close;
        //Stop the run loop.
        running = false;
        quitting = true;

        //Shutdown the Writer thread
        writerThread.shutdown();

        try {
            writerDone.acquire();
        } catch (InterruptedException e) {
            Log.e(TAG, "Interrupted whilst acquiring writer's semaphore");
        }

    }

    private void attemptReconnect(){

        if(D) Log.d(TAG, "Attempting to reconnect");

        try {
            inStream.close();
            outStream.close();
        } catch (IOException e) {
            Log.e(TAG, "Failed to close IO streams");
        }
        writerThread.shutdown();
        //flag to indicate reconnection success
        boolean reconnected = false;

        while (!reconnected) {

            try {
                if (socket.isConnected()) {
                    socket.close();
                }
                //recreate the socket
                socket = waxDevice.createRfcommSocketToServiceRecord(UUID.fromString(DeviceConnection.UUID_STRING));
                socket.connect();
                try {
                    inStream = new BufferedInputStream(socket.getInputStream(), 2096);
                    outStream = socket.getOutputStream();
                    reconnected = true;
                    reconnections++;
                } catch (IOException e2) {
                    Log.e(TAG, "Couldn't create IO streams: " + e2.getMessage());
                    reconnected = false;
                }
            } catch (IOException e) {
                Log.e(TAG, "Couldn't Reconnect to Socket");
                reconnected = false;
            }

        }
        File file = createOutputFile(location, storageDirectory, fileList);

        writerThread = new Writer(file, bigBuffer, sizes);
        if(D) Log.d(TAG, "Starting Writer Thread");
        writerThread.start();
        startStream();

    }

    @Override
    public void run() {
        if(D) Log.d(TAG, "Running Thread");

        int bytes = 0; //Holds the number of bytes read



        //Sleeps are inserted to allow time for device to process commands
        try {
            //Set recording rate
            setRate(rate);

            //Set mode of recording
            Thread.sleep(100);
            setDataMode(mode);

            //Wait for all devices to be ready.
            try {
                ready.await();
            } catch (BrokenBarrierException e) {
                Log.e(TAG, "BARRIER BEEN BUSTED!");
            }
            //Send the start stream command.
            startStream();

        } catch (InterruptedException e) {
            Log.e(TAG, "Sleep Interrupted");
        }
        //Start the writer thread.
        writerThread.start();
        //Until told to stop
        while (running) {

            //Create buffer to hold read bytes
            byte[] buffer = new byte[1024];

            //Read from device
            try {
                bytes = inStream.read(buffer);
            } catch (IOException e) {
                if (!quitting){
                    Log.e(TAG, "\nFailed to read: " + e.getMessage(), e);
                    //reconnect in here
                    attemptReconnect();
                }
            }


            synchronized (writerThread) {
                //if data is available to be written
                if (bytes > 0) {
                    //add to the linked list.
                    bigBuffer.add(buffer);
                    sizes.add(bytes);
                    //let the writer thread know there is data to write.
                    writerThread.notify();
                }
            }
        }

        //Once finished running close all streams
        try {
            //outStream.write("RESET\r\n".getBytes());
            Thread.sleep(250);
            outStream.close();
            inStream.close();
        } catch (IOException e) {
            Log.e(TAG, "Error closing streams");
        } catch (InterruptedException e) {
            Log.e(TAG, "Interrupted Sleep");
        }


    }

}
