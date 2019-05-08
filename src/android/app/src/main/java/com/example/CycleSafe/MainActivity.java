package com.example.CycleSafe;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.BottomNavigationView;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;
import android.widget.TextView;

import android.Manifest;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationManager;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.maps.model.LatLng;

//new

import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import java.lang.reflect.Method;

import android.widget.Switch;

public class MainActivity extends AppCompatActivity implements GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener, com.google.android.gms.location.LocationListener {

    private TextView mTextMessage;

//    private BottomNavigationView.OnNavigationItemSelectedListener mOnNavigationItemSelectedListener
//            = new BottomNavigationView.OnNavigationItemSelectedListener() {
//
//        @Override
//        public boolean onNavigationItemSelected(@NonNull MenuItem item) {
//            switch (item.getItemId()) {
//                case R.id.navigation_home:
//                    mTextMessage.setText(R.string.title_home);
//                    return true;
//                case R.id.navigation_dashboard:
//                    mTextMessage.setText(R.string.title_dashboard);
//                    return true;
//                case R.id.navigation_notifications:
//                    mTextMessage.setText(R.string.title_notifications);
//                    return true;
//            }
//            return false;
//        }
//    };


    private static final String TAG = "MainActivity";
    private TextView mLatitudeTextView;
    private TextView mLongitudeTextView;
    private GoogleApiClient mGoogleApiClient;
    private Location mLocation;
    private LocationManager mLocationManager;

    private LocationRequest mLocationRequest;
    private com.google.android.gms.location.LocationListener listener;
    private long UPDATE_INTERVAL = 2 * 1000;  /* 10 secs */
    private long FASTEST_INTERVAL = 2000; /* 2 sec */

    //new
    BluetoothSocket mmSocket;
    BluetoothDevice mmDevice = null;

    final byte delimiter = 33;
    int readBufferPosition = 0;
    //new

    //new
    //ToggleButton simpleToggleButton;
    Switch brakeLightsSet;
    Switch leftTurnSignalSet, rightTurnSignalSet;
    Switch leftProxSet, rightProxSet;
    Switch leftFrontProxSet, rightFrontProxSet;
    Switch SoundSet;// rightSoundSet;
    Switch HapticSet;// rightHapticSet;
    Switch blTEST, fullSysTEST ;


    public void writeData(BluetoothSocket mmsock, int data)
    {
        OutputStream tmpOut = null;

        // Get the BluetoothSocket input and output streams
        try {
            tmpOut = mmsock.getOutputStream();
        } catch (IOException e) {
            Log.e(TAG, "temp sockets not created", e);
        }

        try {
            tmpOut.write(data);



        } catch (IOException e) {
            Log.e(TAG, "Exception during write", e);
        }

    }
    public void connectBT(BluetoothDevice device) {
        mmDevice = device;
        BluetoothSocket tmp = null;
        UUID SerialPortServiceClass_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");


        // Get a BluetoothSocket for a connection with the
        // given BluetoothDevice
        try {
            if ( true ) {
                Method method;

                method = device.getClass().getMethod("createRfcommSocket", new Class[] { int.class } );
                //Toast.makeText(this, "finish1", Toast.LENGTH_SHORT).show();

                tmp = (BluetoothSocket) method.invoke(device, 1);
                Toast.makeText(this, "finish2", Toast.LENGTH_SHORT).show();

            }
            else {
                tmp = device.createRfcommSocketToServiceRecord( SerialPortServiceClass_UUID );
            }
        } catch (Exception e) {
            Log.e(TAG, "create() failed", e);
        }
        mmSocket = tmp;


        try {
            // This is a blocking call and will only return on a
            // successful connection or an exception

            tmp.connect();
            mmSocket = tmp;
        } catch (IOException e)
        {
            Toast.makeText(this, "finish2", Toast.LENGTH_SHORT).show();

        }
        writeData(tmp, 000);



    }


