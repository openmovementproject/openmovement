package com.rt.WAXBlue;

import android.bluetooth.BluetoothDevice;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * Author: Rob Thompson
 * Date: 05/03/2014
 */
public class DeviceToBeAdded implements Parcelable {

    private BluetoothDevice d;
    private String location;

    public DeviceToBeAdded(BluetoothDevice d, String location) {
        this.d = d;
        this.location = location;
    }

    public BluetoothDevice getDevice(){
        return this.d;
    }

    public String getDeviceName() {
        return d.getName();
    }

    public String getDeviceAddress() {
        return d.getAddress();
    }

    public String getLocation() {
        return this.location;
    }

    @Override
    public String toString() {
        return this.getDeviceName() + ", " + this.getDeviceAddress() + ", " + this.getLocation();
    }

    public static final Parcelable.Creator<DeviceToBeAdded> CREATOR
            = new Parcelable.Creator<DeviceToBeAdded>() {
        public DeviceToBeAdded createFromParcel(Parcel in) {
            return new DeviceToBeAdded(in);
        }

        public DeviceToBeAdded[] newArray(int size) {
            return new DeviceToBeAdded[size];
        }
    };

    public DeviceToBeAdded(Parcel in){
        this.d = in.readParcelable(null);
        this.location = in.readString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel parcel, int i) {
        parcel.writeParcelable(d, 0);
        parcel.writeString(location);
    }
}
