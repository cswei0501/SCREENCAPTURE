/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.android.wifidirect;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.util.Log;
import android.widget.Toast;
import android.os.SystemClock;

/**
 * A BroadcastReceiver that notifies of important wifi p2p events.
 */
public class WiFiDirectBroadcastReceiver extends BroadcastReceiver {

    private WifiP2pManager manager;
    private Channel channel;
    private WiFiDirectActivity activity;
    private boolean mSearching = false;

    /**
     * @param manager WifiP2pManager system service
     * @param channel Wifi p2p channel
     * @param activity activity associated with the receiver
     */
    public WiFiDirectBroadcastReceiver(WifiP2pManager manager, Channel channel,
            WiFiDirectActivity activity) {
        super();
        this.manager = manager;
        this.channel = channel;
        this.activity = activity;
    }

    /*
     * (non-Javadoc)
     * @see android.content.BroadcastReceiver#onReceive(android.content.Context,
     * android.content.Intent)
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {

            // UI update to indicate wifi p2p status.
            int state = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, -1);
            if (state == WifiP2pManager.WIFI_P2P_STATE_ENABLED) {
                // Wifi Direct mode is enabled
                activity.setIsWifiP2pEnabled(true);
            } else {
                activity.setIsWifiP2pEnabled(false);
                activity.resetData();

            }
            Log.d(WiFiDirectActivity.TAG, "P2P state changed - " + state);
        } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {

            // request available peers from the wifi p2p manager. This is an
            // asynchronous call and the calling activity is notified with a
            // callback on PeerListListener.onPeersAvailable()
            if (manager != null) {
                manager.requestPeers(channel, (PeerListListener) activity.getFragmentManager()
                        .findFragmentById(R.id.frag_list));
            }
            Log.d(WiFiDirectActivity.TAG, "P2P peers changed");
        } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {

            Log.d(WiFiDirectActivity.TAG, "P2P WIFI_P2P_CONNECTION_CHANGED_ACTION");
            if (manager == null) {
                return;
            }

            NetworkInfo networkInfo = (NetworkInfo) intent
                    .getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);

            if (networkInfo.isConnected()) {
                Log.d(WiFiDirectActivity.TAG, "P2P Connected");

                // we are connected with the other device, request connection
                // info to find group owner IP

                /*DeviceDetailFragment fragment = (DeviceDetailFragment) activity
                        .getFragmentManager().findFragmentById(R.id.frag_detail);
                manager.requestConnectionInfo(channel, fragment);*/
                
                
                
                final class ConnectHelper implements Runnable {
                    public void run() {
                        SystemClock.sleep(500);
                        DeviceListFragment fragmentList = (DeviceListFragment) activity.getFragmentManager()
                                .findFragmentById(R.id.frag_list);
                        WifiP2pDevice device = fragmentList.getPeersDevice();
                        if(device != null)
                        {
                        	if (activity.startWfdPreview(device.deviceAddress) != 0) {
                        		WiFiDirectBroadcastReceiver.this.activity.disconnect();
                        	}
                        }
                    }

                }
                new Thread(new ConnectHelper()).start();

            } else {
                Log.d(WiFiDirectActivity.TAG, "P2P isn't Connected");
                // It's a disconnect
                if (!mSearching) {
                	activity.resetData();
                	activity.discoverPeers();
                }
            }
        } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            Log.d(WiFiDirectActivity.TAG, "WIFI_P2P_THIS_DEVICE_CHANGED_ACTION");
        	
            DeviceListFragment fragment = (DeviceListFragment) activity.getFragmentManager()
                    .findFragmentById(R.id.frag_list);
            fragment.updateThisDevice((WifiP2pDevice) intent.getParcelableExtra(
                    WifiP2pManager.EXTRA_WIFI_P2P_DEVICE));

        } else if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
        	int discoveryState = intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE,
        			WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);
        	Log.d(WiFiDirectActivity.TAG, "Discovery state changed: " + discoveryState);
        	if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STARTED) {
        		mSearching = true;
        		//Toast.makeText(activity, "Discovery started.", Toast.LENGTH_SHORT).show();
        	} else {
        		mSearching = false;
        		//Toast.makeText(activity, "Discovery finished.", Toast.LENGTH_SHORT).show();
        		
        		DeviceListFragment fragment = (DeviceListFragment) activity.getFragmentManager()
                        .findFragmentById(R.id.frag_list);
        		if (fragment.getPeerCount() == 0) {
        			activity.discoverPeers();
        		}
        	}
        }
    }
}