    private LocationManager locationManager;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        BottomNavigationView navigation = (BottomNavigationView) findViewById(R.id.navigation);
        navigation.setOnNavigationItemSelectedListener(navListner);


//        simpleToggleButton = (ToggleButton) findViewById(R.id.simpleToggleButton);
//        simpleToggleButton.setTextSize(25); // set 25sp displayed text size of toggle button
//
        brakeLightsSet = (Switch) findViewById(R.id.bLights);
//        leftTurnSignalSet = (Switch) findViewById(R.id.lSignal);
//        rightTurnSignalSet = (Switch) findViewById(R.id.rSignal);
//        leftProxSet = (Switch) findViewById(R.id.lProx);
//        rightProxSet = (Switch) findViewById(R.id.rProx);
//        leftFrontProxSet = (Switch) findViewById(R.id.lFProx);
//        rightFrontProxSet = (Switch) findViewById(R.id.rFProx);
        SoundSet = (Switch) findViewById(R.id.sound);
//        rightSoundSet = (Switch) findViewById(R.id.rSound);
        HapticSet = (Switch) findViewById(R.id.haptic);
//        rightHapticSet = (Switch) findViewById(R.id.rHaptic);
//        blTEST= (Switch) findViewById(R.id.brakeTEST);
//        fullSysTEST = (Switch) findViewById(R.id.fullTEST);
//
//
//
//
//        simpleToggleButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                String status = "ToggleButton1 : " + simpleToggleButton.getText() + "\n";
//                Toast.makeText(getApplicationContext(), status, Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                writeData(mmSocket, 101);
//            }
//        });
//
        brakeLightsSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (brakeLightsSet.isChecked())
                {
                    writeData(mmSocket, 111);

                    Toast.makeText(getApplicationContext(), "SENT 111", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 110);

                    Toast.makeText(getApplicationContext(), "SENT 110", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });
//
//        leftTurnSignalSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (leftTurnSignalSet.isChecked())
//                {
//                    writeData(mmSocket, 121);
//
//                    Toast.makeText(getApplicationContext(), "SENT 121", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 120);
//
//                    Toast.makeText(getApplicationContext(), "SENT 120", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        rightTurnSignalSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (rightTurnSignalSet.isChecked())
//                {
//                    writeData(mmSocket, 131);
//
//                    Toast.makeText(getApplicationContext(), "SENT 131", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 130);
//
//                    Toast.makeText(getApplicationContext(), "SENT 130", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        leftProxSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (leftProxSet.isChecked())
//                {
//                    writeData(mmSocket, 141);
//
//                    Toast.makeText(getApplicationContext(), "SENT 141", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 140);
//
//                    Toast.makeText(getApplicationContext(), "SENT 140", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        rightProxSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (rightProxSet.isChecked())
//                {
//                    writeData(mmSocket, 151);
//
//                    Toast.makeText(getApplicationContext(), "SENT 151", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 150);
//
//                    Toast.makeText(getApplicationContext(), "SENT 150", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        leftFrontProxSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (leftFrontProxSet.isChecked())
//                {
//                    writeData(mmSocket, 161);
//
//                    Toast.makeText(getApplicationContext(), "SENT 161", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 160);
//
//                    Toast.makeText(getApplicationContext(), "SENT 160", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        rightFrontProxSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (rightFrontProxSet.isChecked())
//                {
//                    writeData(mmSocket, 171);
//
//                    Toast.makeText(getApplicationContext(), "SENT 171", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 170);
//
//                    Toast.makeText(getApplicationContext(), "SENT 170", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
        SoundSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (SoundSet.isChecked())
                {
                    writeData(mmSocket, 181);

                    Toast.makeText(getApplicationContext(), "SENT 181", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                    writeData(mmSocket, 191);
                }
                else
                {
                    writeData(mmSocket, 180);

                    Toast.makeText(getApplicationContext(), "SENT 180", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's

                    writeData(mmSocket, 190);
                }
            }
        });
//
//        rightSoundSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (rightSoundSet.isChecked())
//                {
//                    writeData(mmSocket, 191);
//
//                    Toast.makeText(getApplicationContext(), "SENT 191", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 190);
//
//                    Toast.makeText(getApplicationContext(), "SENT 190", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        leftHapticSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (leftHapticSet.isChecked())
//                {
//                    writeData(mmSocket, 201);
//
//                    Toast.makeText(getApplicationContext(), "SENT 201", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 200);
//
//                    Toast.makeText(getApplicationContext(), "SENT 200", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
        HapticSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (HapticSet.isChecked())
                {
                    writeData(mmSocket, 211);

                    Toast.makeText(getApplicationContext(), "SENT 211", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's

                    writeData(mmSocket, 201);
                }
                else
                {
                    writeData(mmSocket, 210);

                    Toast.makeText(getApplicationContext(), "SENT 210", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's

                    writeData(mmSocket, 200);
                }
            }
        });
