package com.rt.WAXBlue;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

/**
 * Author: Rob Thompson
 * Date: 04/03/2014
 */
public class BackgroundAdapter extends BaseAdapter {

    private Context mContext;
    private final String values[];

    public BackgroundAdapter(Context mContext, String[] values){
        this.mContext = mContext;
        this.values = values;
    }


    @Override
    public int getCount() {
        return values.length;
    }

    @Override
    public Object getItem(int i) {
        return null;
    }

    @Override
    public long getItemId(int i) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        View gridView;

        if (convertView == null) {
            gridView = new View(mContext);

            gridView = inflater.inflate(R.layout.location, null);

            TextView textView = (TextView) gridView.findViewById(R.id.grid_item_label);
            textView.setText(values[position]);
            textView.setBackgroundResource(R.drawable.grid_background_default);


        }else{
            gridView = (View) convertView;
        }

        return gridView;
    }
}