//
//        blTEST.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (blTEST.isChecked())
//                {
//                    writeData(mmSocket, 241);
//
//                    Toast.makeText(getApplicationContext(), "SENT 241", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 240);
//
//                    Toast.makeText(getApplicationContext(), "SENT 240", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        fullSysTEST.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (fullSysTEST.isChecked())
//                {
//                    writeData(mmSocket, 243);
//
//                    Toast.makeText(getApplicationContext(), "SENT 243", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 0);
//
//                    Toast.makeText(getApplicationContext(), "SENT 0", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });





//        mLatitudeTextView = (TextView) findViewById((R.id.latitude_textview));
//        mLongitudeTextView = (TextView) findViewById((R.id.longitude_textview));

        mGoogleApiClient = new GoogleApiClient.Builder(this)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .addApi(LocationServices.API)
                .build();

        mLocationManager = (LocationManager)this.getSystemService(Context.LOCATION_SERVICE);

        checkLocation(); //check whether location service is enable or not in your  phone
        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        BluetoothDevice ddevice =  mBluetoothAdapter.getRemoteDevice("B8:27:EB:7A:E0:15");
        //Toast.makeText(this, "here", Toast.LENGTH_SHORT).show();

        connectBT(ddevice);
        Toast.makeText(this, "finish", Toast.LENGTH_SHORT).show();




    }

    private BottomNavigationView.OnNavigationItemSelectedListener navListner =
            new BottomNavigationView.OnNavigationItemSelectedListener() {
                @Override
                public boolean onNavigationItemSelected(@NonNull MenuItem menuItem) {
                    //Fragment selecFrag = null;



                    switch(menuItem.getItemId())
                    {
                        case R.id.navigation_dashboard:
                            Intent intent = new Intent(MainActivity.this, DashboardFragment.class);
                            startActivity(intent);
                            break;

                    }
                    //getSupportFragmentManager().beginTransaction().replace(R.id.fragment_container, selecFrag).commit();

                    return true;
                }
            };

    @Override
    public void onConnected(Bundle bundle) {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return;
        }

        startLocationUpdates();

        mLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);

        if(mLocation == null){
            startLocationUpdates();
        }
        if (mLocation != null) {

            // mLatitudeTextView.setText(String.valueOf(mLocation.getLatitude()));
            //mLongitudeTextView.setText(String.valueOf(mLocation.getLongitude()));
        } else {
            Toast.makeText(this, "Location not Detected", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onConnectionSuspended(int i) {
        Log.i(TAG, "Connection Suspended");
        mGoogleApiClient.connect();
    }

    @Override
    public void onConnectionFailed(ConnectionResult connectionResult) {
        Log.i(TAG, "Connection failed. Error: " + connectionResult.getErrorCode());
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (mGoogleApiClient != null) {
            mGoogleApiClient.connect();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mGoogleApiClient.isConnected()) {
            mGoogleApiClient.disconnect();
        }
    }

    protected void startLocationUpdates() {
        // Create the location request
        mLocationRequest = LocationRequest.create()
                .setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY)
                .setInterval(UPDATE_INTERVAL)
                .setFastestInterval(FASTEST_INTERVAL);
        // Request location updates
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return;
        }
        LocationServices.FusedLocationApi.requestLocationUpdates(mGoogleApiClient,
                mLocationRequest, this);
        Log.d("reque", "--->>>>");
    }

    @Override
    public void onLocationChanged(Location location) {

        String msg = "Updated Location: " +
                Double.toString(location.getLatitude()) + "," +
                Double.toString(location.getLongitude());
//        mLatitudeTextView.setText(String.valueOf(location.getLatitude()));
//        mLongitudeTextView.setText(String.valueOf(location.getLongitude() ));
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
        // You can now create a LatLng Object for use with maps
        LatLng latLng = new LatLng(location.getLatitude(), location.getLongitude());
    }

    private boolean checkLocation() {
        if(!isLocationEnabled())
            showAlert();
        return isLocationEnabled();
    }

    private void showAlert() {
        final AlertDialog.Builder dialog = new AlertDialog.Builder(this);
        dialog.setTitle("Enable Location")
                .setMessage("Your Locations Settings is set to 'Off'.\nPlease Enable Location to " +
                        "use this app")
                .setPositiveButton("Location Settings", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface paramDialogInterface, int paramInt) {

                        Intent myIntent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
                        startActivity(myIntent);
                    }
                })
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface paramDialogInterface, int paramInt) {

                    }
                });
        dialog.show();
    }

    private boolean isLocationEnabled() {
        locationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        return locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) ||
                locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
    }

}

